#pragma once
#include <array>

#include "Components/Theme Manager/Theme Manager.h"
#include "Components/Graphics/Graphics.h"

namespace Colors
{
	using namespace Themes;

	// Catppuccin Mocha
	// https://catppuccin.com/palette
	inline constexpr std::array CatppuccinMocha =
	{
		// Window
		ColorEntry{ "Global_Border"_H, Graphics::RGBA( 88, 91, 112, 255 ) },
		ColorEntry{ "Global_WindowBackground"_H, Graphics::RGBA( 30, 30, 46, 255 ) },
		ColorEntry{ "Global_PopupBackground"_H, Graphics::RGBA( 24, 24, 37, 255 ) },
		ColorEntry{ "Global_Text"_H, Graphics::RGBA( 205, 214, 244, 255 ) },
		ColorEntry{ "Global_MutedText"_H, Graphics::RGBA( 127, 132, 156, 255 ) },
		ColorEntry{ "Global_ActiveText"_H, Graphics::RGBA( 245, 224, 220, 255 ) },

		// Button
		ColorEntry{ "Button_Idle"_H, Graphics::RGBA( 49, 50, 68, 255 ) },
		ColorEntry{ "Button_Hover"_H, Graphics::RGBA( 69, 71, 90, 120 ) },
		ColorEntry{ "Button_Click"_H, Graphics::RGBA( 88, 91, 112, 160 ) },
		ColorEntry{ "Button_Outline"_H, Graphics::RGBA( 137, 180, 250, 255 ) },

		// TitleBar
		ColorEntry{ "TitleBar_Background"_H, Graphics::RGBA( 24, 24, 37, 255 ) },
		ColorEntry{ "TitleBar_WatermarkA"_H, Graphics::RGBA( 148, 226, 213, 255 ) },
		ColorEntry{ "TitleBar_WatermarkB"_H, Graphics::RGBA( 116, 199, 236, 255 ) },
		ColorEntry{ "TitleBar_Button_Background"_H, Graphics::RGBA( 69, 71, 90, 255 ) },
		ColorEntry{ "TitleBar_Button_Close"_H, Graphics::RGBA( 243, 139, 168, 255 ) },
		ColorEntry{ "TitleBar_Button_Maximize"_H, Graphics::RGBA( 249, 226, 175, 255 ) },
		ColorEntry{ "TitleBar_Button_Minimize"_H, Graphics::RGBA( 166, 227, 161, 255 ) },
		ColorEntry{ "TitleBar_Button_Hover"_H, Graphics::RGBA( 108, 112, 134, 160 ) },

		// Memory Viewer
		ColorEntry{ "MemoryGrid_TabBackground"_H, Graphics::RGBA( 49, 50, 68, 255 ) },

		// Memory Viewer Types
		ColorEntry{ "MemoryGrid_ColorA"_H, Graphics::RGBA( 69, 71, 90, 255 ) },
		ColorEntry{ "MemoryGrid_Selected"_H, Graphics::RGBA( 137, 180, 250, 140 ) },
		ColorEntry{ "MemoryGrid_Offset"_H, Graphics::RGBA( 235, 160, 172, 255 ) },
		ColorEntry{ "MemoryGrid_VirtualAddress"_H, Graphics::RGBA( 250, 179, 135, 255 ) },
		ColorEntry{ "MemoryGrid_Label"_H, Graphics::RGBA( 137, 180, 250, 255 ) },
		ColorEntry{ "MemoryGrid_PlaceholderLabel"_H, Graphics::RGBA( 127, 132, 156, 255 ) },
		ColorEntry{ "MemoryGrid_Hex"_H, Graphics::RGBA( 205, 214, 244, 255 ) },
		ColorEntry{ "MemoryGrid_Char"_H, Graphics::RGBA( 116, 199, 236, 255 ) },
		ColorEntry{ "MemoryGrid_Formatted"_H, Graphics::RGBA( 203, 166, 247, 255 ) },
		ColorEntry{ "MemoryGrid_Pointer"_H, Graphics::RGBA( 148, 226, 213, 255 ) },
		ColorEntry{ "MemoryGrid_InvalidPointer"_H, Graphics::RGBA( 243, 139, 168, 255 ) },
	};
}
