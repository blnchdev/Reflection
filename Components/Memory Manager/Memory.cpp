#include "Memory.h"

#include "MemoryDefs.h"

namespace Memory
{
	namespace
	{
		auto Instance = Manager();
	}

	Manager* GetManager()
	{
		return &Instance;
	}
}
