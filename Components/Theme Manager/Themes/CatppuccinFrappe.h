#pragma once
#include <array>

#include "Components/Theme Manager/Theme Manager.h"
#include "Components/Graphics/Graphics.h"

namespace Colors
{
	using namespace Themes;

	// Catppuccin Frappé
	// https://catppuccin.com/palette
	inline constexpr std::array CatppuccinFrappe =
	{
		// Window
		ColorEntry{ "Global_Border"_H, Graphics::RGBA( 98, 104, 128, 255 ) },
		ColorEntry{ "Global_WindowBackground"_H, Graphics::RGBA( 48, 52, 70, 255 ) },
		ColorEntry{ "Global_PopupBackground"_H, Graphics::RGBA( 41, 44, 60, 255 ) },
		ColorEntry{ "Global_Text"_H, Graphics::RGBA( 198, 208, 245, 255 ) },
		ColorEntry{ "Global_MutedText"_H, Graphics::RGBA( 131, 139, 167, 255 ) },
		ColorEntry{ "Global_ActiveText"_H, Graphics::RGBA( 242, 213, 207, 255 ) },

		// Button
		ColorEntry{ "Button_Idle"_H, Graphics::RGBA( 65, 69, 89, 255 ) },
		ColorEntry{ "Button_Hover"_H, Graphics::RGBA( 81, 87, 109, 120 ) },
		ColorEntry{ "Button_Click"_H, Graphics::RGBA( 98, 104, 128, 160 ) },
		ColorEntry{ "Button_Outline"_H, Graphics::RGBA( 140, 170, 238, 255 ) },

		// TitleBar
		ColorEntry{ "TitleBar_Background"_H, Graphics::RGBA( 41, 44, 60, 255 ) },
		ColorEntry{ "TitleBar_WatermarkA"_H, Graphics::RGBA( 129, 200, 190, 255 ) },
		ColorEntry{ "TitleBar_WatermarkB"_H, Graphics::RGBA( 133, 193, 220, 255 ) },
		ColorEntry{ "TitleBar_Button_Background"_H, Graphics::RGBA( 81, 87, 109, 255 ) },
		ColorEntry{ "TitleBar_Button_Close"_H, Graphics::RGBA( 231, 130, 132, 255 ) },
		ColorEntry{ "TitleBar_Button_Maximize"_H, Graphics::RGBA( 229, 200, 144, 255 ) },
		ColorEntry{ "TitleBar_Button_Minimize"_H, Graphics::RGBA( 166, 209, 137, 255 ) },
		ColorEntry{ "TitleBar_Button_Hover"_H, Graphics::RGBA( 115, 121, 148, 160 ) },

		// Memory Viewer
		ColorEntry{ "MemoryGrid_TabBackground"_H, Graphics::RGBA( 65, 69, 89, 255 ) },

		// Memory Viewer Types
		ColorEntry{ "MemoryGrid_ColorA"_H, Graphics::RGBA( 81, 87, 109, 255 ) },
		ColorEntry{ "MemoryGrid_Selected"_H, Graphics::RGBA( 140, 170, 238, 140 ) },
		ColorEntry{ "MemoryGrid_Offset"_H, Graphics::RGBA( 234, 153, 156, 255 ) },
		ColorEntry{ "MemoryGrid_VirtualAddress"_H, Graphics::RGBA( 239, 159, 118, 255 ) },
		ColorEntry{ "MemoryGrid_Label"_H, Graphics::RGBA( 140, 170, 238, 255 ) },
		ColorEntry{ "MemoryGrid_PlaceholderLabel"_H, Graphics::RGBA( 131, 139, 167, 255 ) },
		ColorEntry{ "MemoryGrid_Hex"_H, Graphics::RGBA( 198, 208, 245, 255 ) },
		ColorEntry{ "MemoryGrid_Char"_H, Graphics::RGBA( 133, 193, 220, 255 ) },
		ColorEntry{ "MemoryGrid_Formatted"_H, Graphics::RGBA( 202, 158, 230, 255 ) },
		ColorEntry{ "MemoryGrid_Pointer"_H, Graphics::RGBA( 129, 200, 190, 255 ) },
		ColorEntry{ "MemoryGrid_InvalidPointer"_H, Graphics::RGBA( 231, 130, 132, 255 ) },
	};
}
