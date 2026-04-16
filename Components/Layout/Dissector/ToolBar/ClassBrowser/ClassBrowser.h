#pragma once
#include "Libraries/Vector.h"
#include <Windows.h>

#include "Components/Layout/Objects/Button/Button.h"
#include "Components/Layout/Objects/Search Bar/SearchBar.h"
#include "Components/Memory Manager/MemoryStructs.h"

namespace Renderer::Layout
{
	class ClassBrowser
	{
	public:
		static void                   Render( Vector2f Cursor, Vector2f AvailableSize );
		static bool                   WndProc( Vector2f CursorPosition, UINT Message, WPARAM wParam, LPARAM lParam );
		static void                   OnSelect();
		static void                   DrainPending();
		static Memory::Structs::Info* GetInstanceFromSelectedTemplate();
		static void                   Initialize();
	};
}
