#pragma once
#include <functional>
#include <optional>
#include <Windows.h>

#include "Definitions/Globals.h"
#include "Libraries/Vector.h"

namespace Objects
{
	enum SearchBarStatus : uint8_t
	{
		SB_IDLE,
		SB_HOVER,
		SB_ACTIVE,
		SB_DISABLED
	};

	class SearchBar
	{
	public:
		bool     AutoResize = true;
		uint32_t IDX        = 0;

		Vector2f Position = Vector2f::Zero;
		Vector2f Size     = Vector2f::Zero;

		struct FontData
		{
			uint32_t                Idle   = 0;
			std::optional<uint32_t> Active = std::nullopt;
		} Font;

		SearchBarStatus Status   = SB_IDLE;
		float           FontSize = 16.f;

		struct OverrideData
		{
			uint32_t Idle        = 0;
			uint32_t Hover       = 0;
			uint32_t Active      = 0;
			uint32_t Border      = 0;
			uint32_t Placeholder = 0;
		} Override;

		std::string Placeholder = _( "Search..." );
		std::string Query;

		std::function<void( SearchBar* )> OnChange  = nullptr;
		std::function<void( SearchBar* )> OnConfirm = nullptr;

		uint32_t GetActiveColor() const;
		void     Resize();
		void     Render();
		bool     WndProc( Vector2f CursorPosition, uint32_t Message, WPARAM wParam, LPARAM lParam );
		SearchBar();
		explicit SearchBar( std::string Placeholder, const std::function<void( SearchBar* )>& OnChange, const std::function<void( SearchBar* )>& OnConfirm, const OverrideData& ColorData, const FontData& Font );
	};
}
