#pragma once
#include <filesystem>

namespace Memory
{
	class Info;
}

namespace ClassManager
{
	bool IsBacked( const std::shared_ptr<Memory::Info>& Template );

	std::shared_ptr<Memory::Info> GetEmptyView();
	std::shared_ptr<Memory::Info> InstantiateNonBacked( size_t Size );
	std::shared_ptr<Memory::Info> GetInstanceFromPath( std::string_view Path );
	std::shared_ptr<Memory::Info> GetInstanceFromName( std::string_view Name );

	bool BackAndSave( const std::shared_ptr<Memory::Info>& Template, std::string_view Path );
	bool SaveToFile( const std::shared_ptr<Memory::Info>& Template );

	void AddToManager( Memory::Info* Template, std::string_view Path, bool IsCompileTimeTemplate = false );
	void Initialize();
}
