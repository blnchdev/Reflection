#pragma once
#include <filesystem>

namespace Memory::Structs
{
	class Info;
}

namespace ClassManager
{
	Memory::Structs::Info* GetInstanceFromName( std::string_view Name );
	void                   AddToManager( Memory::Structs::Info* Template, std::string_view Path, bool IsCompileTimeTemplate = false );
	void                   Initialize();
}
