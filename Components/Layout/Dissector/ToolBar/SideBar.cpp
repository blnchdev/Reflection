#include "SideBar.h"

#include "Components/Graphics/Graphics.h"
#include "Components/Layout/Objects/Search Bar/SearchBar.h"
#include "Components/Theme Manager/Theme Manager.h"
#include "AttachPanel/AttachPanel.h"
#include "Components/Layout/Dissector/Grid/Grid.h"
#include "TypePicker/TypePicker.h"

namespace Renderer::Layout
{
	namespace
	{
		constexpr float PADDING = 10.f;

		constexpr float AlwaysDisplayBarWidth = 40.f;
		constexpr float ModalWidth            = 250.f;

		float LastHeight        = 0.f;
		float LastToolBarHeight = AlwaysDisplayBarWidth * 5.f;

		int32_t ActiveIDX = 1; // -1 = None, otherwise Buttons[ActiveIDX]

		constexpr uint32_t EmptyColor = Graphics::RGBA( 255, 0, 0, 0 );

		auto NewTab = Objects::Button( "\xEF\x91\x8D", &SideBar::OnButtonCallback, Objects::ButtonType::Circle, {}, {} );

		std::vector Buttons =
		{
			Objects::Button( "\xEF\x83\x86", &SideBar::OnButtonCallback, Objects::ButtonType::Rectangle, { .Idle = EmptyColor, .Border = EmptyColor }, {} ),
			Objects::Button( "\xEE\xAA\x8B", &SideBar::OnButtonCallback, Objects::ButtonType::Rectangle, { .Idle = EmptyColor, .Border = EmptyColor }, {} ),
			Objects::Button( "\xF3\xB0\xB4\x93", &SideBar::OnButtonCallback, Objects::ButtonType::Rectangle, { .Idle = EmptyColor, .Border = EmptyColor }, {} ),
		};
	}

	void SideBar::Present( Vector2f& Cursor, float Opacity, const float Height )
	{
		LastHeight = Height;

		const auto BackgroundColor = Themes::Manager::GetColor( "Global_WindowBackground"_H );
		const auto BorderColor     = Themes::Manager::GetColor( "Global_Border"_H );
		const auto PopupBackground = Themes::Manager::GetColor( "Global_PopupBackground"_H );

		const auto OriginalY = Cursor.y;

		// New Tab Button
		{
			const auto Idle  = Themes::Manager::GetColor( "AddButton_Idle"_H );
			const auto Hover = Themes::Manager::GetColor( "AddButton_Hover"_H );
			const auto Click = Themes::Manager::GetColor( "AddButton_Click"_H );

			Vector2f LocalCursor = Cursor;
			LocalCursor += AlwaysDisplayBarWidth / 2.f;

			NewTab.Position  = LocalCursor;
			NewTab.Radius    = AlwaysDisplayBarWidth / 2.f;
			NewTab.Override  = { .Idle = Idle, .Hover = Hover, .Click = Click, .Border = BorderColor };
			NewTab.Font.Idle = BackgroundColor; // TODO: White text in Themes maybe?
			NewTab.Render();

			LocalCursor.y += AlwaysDisplayBarWidth / 2.f + PADDING / 2.f;
			Cursor.y = LocalCursor.y;
		}

		// AOD Bar
		{
			// Background
			Graphics::AddFilledRectangle( Cursor, AlwaysDisplayBarWidth, LastToolBarHeight, PopupBackground, 100.f );

			Vector2f LocalCursor = Cursor;
			LocalCursor.y += 15.f;

			const uint32_t ButtonIdle   = Themes::Manager::GetColor( "Global_Text"_H );
			const uint32_t ButtonActive = Themes::Manager::GetColor( "Global_ActiveText"_H );

			for ( auto& Button : Buttons )
			{
				Button.IsToggle = true;

				Button.Font.Idle   = ButtonIdle;
				Button.Font.Active = ButtonActive;
				Button.Position    = LocalCursor;

				Button.Size = Vector2f( AlwaysDisplayBarWidth, AlwaysDisplayBarWidth );
				Button.Render();
				LocalCursor.y += Button.Size.y;
			}

			// Border
			Graphics::AddRectangle( Cursor, AlwaysDisplayBarWidth, LastToolBarHeight, BorderColor, 1.0f, 100.f );

			LastToolBarHeight = LocalCursor.y - Cursor.y + 15.f;
			Cursor.x += PADDING + AlwaysDisplayBarWidth;
		}

		Cursor.y = OriginalY;

		if ( ActiveIDX != -1 )
		{
			Graphics::AddFilledRectangle( Cursor, ModalWidth, Height, PopupBackground, Themes::ROUNDING );
			Graphics::AddRectangle( Cursor, ModalWidth, Height, BorderColor, 1.f, Themes::ROUNDING );

			switch ( ActiveIDX )
			{
			case 0:
				AttachPanel::Render( Cursor, Vector2f( ModalWidth, Height ) );
				break;
			case 2:
				TypePicker::Render( Cursor, Vector2f( ModalWidth, Height ) );
			default:
				break;
			}

			Cursor.x += PADDING + ModalWidth;
		}
	}

	bool SideBar::WndProc( const Vector2f Position, const uint32_t Message, const WPARAM wParam, const LPARAM lParam )
	{
		bool ReturnValue = NewTab.WndProc( Position, Message, wParam, lParam );

		switch ( ActiveIDX )
		{
		case 0:
			ReturnValue |= AttachPanel::WndProc( Position, Message, wParam, lParam );
			break;
		case 2:
			ReturnValue |= TypePicker::WndProc( Position, Message, wParam, lParam );
			break;
		default:
			break;
		}

		for ( auto& Button : Buttons )
		{
			ReturnValue |= Button.WndProc( Position, Message, wParam, lParam );
		}

		return ReturnValue;
	}

	Vector2f SideBar::GetSize()
	{
		Vector2f Return = { AlwaysDisplayBarWidth + PADDING, LastHeight };

		if ( ActiveIDX != -1 ) Return.x += ModalWidth + PADDING;

		return Return;
	}

	void SideBar::OnButtonCallback( const Objects::Button* Instance )
	{
		const uint32_t TargetIDX = Instance->IDX;

		if ( NewTab.IDX == TargetIDX )
		{
			const uint32_t LastID = Process::GetLastAttachedPID();
			Grid::NewTab( _( "Empty" ), LastID, Process::GetProcessBase( LastID ), 256 );
			return;
		}

		for ( auto i{ 0l }; i < Buttons.size(); i++ )
		{
			if ( Buttons[ i ].IDX == TargetIDX )
			{
				ActiveIDX = ActiveIDX == i ? -1 : i;

				switch ( i )
				{
				case 0:
					AttachPanel::OnSelect();
					break;
				default:
					break;
				}

				return;
			}
		}
	}
}
