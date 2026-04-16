#include "Dissector.h"

#include <numbers>

#include "Components/Graphics/Graphics.h"
#include "Components/Theme Manager/Theme Manager.h"
#include "Components/Title Bar/TitleBar.h"
#include "Grid/Grid.h"
#include "ToolBar/SideBar.h"

namespace Renderer::Layout
{
	namespace
	{
		Vector2f GridSize = Vector2f::Zero;
	}

	Vector2f Dissector::GetGridSize()
	{
		return GridSize;
	}

	void Dissector::Present( const float Opacity )
	{
		constexpr float PADDING      = 10.f;
		const auto      WindowData   = Window::GetWindowData();
		const Vector2f  TitleBarSize = Titlebar::GetSize();
		Vector2f        Cursor       = { PADDING, TitleBarSize.y + PADDING / 2.f };
		Vector2f        ToolBarSize  = SideBar::GetSize();

		const auto BackgroundColor = Themes::Manager::GetColor( "Global_WindowBackground"_H );
		const auto BorderColor     = Themes::Manager::GetColor( "Global_Border"_H );
		const auto PopupBackground = Themes::Manager::GetColor( "Global_PopupBackground"_H );

		GridSize = { WindowData->Width - ToolBarSize.x - PADDING * 2.f, WindowData->Height - TitleBarSize.y - PADDING };

		SideBar::Present( Cursor, Opacity, GridSize.y );

		// Grid
		{
			Vector2f        RectMin  = { Cursor.x, Cursor.y };
			Vector2f        RectMax  = { Cursor.x + GridSize.x, Cursor.y + GridSize.y };
			constexpr float Rounding = Themes::ROUNDING;

			auto* DrawList = ImGui::GetBackgroundDrawList();

			ImGui::PushClipRect( *reinterpret_cast<ImVec2*>( &RectMin ), *reinterpret_cast<ImVec2*>( &RectMax ), true );
			DrawList->PushClipRect( *reinterpret_cast<ImVec2*>( &RectMin ), *reinterpret_cast<ImVec2*>( &RectMax ), true );

			Graphics::AddFilledRectangle( RectMin, GridSize.x, GridSize.y, PopupBackground, Rounding, ImDrawFlags_RoundCornersAll ); // Base for Memory Grid

			Grid::Present( Cursor, GridSize, Opacity );

			DrawList->PopClipRect();
			ImGui::PopClipRect();

			auto DrawCorner = [DrawList, Rounding] ( Vector2f Point, const float X, const float Y, const float A0, const float A1, const uint32_t Color )
			{
				auto C = Vector2f( Point.x + Rounding * X, Point.y + Rounding * Y );

				DrawList->PathLineTo( *reinterpret_cast<ImVec2*>( &Point ) );
				DrawList->PathArcTo( *reinterpret_cast<ImVec2*>( &C ), Rounding, A0, A1 );

				DrawList->PathFillConvex( Color );
			};

			DrawCorner( RectMin, 1.f, 1.f, std::numbers::pi_v<float>, std::numbers::pi_v<float> * 1.5f, BackgroundColor );                                 // TL
			DrawCorner( Vector2f( RectMax.x, RectMin.y ), -1.f, 1.f, std::numbers::pi_v<float> * 1.5f, std::numbers::pi_v<float> * 2.f, BackgroundColor ); // TR
			DrawCorner( Vector2f( RectMin.x, RectMax.y ), 1.f, -1.f, std::numbers::pi_v<float> * .5f, std::numbers::pi_v<float>, BackgroundColor );        // BL
			DrawCorner( RectMax, -1.f, -1.f, 0.f, std::numbers::pi_v<float> * .5f, BackgroundColor );                                                      // BR

			Graphics::AddRectangle( RectMin - 1.f, GridSize.x + 2.f, GridSize.y + 2.f, BorderColor, 1.f, Rounding, ImDrawFlags_RoundCornersAll );
		}
	}

	bool Dissector::WndProc( const Vector2f Position, const uint32_t Message, const WPARAM wParam, const LPARAM lParam )
	{
		if ( SideBar::WndProc( Position, Message, wParam, lParam ) ) return true;
		if ( Grid::WndProc( Position, Message, wParam, lParam ) ) return true;

		return false;
	}
}
