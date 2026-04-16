#include "Process.h"

#include <format>
#include <memory>
#include <ntstatus.h>
#include <Psapi.h>

#include "Definitions/Globals.h"
#include "Libraries/Aether/Aether.h"
#include "Libraries/Wraith/Wraith.hpp"
#include <Windows.h>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <print>
#include <string>
#include <string_view>
#include <vector>
#include <Definitions/NtDefs.h>

#include "Components/Parser/Parser.h"

namespace Process
{
	namespace
	{
		class ProcessData
		{
		public:
			HANDLE    hProcess    = INVALID_HANDLE_VALUE;
			uintptr_t BaseAddress = 0;
			uintptr_t PEB         = 0;
			uintptr_t HeapBase    = 0;
			uint32_t  PID         = 0;

			explicit ProcessData( const HANDLE Handle, const uint32_t ProcessID ) : hProcess( Handle ), PID( ProcessID )
			{
				PopulateProcessData();
			}

		private:
			void PopulateProcessData()
			{
				PROCESS_BASIC_INFORMATION PBI    = {};
				const NTSTATUS            Status = Aether::NtQueryInformationProcess( this->hProcess, ProcessBasicInformation, &PBI, sizeof( PBI ), nullptr );

				if ( Status != STATUS_SUCCESS ) return;

				const uintptr_t pBaseAddress = reinterpret_cast<uintptr_t>( PBI.PebBaseAddress ) + offsetof( _PEB, ImageBaseAddress );
				( void )Aether::NtReadVirtualMemory( this->hProcess, reinterpret_cast<PVOID>( pBaseAddress ), &this->BaseAddress, sizeof( uintptr_t ), nullptr );

				const uintptr_t pHeapBase = reinterpret_cast<uintptr_t>( PBI.PebBaseAddress ) + offsetof( _PEB, ProcessHeap );
				( void )Aether::NtReadVirtualMemory( this->hProcess, reinterpret_cast<PVOID>( pHeapBase ), &this->HeapBase, sizeof( uintptr_t ), nullptr );

				this->PEB = reinterpret_cast<uint64_t>( PBI.PebBaseAddress );
			}
		};

		std::vector<std::unique_ptr<ProcessData>> AttachedProcesses;
	}

	ProcessStatus Attach( uint32_t PID )
	{
		const auto Iterator = std::ranges::find_if(
		                                           AttachedProcesses,
		                                           [PID] ( const std::unique_ptr<ProcessData>& Proc )
		                                           {
			                                           return Proc->PID == PID;
		                                           } );

		if ( Iterator != AttachedProcesses.end() ) return ProcessStatus::S_ALREADY_ATTACHED;

		const std::optional Handle = Wraith::HijackHandle( PID );

		if ( !Handle.has_value() || Handle.value() == INVALID_HANDLE_VALUE ) return ProcessStatus::E_NO_HIJACK;

		std::println( "Attached to PID {}, handle is 0x{:X}", PID, reinterpret_cast<uint64_t>( Handle.value() ) );

		AttachedProcesses.emplace_back( std::make_unique<ProcessData>( Handle.value(), PID ) );
		Parser::InitializeContext( PID );

		return ProcessStatus::S_ATTACHED;
	}

	bool Detach( uint32_t PID )
	{
		const auto Iterator = std::ranges::find_if(
		                                           AttachedProcesses,
		                                           [PID] ( const std::unique_ptr<ProcessData>& Proc )
		                                           {
			                                           return Proc->PID == PID;
		                                           } );

		if ( Iterator == AttachedProcesses.end() ) return false;

		( void )Aether::NtClose( Iterator->get()->hProcess );
		AttachedProcesses.erase( Iterator );
		Parser::DestroyContext( PID );

		return true;
	}

	std::optional<HANDLE> GetHandle( uint32_t PID )
	{
		const auto Iterator = std::ranges::find_if(
		                                           AttachedProcesses,
		                                           [PID] ( const std::unique_ptr<ProcessData>& Proc )
		                                           {
			                                           return Proc->PID == PID;
		                                           } );

		if ( Iterator == AttachedProcesses.end() ) return std::nullopt;

		return Iterator->get()->hProcess;
	}

	uintptr_t GetProcessBase( uint32_t PID )
	{
		const auto Iterator = std::ranges::find_if(
		                                           AttachedProcesses,
		                                           [PID] ( const std::unique_ptr<ProcessData>& Proc )
		                                           {
			                                           return Proc->PID == PID;
		                                           } );

		if ( Iterator == AttachedProcesses.end() ) return 0;

		return Iterator->get()->BaseAddress;
	}

