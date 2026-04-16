#pragma once
#include <array>

#include "Components/Theme Manager/Theme Manager.h"
#include "Definitions/Globals.h"
#include "Components/Graphics/Graphics.h"

namespace Colors
{
	using namespace Themes; // ugh

	inline constexpr std::array DarkIslands = {
		// Window
		ColorEntry{ "Global_Border"_H, Graphics::RGBA( 61, 61, 64, 255 ) },
		ColorEntry{ "Global_WindowBackground"_H, Graphics::RGBA( 18, 18, 22, 255 ) },
		ColorEntry{ "Global_PopupBackground"_H, Graphics::RGBA( 24, 26, 29, 255 ) },
		ColorEntry{ "Global_Text"_H, Graphics::RGBA( 202, 211, 245, 255 ) },
		ColorEntry{ "Global_MutedText"_H, Graphics::RGBA( 132, 138, 158, 255 ) },
		ColorEntry{ "Global_ActiveText"_H, Graphics::RGBA( 218, 219, 224, 255 ) },

		// Add Button (New Tab)
		ColorEntry{ "AddButton_Idle"_H, Graphics::RGBA( 159, 226, 191, 255 ) },
		ColorEntry{ "AddButton_Hover"_H, Graphics::RGBA( 188, 235, 211, 255 ) },
		ColorEntry{ "AddButton_Click"_H, Graphics::RGBA( 212, 242, 226, 255 ) },

		// Button
		ColorEntry{ "Button_Idle"_H, Graphics::RGBA( 28, 30, 36, 255 ) },
		ColorEntry{ "Button_Hover"_H, Graphics::RGBA( 34, 37, 45, 120 ) },
		ColorEntry{ "Button_Click"_H, Graphics::RGBA( 42, 46, 56, 160 ) },
		ColorEntry{ "Button_Outline"_H, Graphics::RGBA( 132, 162, 255, 255 ) },

		// TitleBar
		ColorEntry{ "TitleBar_Background"_H, Graphics::RGBA( 10, 11, 14, 255 ) },
		ColorEntry{ "TitleBar_WatermarkA"_H, Graphics::RGBA( 88, 188, 176, 255 ) },
		ColorEntry{ "TitleBar_WatermarkB"_H, Graphics::RGBA( 102, 136, 230, 255 ) },
		ColorEntry{ "TitleBar_Button_Background"_H, Graphics::RGBA( 72, 78, 96, 255 ) },
		ColorEntry{ "TitleBar_Button_Close"_H, Graphics::RGBA( 214, 96, 112, 255 ) },
		ColorEntry{ "TitleBar_Button_Maximize"_H, Graphics::RGBA( 222, 150, 104, 255 ) },
		ColorEntry{ "TitleBar_Button_Minimize"_H, Graphics::RGBA( 120, 196, 132, 255 ) },
		ColorEntry{ "TitleBar_Button_Hover"_H, Graphics::RGBA( 48, 52, 64, 160 ) },

		// Memory Viewer
		ColorEntry{ "MemoryGrid_TabBackground"_H, Graphics::RGBA( 22, 22, 25, 255 ) },

		// Memory Viewer Types
		ColorEntry{ "MemoryGrid_ColorA"_H, Graphics::RGBA( 38, 41, 50, 255 ) },
		ColorEntry{ "MemoryGrid_Selected"_H, Graphics::RGBA( 96, 104, 128, 140 ) },
		ColorEntry{ "MemoryGrid_Offset"_H, Graphics::RGBA( 210, 102, 118, 255 ) },
		ColorEntry{ "MemoryGrid_VirtualAddress"_H, Graphics::RGBA( 226, 158, 112, 255 ) },
		ColorEntry{ "MemoryGrid_Label"_H, Graphics::RGBA( 77, 200, 255, 255 ) },
		ColorEntry{ "MemoryGrid_PlaceholderLabel"_H, Graphics::RGBA( 132, 138, 158, 255 ) },
		ColorEntry{ "MemoryGrid_Hex"_H, Graphics::RGBA( 214, 218, 235, 255 ) },
		ColorEntry{ "MemoryGrid_Char"_H, Graphics::RGBA( 112, 146, 232, 255 ) },
		ColorEntry{ "MemoryGrid_Formatted"_H, Graphics::RGBA( 168, 136, 232, 255 ) },
		ColorEntry{ "MemoryGrid_Pointer"_H, Graphics::RGBA( 46, 235, 212, 255 ) },
		ColorEntry{ "MemoryGrid_InvalidPointer"_H, Graphics::RGBA( 240, 144, 144, 255 ) },
	};
}
