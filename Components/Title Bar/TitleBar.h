#pragma once
#include <Windows.h>

#include "Libraries/Vector.h"

namespace Renderer
{
	class Titlebar
	{
	public:
		static void     Render();
		static LRESULT  HandleNcHitTest( LPARAM lParam );
		static void     WndProc( UINT Message );
		static Vector2f GetSize();
	};
}
