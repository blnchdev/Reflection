#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <Windows.h>

enum class ProcessStatus : uint8_t
{
	S_ATTACHED,
	S_ALREADY_ATTACHED,

	E_NO_HIJACK,
	E_ACCESS_DENIED
};

namespace Process
{
	ProcessStatus         Attach( uint32_t PID );
	bool                  Detach( uint32_t PID );
	std::optional<HANDLE> GetHandle( uint32_t PID );
	uintptr_t             GetProcessBase( uint32_t PID );
	uintptr_t             GetProcessPEB( uint32_t PID );
	uintptr_t             GetProcessHeapBase( uint32_t PID );
	uintptr_t             GetModuleBase( uint32_t PID, std::string_view ModuleName );
	const char*           StatusWhat( ProcessStatus Status );
	uint32_t              GetLastAttachedPID();

	// Module resolving for MemoryViewer
	struct PointerData
	{
		bool        Valid    = false;
		bool        InModule = false;
		std::string ModuleName;
		uintptr_t   BaseAddress;
		uintptr_t   Offset;
	};

	bool IsValidPointer( uint32_t PID, uintptr_t VirtualAddress );
	void ResolvePointer( uint32_t PID, uintptr_t VirtualAddress, PointerData* Data );

	bool ReadMemory( uint32_t PID, uintptr_t Address, void* Buffer, size_t Size );
	bool ReadMemory( HANDLE hProcess, uintptr_t Address, void* Buffer, size_t Size );
	bool WriteMemory( HANDLE hProcess, uintptr_t Address, void* Buffer, size_t Size );
}
