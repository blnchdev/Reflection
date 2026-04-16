#pragma once
#include <cstdint>
#include <utility>

// TODO: Move these to Globals...
consteval uint32_t operator""_H( const char* String, const size_t Length )
{
	uint32_t Hash = 2166136261u;
	for ( size_t i = 0; i < Length; ++i ) Hash = ( Hash ^ static_cast<uint32_t>( String[ i ] ) ) * 16777619u;
	return Hash;
}

#define HASH(Str) (Str##_hash)

namespace Themes
{
	using ColorEntry = std::pair<uint32_t, uint32_t>;

	enum class ThemeID : uint8_t
	{
		DarkIslands,
		LightIslands,

		CatppuccinLatte,
		CatppuccinFrappe,
		CatppuccinMacchiato,
		CatppuccinMocha,

		Count
	};

	constexpr float ROUNDING = 15.f;

	class Manager
	{
	public:
		static uint32_t GetColor( uint32_t Hash );
		static void     SetTheme( ThemeID NewTheme );
		static ThemeID  GetTheme();
		static void     Initialize();
	};
}
