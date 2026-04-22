#pragma once
#include <filesystem>

namespace Memory::Structs
{
	class Info;
}

namespace ClassManager
{
	std::shared_ptr<Memory::Structs::Info> GetInstanceFromPath( std::string_view Path );
	std::shared_ptr<Memory::Structs::Info> GetInstanceFromName( std::string_view Name );
	void                                   AddToManager( Memory::Structs::Info* Template, std::string_view Path, bool IsCompileTimeTemplate = false );
	void                                   Initialize();
}
