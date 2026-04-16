#include "Parser.h"

#include <charconv>

#include "Definitions/Globals.h"

#include <memory>
#include <unordered_map>

#include "Components/Process Manager/Process.h"

namespace Parser
{
	namespace
	{
		struct ProcessInformation
		{
			uint32_t                                   PID;
			std::unordered_map<std::string, uintptr_t> Context;
		};

		std::unordered_map<uint32_t, std::unique_ptr<ProcessInformation>> Informations = {};

		namespace Helpers
		{
			std::string_view Trim( std::string_view SV )
			{
				while ( !SV.empty() && SV.front() == ' ' ) SV.remove_prefix( 1 );
				while ( !SV.empty() && SV.back() == ' ' ) SV.remove_suffix( 1 );
				return SV;
			}

			bool TryParseIntegerLiteral( std::string_view& SV, uintptr_t& Out )
			{
				SV = Trim( SV );
				if ( SV.empty() || !std::isdigit( static_cast<unsigned char>( SV.front() ) ) ) return false;

				int              Base  = 10;
				std::string_view Parse = SV;

				if ( Parse.size() > 2 && Parse[ 0 ] == '0' && ( Parse[ 1 ] == 'x' || Parse[ 1 ] == 'X' ) )
				{
					Base  = 16;
					Parse = Parse.substr( 2 );
				}

				const auto [ Pointer, ErrCode ] = std::from_chars( Parse.data(), Parse.data() + Parse.size(), Out, Base );
				if ( ErrCode != std::errc{} ) return false;

				const size_t Consumed = Parse.data() - SV.data() + ( Pointer - Parse.data() );
				SV.remove_prefix( Consumed );
				return true;
			}

			// TODO: This should probably be rewritten
			// SV should always be \0'd by the caller, this isn't really obviously documented on ParseComplex
			bool MightBeModule( const std::string_view SV )
			{
				const auto Dot = SV.rfind( '.' );
				if ( Dot == std::string_view::npos ) return false;
				const auto Extension = SV.substr( Dot + 1 );
				return _strnicmp( Extension.data(), "dll", 3 ) == 0 || _strnicmp( Extension.data(), "exe", 3 ) == 0 || _strnicmp( Extension.data(), "sys", 3 ) == 0;
			}
		}

		namespace Evaluation
		{
			uintptr_t Expression( std::string_view& SV, ProcessInformation* Information );

			uintptr_t Bracketed( std::string_view& SV, ProcessInformation* Information )
			{
				int    Depth = 1;
				size_t i     = 0;

				for ( ; i < SV.size() && Depth; ++i )
				{
					if ( SV[ i ] == '[' ) ++Depth;
					else if ( SV[ i ] == ']' ) --Depth;
				}

				if ( Depth != 0 ) return 0; // TODO: Log this?

				std::string_view Inner = SV.substr( 0, i - 1 );
				SV.remove_prefix( i );

				const uintptr_t Address = Expression( Inner, Information );
				uintptr_t       ReturnValue;
				( void )Process::ReadMemory( Information->PID, Address, &ReturnValue, sizeof( uintptr_t ) );
				return ReturnValue;
			}

			uintptr_t Atom( std::string_view& SV, ProcessInformation* Information )
			{
				SV = Helpers::Trim( SV );
				if ( SV.empty() ) return 0;

				if ( SV.front() == '[' )
				{
					SV.remove_prefix( 1 );
					return Bracketed( SV, Information );
				}

				if ( std::isdigit( static_cast<unsigned char>( SV.front() ) ) || ( SV.size() > 1 && SV.front() == '0' && ( SV[ 1 ] == 'x' || SV[ 1 ] == 'X' ) ) )
				{
					uintptr_t Value{};
					if ( !Helpers::TryParseIntegerLiteral( SV, Value ) ) return 0;
					return Value;
				}

				size_t Length = 0;
				while ( Length < SV.size() && SV[ Length ] != '+' && SV[ Length ] != '-' && SV[ Length ] != ']' ) ++Length;

				const std::string_view Token = Helpers::Trim( SV.substr( 0, Length ) );
				SV.remove_prefix( Length );

				if ( Helpers::MightBeModule( Token ) ) return Process::GetModuleBase( Information->PID, Token );

				const auto Iterator = Information->Context.find( std::string( Token ) );

				if ( Iterator != Information->Context.end() ) return Iterator->second;

				return 0;
			}

			uintptr_t Expression( std::string_view& SV, ProcessInformation* Information )
			{
				uintptr_t Result = Atom( SV, Information );

				while ( !SV.empty() )
				{
					SV = Helpers::Trim( SV );
					if ( SV.empty() || SV.front() == ']' ) break;

					if ( SV.front() != '+' && SV.front() != '-' ) return 0;

					const bool Add = SV.front() == '+';
					SV.remove_prefix( 1 );

					uintptr_t Offset{};
					if ( !Helpers::TryParseIntegerLiteral( SV, Offset ) )
					{
						Offset = Atom( SV, Information );
					}

					Result = Add ? Result + Offset : Result - Offset;
				}

				return Result;
			}
		}
	}

	uintptr_t ParseComplex( const std::string_view Expression, const uint32_t PID, std::optional<uintptr_t> This )
	{
		const auto Iterator = Informations.find( PID );
		if ( Iterator == Informations.end() ) return 0;

		const auto Information = Iterator->second.get();

		std::optional<uintptr_t> PreviousThis = std::nullopt;
		if ( This.has_value() )
		{
			if ( const auto ThisIterator = Information->Context.find( "this" ); ThisIterator != Information->Context.end() ) PreviousThis = ThisIterator->second;

			Information->Context[ "this" ] = This.value();
		}

		std::string_view SV     = Helpers::Trim( Expression );
		const uintptr_t  Result = Evaluation::Expression( SV, Information );
		SV                      = Helpers::Trim( SV );

		if ( This.has_value() )
		{
			if ( PreviousThis.has_value() ) Information->Context[ "this" ] = PreviousThis.value();
			else Information->Context.erase( "this" );
		}

		if ( !SV.empty() ) return 0;
		return Result;
	}

	uintptr_t ParseSimple( std::string_view Expression )
	{
		uintptr_t Value{};
		if ( !Helpers::TryParseIntegerLiteral( Expression, Value ) ) return 0;
		return Value;
	}

	void InitializeContext( const uint32_t PID )
	{
		auto Information                      = std::make_unique<ProcessInformation>();
		Information->PID                      = PID;
		Information->Context[ "ProcessBase" ] = Process::GetProcessBase( PID );
		Information->Context[ "PEB" ]         = Process::GetProcessPEB( PID );
		Information->Context[ "HeapBase" ]    = Process::GetProcessHeapBase( PID );
		Informations[ PID ]                   = std::move( Information );
	}

	void DestroyContext( const uint32_t PID )
	{
		Informations.erase( PID );
	}
}
