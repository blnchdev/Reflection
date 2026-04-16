#include "Theme Manager.h"

#include <algorithm>
#include <array>
#include <string>
#include <vector>

#include "Components/Config/Config.h"
#include "Components/Graphics/Graphics.h"
#include "Themes/CatppuccinFrappe.h"
#include "Themes/CatppuccinLatte.h"
#include "Themes/CatppuccinMacchiato.h"
#include "Themes/CatppuccinMocha.h"
#include "Themes/DarkIslands.h"
#include "Themes/LightIslands.h"

namespace Themes
{
	namespace
	{
		using ThemeMap = std::vector<ColorEntry>;

		struct Theme
		{
			std::string Name;
			ThemeMap    Colors;
			ThemeID     ID;

			uint32_t GetColor( const uint32_t Hash )
			{
				const auto Iterator = std::ranges::lower_bound( Colors, Hash, {}, &ColorEntry::first );

				if ( Iterator != Colors.end() && Iterator->first == Hash ) return Iterator->second;

				return Graphics::RGBA( 255, 0, 0, 255 ); // Should make it pretty obvious if something is missing
			}
		};

		template <size_t N>
		ThemeMap MakeThemeMap( const std::array<ColorEntry, N>& Entries )
		{
			ThemeMap Map( Entries.begin(), Entries.end() );
			std::ranges::sort( Map, {}, &ColorEntry::first );
			return Map;
		}

		auto                                                     Active = ThemeID::DarkIslands;
		std::array<Theme, static_cast<size_t>( ThemeID::Count )> Themes = {};
	}


	uint32_t Manager::GetColor( const uint32_t Hash )
	{
		return Themes[ static_cast<size_t>( Active ) ].GetColor( Hash );
	}

	void Manager::SetTheme( const ThemeID NewTheme )
	{
		Active = NewTheme;
	}

	ThemeID Manager::GetTheme()
	{
		return Active;
	}

	void Manager::Initialize()
	{
		Themes[ static_cast<size_t>( ThemeID::DarkIslands ) ] = Theme{
			.Name = _( "Dark Islands" ),
			.Colors = MakeThemeMap( Colors::DarkIslands ),
			.ID = ThemeID::DarkIslands
		};

		Themes[ static_cast<size_t>( ThemeID::LightIslands ) ] = Theme{
			.Name = _( "Light Islands" ),
			.Colors = MakeThemeMap( Colors::LightIslands ),
			.ID = ThemeID::LightIslands
		};

		Themes[ static_cast<size_t>( ThemeID::CatppuccinLatte ) ] = Theme{
			.Name = _( "Catppuccin Latte" ),
			.Colors = MakeThemeMap( Colors::CatppuccinLatte ),
			.ID = ThemeID::CatppuccinLatte
		};

		Themes[ static_cast<size_t>( ThemeID::CatppuccinFrappe ) ] = Theme{
			.Name = _( "Catppuccin Frappe" ),
			.Colors = MakeThemeMap( Colors::CatppuccinFrappe ),
			.ID = ThemeID::CatppuccinFrappe
		};

		Themes[ static_cast<size_t>( ThemeID::CatppuccinMacchiato ) ] = Theme{
			.Name = _( "Catppuccin Macchiato" ),
			.Colors = MakeThemeMap( Colors::CatppuccinMacchiato ),
			.ID = ThemeID::CatppuccinMacchiato
		};

		Themes[ static_cast<size_t>( ThemeID::CatppuccinMocha ) ] = Theme{
			.Name = _( "Catppuccin Mocha" ),
			.Colors = MakeThemeMap( Colors::CatppuccinMocha ),
			.ID = ThemeID::CatppuccinMocha
		};

		const auto ThemeName = Config::GetOrSet( _( "UI" ), _( "Theme" ), std::string( "Dark Islands" ) );

		const auto Iterator = std::ranges::find( Themes, ThemeName, &Theme::Name );

		if ( Iterator != Themes.end() )
		{
			SetTheme( Iterator->ID );
		}
	}
}
