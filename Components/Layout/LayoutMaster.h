#pragma once
#include <cstdint>
#include <Windows.h>

#include "Libraries/Vector.h"

namespace Renderer
{
	enum class LayoutStage : uint8_t
	{
		Loading, // Unused for now
		Dissector,
		Settings,

		SIZE_
	};

	class LayoutMaster
	{
	public:
		static void Present();
		static bool WndProc( Vector2f Position, uint32_t Message, WPARAM wParam, LPARAM lParam );
		static void SetOpacity( LayoutStage Layer, float Opacity ); // This instantly changes the opacity of a given layer, for normal transitions use LayoutMaster::Transition
	};
}
