#include "SearchBar.h"

#include <chrono>
#include <print>

#include "Components/FontManager/FontManager.h"
#include "Components/Graphics/Graphics.h"
#include "Components/Theme Manager/Theme Manager.h"

namespace Objects
{
	SearchBar::SearchBar() = default;

	SearchBar::SearchBar( std::string Placeholder, const std::function<void( SearchBar* )>& OnChange, const std::function<void( SearchBar* )>& OnConfirm, const OverrideData& ColorData, const FontData& Font ) : Font( Font ),
	                                                                                                                                                                                                              Override( ColorData ),
	                                                                                                                                                                                                              Placeholder( std::move( Placeholder ) ),
	                                                                                                                                                                                                              OnChange( OnChange ),
	                                                                                                                                                                                                              OnConfirm( OnConfirm )
	{
	}

	uint32_t SearchBar::GetActiveColor() const
	{
		switch ( Status )
		{
		case SB_HOVER:
			return Override.Hover;
		case SB_ACTIVE:
			return Override.Active;
		default:
			return Override.Idle;
		}
	}

	void SearchBar::Resize()
	{
		if ( !AutoResize ) return;

		const std::string& Ref      = Query.empty() ? Placeholder : Query;
		const ImVec2       TextSize = Renderer::FontManager::FontSize( Ref, FontSize, false );
		Size.x                      = TextSize.x + 16.f;
		Size.y                      = TextSize.y + 8.f;
	}

	void SearchBar::Render()
	{
		const uint32_t BackgroundColor = Override.Idle != 0 ? Override.Idle : Themes::Manager::GetColor( "Global_WindowBackground"_H );
		const uint32_t BorderColor     = Override.Border != 0 ? Override.Border : Themes::Manager::GetColor( "Global_Border"_H );

		Graphics::AddFilledRectangle( Position, Size.x, Size.y, BackgroundColor, Themes::ROUNDING );
		Graphics::AddRectangle( Position, Size.x, Size.y, BorderColor, 1.f, Themes::ROUNDING );

		const Vector2f TextPosition = { Position.x + 8.f, Position.y + Size.y / 2.f };

		if ( Query.empty() )
		{
			const uint32_t PlaceholderColor = Themes::Manager::GetColor( "MemoryGrid_PlaceholderLabel"_H ); // TODO separate color
			Renderer::FontManager::AddText( Placeholder, FontSize, PlaceholderColor, TextPosition, false, DT_VCENTER );
		}
		else
		{
			const uint32_t TextColor = Status == SB_ACTIVE && Font.Active.has_value() ? Font.Active.value() : Themes::Manager::GetColor( "Global_Text"_H );

			Renderer::FontManager::AddModifiableText( this->Query,
			                                          FontSize,
			                                          TextColor,
			                                          TextPosition,
			                                          false,
			                                          DT_VCENTER,
			                                          Size.x - 16.f,
			                                          [this] ( std::string NewValue, [[maybe_unused]] bool WasCompleted )
			                                          {
				                                          this->Query = std::move( NewValue );
				                                          if ( OnChange ) OnChange( this );
			                                          }
			                                        );
		}
	}

	bool SearchBar::WndProc( const Vector2f CursorPosition, const uint32_t Message, const WPARAM wParam, LPARAM lParam )
	{
		const bool Hovered = CursorPosition.x >= Position.x && CursorPosition.x <= Position.x + Size.x &&
		                     CursorPosition.y >= Position.y && CursorPosition.y <= Position.y + Size.y;

		switch ( Message )
		{
		case WM_MOUSEMOVE:
			{
				if ( Status != SB_ACTIVE ) Status = Hovered ? SB_HOVER : SB_IDLE;
				return Hovered;
			}
		case WM_CHAR:
			{
				if ( this->Status == SB_ACTIVE )
				{
					const wchar_t WideChar = static_cast<wchar_t>( wParam );

					if ( WideChar == VK_BACK )
					{
						if ( !this->Query.empty() ) this->Query.pop_back();
						if ( this->OnChange ) OnChange( this );
						return true;
					}

					if ( WideChar >= 32 && WideChar != 127 )
					{
						char Buffer[ 5 ] = {};
						WideCharToMultiByte( CP_UTF8, 0, &WideChar, 1, Buffer, sizeof( Buffer ), nullptr, nullptr );
						this->Query.append( Buffer );
						if ( this->OnChange ) OnChange( this );
						return true;
					}
				}
			}
			break;
		case WM_LBUTTONDOWN:
			{
				if ( Hovered )
				{
					Status = SB_ACTIVE;
					return true;
				}

				if ( Status == SB_ACTIVE ) Status = SB_IDLE;

				return false;
			}
		case WM_KEYDOWN:
			{
				if ( Status != SB_ACTIVE ) return false;

				if ( wParam == VK_RETURN )
				{
					if ( OnConfirm ) OnConfirm( this );
					Status = SB_IDLE;
					return true;
				}

				if ( wParam == VK_ESCAPE )
				{
					Status = SB_IDLE;
					return true;
				}

				return true;
			}

		default:
			break;
		}

		return false;
	}
}
