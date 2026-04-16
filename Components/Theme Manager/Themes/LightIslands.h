#pragma once
#include <array>

#include "Components/Theme Manager/Theme Manager.h"
#include "Components/Graphics/Graphics.h"

namespace Colors
{
	using namespace Themes;

	inline constexpr std::array LightIslands = {
		// Window
		ColorEntry{ "Global_Border"_H, Graphics::RGBA( 200, 196, 176, 255 ) },
		ColorEntry{ "Global_WindowBackground"_H, Graphics::RGBA( 245, 242, 236, 255 ) },
		ColorEntry{ "Global_PopupBackground"_H, Graphics::RGBA( 237, 234, 227, 255 ) },
		ColorEntry{ "Global_Text"_H, Graphics::RGBA( 60, 56, 48, 255 ) },
		ColorEntry{ "Global_MutedText"_H, Graphics::RGBA( 148, 144, 132, 255 ) },
		ColorEntry{ "Global_ActiveText"_H, Graphics::RGBA( 30, 28, 23, 255 ) },

		// Add Button (New Tab)
		ColorEntry{ "AddButton_Idle"_H, Graphics::RGBA( 159, 226, 191, 255 ) },
		ColorEntry{ "AddButton_Hover"_H, Graphics::RGBA( 188, 235, 211, 255 ) },
		ColorEntry{ "AddButton_Click"_H, Graphics::RGBA( 212, 242, 226, 255 ) },

		// Button
		ColorEntry{ "Button_Idle"_H, Graphics::RGBA( 232, 229, 220, 255 ) },
		ColorEntry{ "Button_Hover"_H, Graphics::RGBA( 210, 207, 196, 120 ) },
		ColorEntry{ "Button_Click"_H, Graphics::RGBA( 195, 192, 181, 160 ) },
		ColorEntry{ "Button_Outline"_H, Graphics::RGBA( 75, 114, 204, 255 ) },

		// TitleBar
		ColorEntry{ "TitleBar_Background"_H, Graphics::RGBA( 221, 217, 208, 255 ) },
		ColorEntry{ "TitleBar_WatermarkA"_H, Graphics::RGBA( 26, 158, 142, 255 ) },
		ColorEntry{ "TitleBar_WatermarkB"_H, Graphics::RGBA( 58, 98, 200, 255 ) },
		ColorEntry{ "TitleBar_Button_Background"_H, Graphics::RGBA( 180, 176, 164, 255 ) },
		ColorEntry{ "TitleBar_Button_Close"_H, Graphics::RGBA( 192, 56, 72, 255 ) },
		ColorEntry{ "TitleBar_Button_Maximize"_H, Graphics::RGBA( 190, 120, 48, 255 ) },
		ColorEntry{ "TitleBar_Button_Minimize"_H, Graphics::RGBA( 72, 160, 84, 255 ) },
		ColorEntry{ "TitleBar_Button_Hover"_H, Graphics::RGBA( 160, 157, 148, 160 ) },

		// Memory Viewer
		ColorEntry{ "MemoryGrid_TabBackground"_H, Graphics::RGBA( 232, 229, 220, 255 ) },

		// Memory Viewer Types
		ColorEntry{ "MemoryGrid_ColorA"_H, Graphics::RGBA( 224, 221, 212, 255 ) },
		ColorEntry{ "MemoryGrid_Selected"_H, Graphics::RGBA( 160, 168, 192, 140 ) },
		ColorEntry{ "MemoryGrid_Offset"_H, Graphics::RGBA( 192, 57, 78, 255 ) },
		ColorEntry{ "MemoryGrid_VirtualAddress"_H, Graphics::RGBA( 184, 96, 32, 255 ) },
		ColorEntry{ "MemoryGrid_Label"_H, Graphics::RGBA( 0, 112, 184, 255 ) },
		ColorEntry{ "MemoryGrid_PlaceholderLabel"_H, Graphics::RGBA( 148, 144, 132, 255 ) },
		ColorEntry{ "MemoryGrid_Hex"_H, Graphics::RGBA( 52, 50, 44, 255 ) },
		ColorEntry{ "MemoryGrid_Char"_H, Graphics::RGBA( 56, 96, 192, 255 ) },
		ColorEntry{ "MemoryGrid_Formatted"_H, Graphics::RGBA( 112, 64, 184, 255 ) },
		ColorEntry{ "MemoryGrid_Pointer"_H, Graphics::RGBA( 10, 128, 112, 255 ) },
		ColorEntry{ "MemoryGrid_InvalidPointer"_H, Graphics::RGBA( 192, 64, 64, 255 ) },
	};
}
