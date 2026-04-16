#pragma once
#include "Libraries/Vector.h"
#include <Windows.h>

#include "Components/Layout/Objects/Button/Button.h"
#include "Components/Layout/Objects/Search Bar/SearchBar.h"

namespace Renderer::Layout
{
	class TypePicker
	{
	public:
		static void Render( Vector2f Cursor, Vector2f AvailableSize );
		static bool WndProc( Vector2f CursorPosition, UINT Message, WPARAM wParam, LPARAM lParam );
		static void OnSelect( Vector2f AvailableSize );
		static void OnTypedButtonTrigger( Objects::Button* Instance );
	};
}
