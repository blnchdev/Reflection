#include "ClassManager.h"

#include <fstream>
#include <string>
#include <string_view>
#include <print>

#include "Components/Memory Manager/MemoryStructs.h"
#include "Components/Notifications/Notifications.h"

#include "Libraries/nlohmann/json.hpp"

using json = nlohmann::ordered_json;

namespace ClassManager
{
	namespace
	{
		int64_t CurrentID = 0;

		// TODO: Favorites/Favorite Directories (store in Config)
		// TODO: Search Bar
		// TODO: Notification on load/save
		// TODO: Think about this implementation carefully

		// Forward Declarations
		std::optional<std::string> GetPathByTemplateName( std::string_view Name );

		void FieldToJSON( json& J, const Memory::Structs::Field& Field )
		{
			J =
			{
				{ "Type", static_cast<std::underlying_type_t<DataStructureType>>( Field.Type ) }, // Frankly this'll stay uint8_t forever, but it's kinda cool to use std::underlying_type_t lol
				{ "Offset", Field.Offset },
				{ "Name", Field.Name },
				{ "Size", Field.Size },
			};

			if ( Field.EmbeddedInfo ) J[ "Embedded" ] = GetPathByTemplateName( Field.EmbeddedInfo->Label );
			else J[ "Embedded" ]                      = nlohmann::json( nullptr );
		}

		void TemplateToJSON( json& J, const Memory::Structs::Info* Template )
		{
			J =
			{
				{ "Name", Template->Label }
			};

			json Buffer = {};
			json Fields = json::array();

			auto PushBack = [&] ( const Memory::Structs::Field& F )
			{
				FieldToJSON( Buffer, F );
				Fields.emplace_back( std::move( Buffer ) );
			};

			std::ranges::for_each( Template->Fields, PushBack );

			J[ "Fields" ] = std::move( Fields );
		}

		void JSONToField( const json& J, Memory::Structs::Field& Field )
		{
			J[ "Type" ].get_to( Field.Type );
			J[ "Offset" ].get_to( Field.Offset );
			J[ "Name" ].get_to( Field.Name );
			J[ "Size" ].get_to( Field.Size );

			if ( J[ "Embedded" ].is_null() )
			{
				Field.EmbeddedInfo = nullptr;
			}
			else
			{
				// TODO
			}
		}

		void JSONToTemplate( const json& J, Memory::Structs::Info* Template )
		{
			J[ "Name" ].get_to( Template->Label );

			json Array = J[ "Fields" ];

			for ( const auto& Entry : Array )
			{
				Memory::Structs::Field F;
				JSONToField( Entry, F );
				Template->Fields.push_back( F );
			}
		}

		struct ClassFile
		{
			ClassFile( const std::filesystem::path& Root, std::filesystem::path FilePath, const std::optional<Memory::Structs::Info>& Template = std::nullopt ) : Absolute( std::move( FilePath ) ), Relative( Absolute.lexically_relative( Root ) )
			{
				if ( Template.has_value() )
				{
					this->BackingData = std::make_unique<Memory::Structs::Info>( Template.value() );
				}
				else
				{
					this->BackingData = std::make_unique<Memory::Structs::Info>( Memory::Structs::Empty );
				}
			}

			void Parse() const
			{
				std::ifstream File( Absolute );
				if ( !File.is_open() ) return; // TODO: Log

				const json J = nlohmann::json::parse( File, nullptr, false );

				if ( !J.is_discarded() ) return; // TODO: Log

				JSONToTemplate( J, BackingData.get() );
			}

			void Save() const
			{
				json J = {};
				TemplateToJSON( J, BackingData.get() );
				std::ofstream File( Absolute );

				if ( !File.is_open() ) return; // TODO: Log

				File << J.dump( 2 );
			}

			std::unique_ptr<Memory::Structs::Info> BackingData = nullptr;
			std::filesystem::path                  Absolute    = {};
			std::filesystem::path                  Relative    = {};
		};

