#pragma once
#include <filesystem>

#include "Libraries/toml.hpp"

namespace Config
{
	struct File
	{
		std::filesystem::path Path  = {};
		toml::table           Table = {};

		File();
	};

	File* GetMainConfig();
	void  Initialize();

	template <typename T>
	T GetOrSet( std::string_view Section, std::string_view Key, T&& Default )
	{
		const auto Main = GetMainConfig();

		if ( !Main ) return T{};

		auto& Table = Main->Table;

		if ( auto* SectionTable = Table[ Section ].as_table() )
		{
			if ( auto Val = ( *SectionTable )[ Key ].value<T>() ) return *Val;
		}

		if ( !Table.contains( Section ) ) Table.insert( Section, toml::table{} );

		Table[ Section ].as_table()->insert_or_assign( Key, std::forward<T>( Default ) );

		std::ofstream File( Main->Path );
		File << Table;

		return Default;
	}

	template <typename T>
	void Set( std::string_view Section, std::string_view Key, T&& Value )
	{
		const auto Main = GetMainConfig();

		if ( !Main ) return;

		auto& Table = Main->Table;

		if ( !Table.contains( Section ) ) Table.insert( Section, toml::table{} );

		Table[ Section ].as_table()->insert_or_assign( Key, std::forward<T>( Value ) );

		std::ofstream File( Main->Path );
		File << Table;
	}
}
