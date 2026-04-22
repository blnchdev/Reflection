#include "Line.h"

#include <print>

#include "Components/Graphics/Graphics.h"
#include "Components/Layout/Dissector/Grid/Grid.h"

namespace Objects
{
	namespace
	{
		Button MakeTransparentButton( const Objects::ButtonType Type )
		{
			Button Button;
			Button.Type            = Type;
			Button.Override.Border = Graphics::RGBA( 255, 255, 255, 0 );
			Button.Override.Hover  = Graphics::RGBA( 255, 255, 255, 0 );
			Button.Override.Click  = Graphics::RGBA( 255, 255, 255, 0 );
			Button.Override.Idle   = Graphics::RGBA( 255, 255, 255, 0 );
			return Button;
		}
	}

	void Line::Render( Vector2f& Cursor, Renderer::Layout::RenderParameters* RenderData, Memory::Info* BackingData, Memory::Field* Field, size_t& i, bool IsSelected )
	{
		Origin = Cursor;

		// This should never happen but in case...
		if ( !this->Owner )
		{
			std::println( "Line::Line() had no Owner!" );
			__debugbreak();
			return;
		}

		Renderer::Layout::LineData& Data = this->Owner->Data;

		Renderer::Layout::Internals::RenderLine( Cursor, RenderData, Data, BackingData, Field, i, IsSelected );

		if ( Data.Embedded.Child )
		{
			const auto Position    = Vector2f( Origin.x - TAB_WIDTH / 2.f, Origin.y + LINE_HEIGHT / 2.f );
			ExpandButton->Label    = Data.Embedded.Expanded ? "\xF3\xB0\x81\x86" : "\xF3\xB0\x81\x95";
			ExpandButton->FontSize = 8.f;
			ExpandButton->Position = Position;
			ExpandButton->Size.x   = LINE_HEIGHT * 0.75f;
			ExpandButton->Size.y   = LINE_HEIGHT * 0.75f; // TODO: Move Size, FontSize in Line::Line() rather than re-assign on render
			ExpandButton->Render();

			if ( Data.Embedded.Expanded )
			{
				Cursor.x += TAB_WIDTH;

				if ( Field->Type != T_pFunction )
				{
					Renderer::Layout::Grid::RenderView( Data.Embedded.Child, Cursor, i );
				}
				else
				{
					Renderer::Layout::Internals::RenderFunction( Cursor, RenderData, Data, i );
				}

				Cursor.x -= TAB_WIDTH;
			}
		}
	}

	LineWndProcResult Line::WndProc( const Vector2f& CursorPos, const uint32_t Message, const WPARAM wParam, const LPARAM lParam ) const
	{
		if ( ExpandButton && ExpandButton->WndProc( CursorPos, Message, wParam, lParam ) )
		{
			if ( Message == WM_LBUTTONDOWN ) this->Owner->Data.Embedded.Expanded = !this->Owner->Data.Embedded.Expanded;
			return BUTTON_CLICKED;
		}

		// This is really a bandaid fix but since we don't store rendered Width, we leave it to the caller to check X bounds, hence the "LINE_HEIGHT_MATCHES"
		// 16-04-26: This might not be true anymore after the Nth rewrite, TODO: check!
		if ( Message == WM_LBUTTONDOWN && CursorPos.y >= this->Origin.y && CursorPos.y <= this->Origin.y + LINE_HEIGHT ) return LINE_HEIGHT_MATCHES;

		return NOT_CONSUMED;
	}

	void Line::EnsureExpandButton()
	{
		if ( ExpandButton ) return;
		ExpandButton = std::make_shared<Button>( MakeTransparentButton( ButtonType::Circle ) );
	}
}
