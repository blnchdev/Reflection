#include "Config.h"

#include <filesystem>

#include "Definitions/Globals.h"
#include "Libraries/toml.hpp"

namespace Config
{
	namespace
	{
		std::filesystem::path GetConfigPath()
		{
			char*  AppData   = nullptr;
			size_t szAppData = 0;

			const errno_t Error = _dupenv_s( &AppData, &szAppData, _( "APPDATA" ) );

			if ( Error != 0 || !AppData )
			{
				printf( _( "ERROR: Could not find APPDATA env. variable! Config will not be saved!\n" ) );
				return {};
			}

			std::unique_ptr<char, decltype(&std::free)> Guard( AppData, std::free );

			const auto Path = std::filesystem::path( AppData ) / _( "Reflection" );
			std::filesystem::create_directories( Path );

			return Path / _( "settings.toml" );
		}

		std::unique_ptr<File> Main = nullptr;
	}

	File::File() : Path( GetConfigPath() )
	{
		if ( std::filesystem::exists( Path ) )
		{
			Table = toml::parse_file( Path.string() );
		}
	}

	File* GetMainConfig()
	{
		return Main.get();
	}

	void Initialize()
	{
		Main = std::make_unique<File>();
	}
}
