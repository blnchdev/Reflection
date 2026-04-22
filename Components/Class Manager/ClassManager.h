#pragma once
#include <filesystem>

#include "Components/Memory Manager/MemoryStructs.h"


namespace ClassManager
{
	struct ClassFile
	{
		ClassFile( const std::filesystem::path& RootPath, std::filesystem::path FilePath, const std::optional<Memory::Info>& Template = std::nullopt );

		void Parse() const;

		bool Save() const;

		std::shared_ptr<Memory::Info> BackingData = nullptr;
		std::filesystem::path         Absolute    = {};
		std::filesystem::path         Relative    = {};
	};

	bool IsBacked( const std::shared_ptr<Memory::Info>& Template );

	std::filesystem::path         GetRootPath();
	const std::vector<ClassFile>& GetBackedFiles();

	std::shared_ptr<Memory::Info> GetEmptyView();
	std::shared_ptr<Memory::Info> InstantiateNonBacked( size_t Size );
	std::shared_ptr<Memory::Info> GetInstanceFromPath( std::string_view Path );
	std::shared_ptr<Memory::Info> GetInstanceFromName( std::string_view Name );

	bool BackAndSave( const std::shared_ptr<Memory::Info>& Template, std::string_view Path );
	bool SaveToFile( const std::shared_ptr<Memory::Info>& Template );


	void AddToManager( Memory::Info* Template, std::string_view Path, bool IsCompileTimeTemplate = false );
	void Initialize();
}
