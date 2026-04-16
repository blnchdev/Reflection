#pragma once
#include <array>

#include "Components/Theme Manager/Theme Manager.h"
#include "Components/Graphics/Graphics.h"

namespace Colors
{
	using namespace Themes;

	// Catppuccin Macchiato
	// https://catppuccin.com/palette
	inline constexpr std::array CatppuccinMacchiato =
	{
		// Window
		ColorEntry{ "Global_Border"_H, Graphics::RGBA( 91, 96, 120, 255 ) },
		ColorEntry{ "Global_WindowBackground"_H, Graphics::RGBA( 36, 39, 58, 255 ) },
		ColorEntry{ "Global_PopupBackground"_H, Graphics::RGBA( 30, 32, 48, 255 ) },
		ColorEntry{ "Global_Text"_H, Graphics::RGBA( 202, 211, 245, 255 ) },
		ColorEntry{ "Global_MutedText"_H, Graphics::RGBA( 128, 135, 162, 255 ) },
		ColorEntry{ "Global_ActiveText"_H, Graphics::RGBA( 244, 219, 214, 255 ) },

		// Button
		ColorEntry{ "Button_Idle"_H, Graphics::RGBA( 54, 58, 79, 255 ) },
		ColorEntry{ "Button_Hover"_H, Graphics::RGBA( 73, 77, 100, 120 ) },
		ColorEntry{ "Button_Click"_H, Graphics::RGBA( 91, 96, 120, 160 ) },
		ColorEntry{ "Button_Outline"_H, Graphics::RGBA( 138, 173, 244, 255 ) },

		// TitleBar
		ColorEntry{ "TitleBar_Background"_H, Graphics::RGBA( 30, 32, 48, 255 ) },
		ColorEntry{ "TitleBar_WatermarkA"_H, Graphics::RGBA( 139, 213, 202, 255 ) },
		ColorEntry{ "TitleBar_WatermarkB"_H, Graphics::RGBA( 125, 196, 228, 255 ) },
		ColorEntry{ "TitleBar_Button_Background"_H, Graphics::RGBA( 73, 77, 100, 255 ) },
		ColorEntry{ "TitleBar_Button_Close"_H, Graphics::RGBA( 237, 135, 150, 255 ) },
		ColorEntry{ "TitleBar_Button_Maximize"_H, Graphics::RGBA( 238, 212, 159, 255 ) },
		ColorEntry{ "TitleBar_Button_Minimize"_H, Graphics::RGBA( 166, 218, 149, 255 ) },
		ColorEntry{ "TitleBar_Button_Hover"_H, Graphics::RGBA( 110, 115, 141, 160 ) },

		// Memory Viewer
		ColorEntry{ "MemoryGrid_TabBackground"_H, Graphics::RGBA( 54, 58, 79, 255 ) },

		// Memory Viewer Types
		ColorEntry{ "MemoryGrid_ColorA"_H, Graphics::RGBA( 73, 77, 100, 255 ) },
		ColorEntry{ "MemoryGrid_Selected"_H, Graphics::RGBA( 138, 173, 244, 140 ) },
		ColorEntry{ "MemoryGrid_Offset"_H, Graphics::RGBA( 238, 153, 160, 255 ) },
		ColorEntry{ "MemoryGrid_VirtualAddress"_H, Graphics::RGBA( 245, 169, 127, 255 ) },
		ColorEntry{ "MemoryGrid_Label"_H, Graphics::RGBA( 138, 173, 244, 255 ) },
		ColorEntry{ "MemoryGrid_PlaceholderLabel"_H, Graphics::RGBA( 128, 135, 162, 255 ) },
		ColorEntry{ "MemoryGrid_Hex"_H, Graphics::RGBA( 202, 211, 245, 255 ) },
		ColorEntry{ "MemoryGrid_Char"_H, Graphics::RGBA( 125, 196, 228, 255 ) },
		ColorEntry{ "MemoryGrid_Formatted"_H, Graphics::RGBA( 198, 160, 246, 255 ) },
		ColorEntry{ "MemoryGrid_Pointer"_H, Graphics::RGBA( 139, 213, 202, 255 ) },
		ColorEntry{ "MemoryGrid_InvalidPointer"_H, Graphics::RGBA( 237, 135, 150, 255 ) },
	};
}
