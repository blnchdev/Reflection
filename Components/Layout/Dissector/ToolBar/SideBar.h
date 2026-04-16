#pragma once
#include <Windows.h>

#include "Components/Layout/Objects/Button/Button.h"
#include "Components/Layout/Objects/Search Bar/SearchBar.h"
#include "Components/Memory Manager/MemoryDefs.h"
#include "Components/Memory Manager/MemoryStructs.h"
#include "Libraries/Vector.h"

namespace Renderer::Layout
{
	class SideBar
	{
	public:
		static void     Present( Vector2f& Cursor, float Opacity, float Height );
		static bool     WndProc( Vector2f Position, uint32_t Message, WPARAM wParam, LPARAM lParam );
		static Vector2f GetSize();
		static void     OnButtonCallback( const Objects::Button* Instance );
	};
}
