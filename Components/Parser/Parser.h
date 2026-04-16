#pragma once
#include <cstdint>
#include <optional>
#include <string_view>


namespace Parser
{
	uintptr_t ParseComplex( std::string_view Expression, uint32_t PID, std::optional<uintptr_t> This = std::nullopt );
	uintptr_t ParseSimple( std::string_view Expression );

	void InitializeContext( uint32_t PID );
	void DestroyContext( uint32_t PID );
}
