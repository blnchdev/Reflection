#pragma once
#include <Windows.h>

#include "Components/Layout/Objects/Button/Button.h"
#include "Libraries/Vector.h"

namespace Renderer::Layout
{
	class Dissector
	{
	public:
		static Vector2f GetGridSize();
		static void     Present( float Opacity );
		static bool     WndProc( Vector2f Position, uint32_t Message, WPARAM wParam, LPARAM lParam );
	};
}
