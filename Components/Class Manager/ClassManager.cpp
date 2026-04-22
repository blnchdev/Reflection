#include "ClassManager.h"

#include <fstream>
#include <string>
#include <string_view>
#include <print>

#include "Templates.h"
#include "Components/Layout/Dissector/ToolBar/ClassBrowser/ClassBrowser.h"
#include "Components/Memory Manager/MemoryStructs.h"
#include "Components/Notifications/Notifications.h"

#include "Libraries/nlohmann/json.hpp"

using json = nlohmann::ordered_json;

namespace ClassManager
{
	using namespace Memory;

	namespace
	{
		int64_t CurrentID = 0;

		// TODO: Favorites/Favorite Directories (store in Config)
		// TODO: Search Bar
		// TODO: Notification on load/save
		// TODO: Think about this implementation carefully

		// Forward Declarations
		std::optional<std::string> GetPathByTemplateName( std::string_view Name );

		void FieldToJSON( json& J, const Field& Field )
		{
			J =
			{
				{ "Type", static_cast<std::underlying_type_t<DataStructureType>>( Field.Type ) }, // Frankly this'll stay uint8_t forever, but it's kinda cool to use std::underlying_type_t lol
				{ "Offset", Field.Offset },
				{ "Name", Field.Name },
				{ "Size", Field.Size },
			};

			if ( std::holds_alternative<std::shared_ptr<Info>>( Field.Embedded ) )
			{
				auto& EmbeddedInfo = std::get<std::shared_ptr<Info>>( Field.Embedded );
				J[ "Embedded" ]    = GetPathByTemplateName( EmbeddedInfo->Label );
			}
			else
			{
				J[ "Embedded" ] = nlohmann::json( nullptr );
			}
		}

		void TemplateToJSON( json& J, const Info* Template )
		{
			J =
			{
				{ "Name", Template->Label }
			};

			json Buffer = {};
			json Fields = json::array();

			auto PushBack = [&] ( const Field& F )
			{
				FieldToJSON( Buffer, F );
				Fields.emplace_back( std::move( Buffer ) );
			};

			std::ranges::for_each( Template->Fields, PushBack );

			J[ "Fields" ] = std::move( Fields );
		}

		void JSONToField( const json& J, Field& Field )
		{
			J[ "Type" ].get_to( Field.Type );
			J[ "Offset" ].get_to( Field.Offset );
			J[ "Name" ].get_to( Field.Name );
			J[ "Size" ].get_to( Field.Size );

			if ( J[ "Embedded" ].is_null() )
			{
				Field.Embedded = _( "RESERVED_NONE" );
			}
			else
			{
				Field.Embedded = J[ "Embedded" ].get<std::string>();
			}
		}

		void JSONToTemplate( const json& J, Info* Template )
		{
			J[ "Name" ].get_to( Template->Label );

			json Array = J[ "Fields" ];

			for ( const auto& Entry : Array )
			{
				Field F;
				JSONToField( Entry, F );
				Template->Fields.push_back( F );
			}
		}

		struct ClassDirectory
		{
			explicit ClassDirectory( std::filesystem::path Root ) : Path( std::move( Root ) )
			{
				if ( !std::filesystem::is_directory( Path ) )
				{
					Renderer::NotificationManager::AddNotification( _( "Error with ClassDirectory" ), _( "Root is not a directory. Classes can not be saved or loaded." ), Renderer::Error, true );
					return;
				}
			}

			void Iterate()
			{
				for ( const auto& File : std::filesystem::recursive_directory_iterator{ Path } )
				{
					if ( !File.is_regular_file() || File.path().extension() != ".json" ) continue;

					auto& Entry = Files.emplace_back( Path, File.path() );
					Entry.Parse();
				}
			}

			std::vector<ClassFile> Files = {};
			std::filesystem::path  Path  = {};
		};

		std::unique_ptr<ClassDirectory>    Root      = nullptr;
		std::vector<std::shared_ptr<Info>> NonBacked = {};
		std::shared_ptr<Info>              Dummy     = std::make_shared<Info>( Templates::Empty );

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

	ClassFile::ClassFile( const std::filesystem::path& RootPath, std::filesystem::path FilePath, const std::optional<Memory::Info>& Template ) : Absolute( std::move( FilePath ) ), Relative( Absolute.lexically_relative( RootPath ) )
	{
		if ( Template.has_value() )
		{
			this->BackingData = std::make_shared<Info>( Template.value() );
		}
		else
		{
			this->BackingData = std::make_shared<Info>( Templates::Empty );
		}
	}

	void ClassFile::Parse() const
	{
		std::ifstream File( Absolute );
		if ( !File.is_open() ) return; // TODO: Log

		const json J = nlohmann::json::parse( File, nullptr, false );

		if ( J.is_discarded() ) return; // TODO: Log

		JSONToTemplate( J, BackingData.get() );
	}

	bool ClassFile::Save() const
	{
		json J = {};
		TemplateToJSON( J, BackingData.get() );
		std::ofstream File( Absolute );

		if ( !File.is_open() ) return false;

		File << J.dump( 2 );

		return true;
	}

	bool IsBacked( const std::shared_ptr<Info>& Template )
	{
		const auto Iterator = std::ranges::find( NonBacked, Template->ID, &Info::ID );

		return Iterator == NonBacked.end();
	}

	std::filesystem::path GetRootPath()
	{
		return Root->Path;
	}