		struct ClassDirectory
		{
			explicit ClassDirectory( std::filesystem::path Root ) : Path( std::move( Root ) )
			{
				if ( !std::filesystem::is_directory( Path ) )
				{
					Renderer::NotificationManager::AddNotification( _( "Error with ClassDirectory" ), _( "Root is not a directory. Classes can not be saved or loaded." ), Renderer::Error, true );
					return;
				}

				for ( const auto& File : std::filesystem::recursive_directory_iterator{ Path } )
				{
					if ( !File.is_regular_file() || File.path().extension() != ".json" ) continue;

					Files.emplace_back( File.path(), Path );
				}
			}


			std::vector<ClassFile> Files = {};
			std::filesystem::path  Path  = {};
		};

		std::unique_ptr<ClassDirectory> Root = nullptr;

		std::optional<std::filesystem::path> GetRoot()
		{
			char*  AppData   = nullptr;
			size_t szAppData = 0;

			const errno_t Error = _dupenv_s( &AppData, &szAppData, _( "APPDATA" ) );

			if ( Error != 0 || !AppData )
			{
				Renderer::NotificationManager::AddNotification( _( "Error with ClassDirectory" ), _( "Could not find APPDATA env. Classes can not be saved or loaded." ), Renderer::Error, true );
				return std::nullopt;
			}

			std::unique_ptr<char, decltype(&std::free)> Guard( AppData, std::free );

			auto Path = std::filesystem::path( AppData ) / _( "Reflection" ) / _( "Classes" );
			std::filesystem::create_directories( Path );

			return Path;
		}

		std::optional<std::string> GetPathByTemplateName( const std::string_view Name )
		{
			if ( !Root ) return std::nullopt;

			auto TryMatch = [&] ( const ClassFile& File )
			{
				return File.BackingData && Name == File.BackingData->Label;
			};

			const auto Iterator = std::ranges::find_if( Root->Files, TryMatch );

			if ( Iterator == Root->Files.end() ) return std::nullopt;

			return Iterator->Relative.string();
		}
	}

	Memory::Structs::Info* GetInstanceFromName( std::string_view Name )
	{
	}

	void AddToManager( Memory::Structs::Info* Template, std::string_view Path, bool IsCompileTimeTemplate )
	{
		if ( !Root ) return;

		const auto Start = Path.find_first_not_of( '/' );
		const auto End   = Path.find_last_not_of( '/' );

		std::filesystem::path ActualPath;

		if ( Start == std::string_view::npos )
		{
			ActualPath = Root->Path;
		}
		else
		{
			ActualPath = Root->Path / Path.substr( Start, End - Start + 1 );
		}

		std::filesystem::create_directories( ActualPath );

		const auto FilePath = ActualPath / ( Template->Label + ".json" );

		ClassFile Instance = { Root->Path, FilePath, *Template };

		++CurrentID;
		Instance.BackingData->ID = CurrentID;

		if ( std::filesystem::exists( FilePath ) )
		{
			Instance.Parse();
		}
		else
		{
			Instance.Save();
			Template = Instance.BackingData.get();
			Root->Files.push_back( std::move( Instance ) );
		}
	}

	void Initialize()
	{
		auto Path = GetRoot();

		if ( !Path.has_value() ) return;

		Root = std::make_unique<ClassDirectory>( Path.value() );

		AddToManager( &Memory::Structs::LIST_ENTRY, "/Win32/Types/", true );
		AddToManager( &Memory::Structs::PEB_LDR_DATA, "/Win32/Usermode/", true );

		AddToManager( &Memory::Structs::CURDIR, "/Win32/Types/", true );
		AddToManager( &Memory::Structs::UNICODE_STRING, "/Win32/Types/", true );
		AddToManager( &Memory::Structs::RTL_DRIVE_LETTER_CURDIR, "/Win32/Types/", true );
		AddToManager( &Memory::Structs::RTL_USER_PROCESS_PARAMETERS, "/Win32/Usermode/", true );

		AddToManager( &Memory::Structs::PEB, "/Win32/Usermode/", true );
		AddToManager( &Memory::Structs::KERNEL_CALLBACK_TABLE, "/Win32/Usermode/", true );

		std::println( "Files Size {}", Root->Files.size() );

		for ( const auto& Entry : Root->Files )
		{
			std::println( "{}", Entry.Relative.string() );
		}
	}
}
