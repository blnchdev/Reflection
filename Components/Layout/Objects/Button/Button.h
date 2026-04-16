#pragma once
#include <functional>
#include <optional>
#include <Windows.h>

#include "imgui.h"
#include "Components/Theme Manager/Theme Manager.h"
#include "Definitions/Globals.h"
#include "Libraries/Vector.h"

namespace Objects
{
	enum ButtonStatus : uint8_t
	{
		IDLE,
		HOVER,
		CLICK,
		ACTIVE
	};

	enum class ButtonType : uint8_t
	{
		Rectangle,
		Circle
	};

	class Button
	{
	public:
		bool        IsRounded  = false;
		ImDrawFlags Flags      = 0;
		float       Rounding   = Themes::ROUNDING;
		bool        IsToggle   = false;
		bool        AutoResize = true;
		uint32_t    IDX        = 0;

		Vector2f Position = Vector2f::Zero;

		union
		{
			Vector2f Size = Vector2f::Zero;
			float    Radius;
		};

		struct FontData
		{
			std::optional<uint32_t> Idle   = std::nullopt;
			std::optional<uint32_t> Active = std::nullopt;
		} Font;

		ButtonStatus Status   = IDLE;
		ButtonType   Type     = ButtonType::Rectangle;
		float        FontSize = 16.f;

		struct OverrideData
		{
			uint32_t Idle   = 0;
			uint32_t Hover  = 0;
			uint32_t Click  = 0;
			uint32_t Border = 0;
		} Override;

		std::string Label = _( "+" );

		std::function<void( Button* )> Callback = nullptr;

		uint32_t GetActiveColor() const;
		void     Resize();
		void     Render() const;
		bool     WndProc( Vector2f CursorPosition, uint32_t Message, WPARAM wParam, LPARAM lParam );
		Button();
		explicit Button( std::string Label, const std::function<void( Button* )>& Callback, ButtonType Type, const OverrideData& ColorData, const FontData& Font );
	};
}