	const std::vector<ClassFile>& GetBackedFiles()
	{
		return Root->Files;
	}

	std::shared_ptr<Info> GetEmptyView()
	{
		return Dummy;
	}

	std::shared_ptr<Info> InstantiateNonBacked( const size_t Size )
	{
		++CurrentID;

		auto Instance  = std::make_shared<Info>( Templates::Empty );
		Instance->Size = Size;
		Instance->ID   = CurrentID;

		NonBacked.push_back( Instance );

		return Instance;
	}

	std::shared_ptr<Info> GetInstanceFromPath( const std::string_view Path )
	{
		if ( !Root ) return nullptr;

		auto TryMatch = [&] ( const ClassFile& File )
		{
			return File.BackingData && File.Relative.string() == Path;
		};

		const auto Iterator = std::ranges::find_if( Root->Files, TryMatch );

		if ( Iterator == Root->Files.end() ) return nullptr;

		return Iterator->BackingData;
	}

	std::shared_ptr<Info> GetInstanceFromName( const std::string_view Name )
	{
		if ( !Root ) return nullptr;

		auto TryMatch = [&] ( const ClassFile& File )
		{
			return File.BackingData && Name == File.BackingData->Label;
		};

		const auto Iterator = std::ranges::find_if( Root->Files, TryMatch );

		if ( Iterator == Root->Files.end() ) return nullptr;

		return Iterator->BackingData;
	}

	// TODO: This code is pretty similar to AddToManager
	bool BackAndSave( const std::shared_ptr<Info>& Template, const std::string_view Path )
	{
		if ( !Root ) return false;

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

		// TODO: Match FilePath here and ask the user if they want to override a saved class or not
		// ^ this probably entails changing the return value to a status, and having a bool argument 'OverwriteAllowed' defaulted to false?

		ClassFile Instance = { Root->Path, ActualPath, *Template };

		Root->Files.push_back( std::move( Instance ) );

		const auto SubRange = std::ranges::remove( NonBacked, Template->ID, &Info::ID );
		NonBacked.erase( SubRange.begin(), SubRange.end() );

		return true;
	}

	bool SaveToFile( const std::shared_ptr<Info>& Template )
	{
		if ( !Root ) return false;

		const uint64_t ToMatch = Template->ID;

		auto TryMatch = [&] ( const ClassFile& File )
		{
			return File.BackingData->ID == ToMatch;
		};

		const auto Iterator = std::ranges::find_if( Root->Files, TryMatch );

		if ( Iterator == Root->Files.end() ) return false;

		return Iterator->Save();
	}

	void AddToManager( Info* Template, const std::string_view Path, [[maybe_unused]] bool IsCompileTimeTemplate )
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

		const auto Iterator = std::ranges::find( Root->Files, FilePath, &ClassFile::Absolute );

		// Already exist, don't push the same file multiple times
		if ( Iterator != Root->Files.end() )
		{
			++CurrentID;
			Iterator->BackingData->ID   = CurrentID;
			Iterator->BackingData->Size = Template->Size;
			return;
		}

		ClassFile Instance = { Root->Path, FilePath, *Template };

		++CurrentID;
		Instance.BackingData->ID = CurrentID;

		if ( std::filesystem::exists( FilePath ) )
		{
			Instance.Parse();
			Root->Files.push_back( std::move( Instance ) );
		}
		else
		{
			( void )Instance.Save();
			Root->Files.push_back( std::move( Instance ) );
		}
	}

	void Initialize()
	{
		auto Path = GetRoot();

		if ( !Path.has_value() ) return;

		Root = std::make_unique<ClassDirectory>( Path.value() );
		Root->Iterate();

		// Do we actually want compile-time templates to be saved to file?
		// These should honestly be non-backed? %appdata%/Reflection/Classes should probably be exclusively user-defined
		AddToManager( &Templates::LIST_ENTRY, "/Win32/Types/", true );
		AddToManager( &Templates::PEB_LDR_DATA, "/Win32/Usermode/", true );

		AddToManager( &Templates::CURDIR, "/Win32/Types/", true );
		AddToManager( &Templates::UNICODE_STRING, "/Win32/Types/", true );
		AddToManager( &Templates::RTL_DRIVE_LETTER_CURDIR, "/Win32/Types/", true );
		AddToManager( &Templates::RTL_USER_PROCESS_PARAMETERS, "/Win32/Usermode/", true );

		AddToManager( &Templates::PEB, "/Win32/Usermode/", true );
		AddToManager( &Templates::KERNEL_CALLBACK_TABLE, "/Win32/Usermode/", true );

		auto Flatten = [&] ( Field& Field )
		{
			if ( !std::holds_alternative<std::string>( Field.Embedded ) )
			{
				return;
			}

			const auto& String = std::get<std::string>( Field.Embedded );

			if ( String == _( "RESERVED_NONE" ) ) return;

			Field.Embedded = GetInstanceFromName( String );

			if ( Field.Type == T_EmbeddedClass )
			{
				const auto& EmbeddedData = std::get<std::shared_ptr<Info>>( Field.Embedded );
				Field.Size               = EmbeddedData ? EmbeddedData->Size : 8;
			}
		};

		auto Populate = [&] ( const ClassFile& File )
		{
			if ( !File.BackingData ) return;

			std::ranges::for_each( File.BackingData->Fields, Flatten );
		};

		std::ranges::for_each( Root->Files, Populate );

		Renderer::Layout::ClassBrowser::Initialize();
	}
}