	uintptr_t GetProcessPEB( uint32_t PID )
	{
		const auto Iterator = std::ranges::find_if(
		                                           AttachedProcesses,
		                                           [PID] ( const std::unique_ptr<ProcessData>& Proc )
		                                           {
			                                           return Proc->PID == PID;
		                                           } );

		if ( Iterator == AttachedProcesses.end() ) return 0;

		return Iterator->get()->PEB;
	}

	uintptr_t GetProcessHeapBase( uint32_t PID )
	{
		const auto Iterator = std::ranges::find_if(
		                                           AttachedProcesses,
		                                           [PID] ( const std::unique_ptr<ProcessData>& Proc )
		                                           {
			                                           return Proc->PID == PID;
		                                           } );

		if ( Iterator == AttachedProcesses.end() ) return 0;

		return Iterator->get()->HeapBase;
	}

	uintptr_t GetModuleBase( const uint32_t PID, std::string_view ModuleName )
	{
		const HANDLE hSnapshot = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, PID );
		if ( hSnapshot == INVALID_HANDLE_VALUE ) return 0;

		MODULEENTRY32W     Entry{ .dwSize = sizeof( MODULEENTRY32W ) };
		const std::wstring WideModuleName( ModuleName.begin(), ModuleName.end() );

		for ( BOOL Status = Module32FirstW( hSnapshot, &Entry ); Status; Status = Module32NextW( hSnapshot, &Entry ) )
		{
			if ( _wcsicmp( Entry.szModule, WideModuleName.c_str() ) == 0 )
			{
				CloseHandle( hSnapshot );
				return reinterpret_cast<uintptr_t>( Entry.modBaseAddr );
			}
		}

		CloseHandle( hSnapshot );
		return 0;
	}

	const char* StatusWhat( const ProcessStatus Status )
	{
		switch ( Status )
		{
		case ProcessStatus::S_ATTACHED:
			return "Successfully attached";
		case ProcessStatus::S_ALREADY_ATTACHED:
			return "Already attached to this process";
		case ProcessStatus::E_ACCESS_DENIED:
			return "Access Denied";
		case ProcessStatus::E_NO_HIJACK:
			return "Hijacking a HANDLE failed";
		}

		return "Unknown";
	}

	uint32_t GetLastAttachedPID()
	{
		return AttachedProcesses.empty() ? 0 : AttachedProcesses.back()->PID;
	}

	bool IsValidPointer( const uint32_t PID, const uintptr_t VirtualAddress )
	{
		uint8_t    Buffer;
		const auto ReturnValue = GetHandle( PID );

		if ( !ReturnValue.has_value() ) return false;

		return ReadMemory( ReturnValue.value(), VirtualAddress, &Buffer, sizeof( uint8_t ) );
	}

	void ResolvePointer( const uint32_t PID, const uintptr_t VirtualAddress, PointerData* Data )
	{
		MEMORY_BASIC_INFORMATION MBI         = {};
		const auto               ReturnValue = GetHandle( PID );

		if ( !ReturnValue.has_value() )
		{
			Data->Valid = false;
			return;
		}

		if ( !VirtualQueryEx( ReturnValue.value(), reinterpret_cast<LPCVOID>( VirtualAddress ), &MBI, sizeof( MBI ) ) )
		{
			Data->Valid = false;
			return;
		}

		Data->Valid = true;

		auto hModule = static_cast<HMODULE>( MBI.AllocationBase );

		char ModulePath[ MAX_PATH ]{};
		if ( !GetModuleFileNameExA( GetCurrentProcess(), hModule, ModulePath, MAX_PATH ) )
		{
			return;
		}

		Data->InModule = true;

		std::string_view FullPath  = ModulePath;
		const auto       LastSlash = FullPath.rfind( '\\' );

		const uintptr_t BaseAddress = reinterpret_cast<uintptr_t>( hModule );

		Data->Valid       = true;
		Data->BaseAddress = BaseAddress;
		Data->Offset      = VirtualAddress - BaseAddress;
		Data->ModuleName  = std::string{ LastSlash != std::string_view::npos ? FullPath.substr( LastSlash + 1 ) : FullPath };
	}

	bool ReadMemory( const uint32_t PID, const uintptr_t Address, void* Buffer, const size_t Size )
	{
		const auto hProcess = GetHandle( PID );
		if ( !hProcess.has_value() ) return false;
		return ReadMemory( hProcess.value(), Address, Buffer, Size );
	}

	bool ReadMemory( const HANDLE hProcess, const uintptr_t Address, void* Buffer, const size_t Size )
	{
		return Aether::NtReadVirtualMemory( hProcess, reinterpret_cast<PVOID>( Address ), Buffer, Size, nullptr ) == STATUS_SUCCESS;
	}
}
