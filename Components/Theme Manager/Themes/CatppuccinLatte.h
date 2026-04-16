#pragma once
#include <array>

#include "Components/Theme Manager/Theme Manager.h"
#include "Components/Graphics/Graphics.h"

namespace Colors
{
	using namespace Themes;

	// Catppuccin Latte
	// https://catppuccin.com/palette
	inline constexpr std::array CatppuccinLatte =
	{
		// Window
		ColorEntry{ "Global_Border"_H, Graphics::RGBA( 172, 176, 190, 255 ) },
		ColorEntry{ "Global_WindowBackground"_H, Graphics::RGBA( 239, 241, 245, 255 ) },
		ColorEntry{ "Global_PopupBackground"_H, Graphics::RGBA( 230, 233, 239, 255 ) },
		ColorEntry{ "Global_Text"_H, Graphics::RGBA( 76, 79, 105, 255 ) },
		ColorEntry{ "Global_MutedText"_H, Graphics::RGBA( 140, 143, 161, 255 ) },
		ColorEntry{ "Global_ActiveText"_H, Graphics::RGBA( 220, 138, 120, 255 ) },

		// Button
		ColorEntry{ "Button_Idle"_H, Graphics::RGBA( 204, 208, 218, 255 ) },
		ColorEntry{ "Button_Hover"_H, Graphics::RGBA( 188, 192, 204, 120 ) },
		ColorEntry{ "Button_Click"_H, Graphics::RGBA( 172, 176, 190, 160 ) },
		ColorEntry{ "Button_Outline"_H, Graphics::RGBA( 30, 102, 245, 255 ) },

		// TitleBar
		ColorEntry{ "TitleBar_Background"_H, Graphics::RGBA( 230, 233, 239, 255 ) },
		ColorEntry{ "TitleBar_WatermarkA"_H, Graphics::RGBA( 23, 146, 153, 255 ) },
		ColorEntry{ "TitleBar_WatermarkB"_H, Graphics::RGBA( 32, 159, 181, 255 ) },
		ColorEntry{ "TitleBar_Button_Background"_H, Graphics::RGBA( 188, 192, 204, 255 ) },
		ColorEntry{ "TitleBar_Button_Close"_H, Graphics::RGBA( 210, 15, 57, 255 ) },
		ColorEntry{ "TitleBar_Button_Maximize"_H, Graphics::RGBA( 223, 142, 29, 255 ) },
		ColorEntry{ "TitleBar_Button_Minimize"_H, Graphics::RGBA( 64, 160, 43, 255 ) },
		ColorEntry{ "TitleBar_Button_Hover"_H, Graphics::RGBA( 156, 160, 176, 160 ) },

		// Memory Viewer
		ColorEntry{ "MemoryGrid_TabBackground"_H, Graphics::RGBA( 204, 208, 218, 255 ) },

		// Memory Viewer Types
		ColorEntry{ "MemoryGrid_ColorA"_H, Graphics::RGBA( 188, 192, 204, 255 ) },
		ColorEntry{ "MemoryGrid_Selected"_H, Graphics::RGBA( 30, 102, 245, 140 ) },
		ColorEntry{ "MemoryGrid_Offset"_H, Graphics::RGBA( 230, 69, 83, 255 ) },
		ColorEntry{ "MemoryGrid_VirtualAddress"_H, Graphics::RGBA( 254, 100, 11, 255 ) },
		ColorEntry{ "MemoryGrid_Label"_H, Graphics::RGBA( 30, 102, 245, 255 ) },
		ColorEntry{ "MemoryGrid_PlaceholderLabel"_H, Graphics::RGBA( 140, 143, 161, 255 ) },
		ColorEntry{ "MemoryGrid_Hex"_H, Graphics::RGBA( 76, 79, 105, 255 ) },
		ColorEntry{ "MemoryGrid_Char"_H, Graphics::RGBA( 32, 159, 181, 255 ) },
		ColorEntry{ "MemoryGrid_Formatted"_H, Graphics::RGBA( 136, 57, 239, 255 ) },
		ColorEntry{ "MemoryGrid_Pointer"_H, Graphics::RGBA( 23, 146, 153, 255 ) },
		ColorEntry{ "MemoryGrid_InvalidPointer"_H, Graphics::RGBA( 210, 15, 57, 255 ) },
	};
}
