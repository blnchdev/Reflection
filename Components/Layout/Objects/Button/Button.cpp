#include "Button.h"

#include "Components/FontManager/FontManager.h"
#include "Components/Graphics/Graphics.h"
#include "Components/Theme Manager/Theme Manager.h"

namespace Objects
{
	namespace
	{
		uint32_t GlobalIDX = 0;
	}

	uint32_t Button::GetActiveColor() const
	{
		switch ( Status )
		{
		case IDLE:
			return Override.Idle != 0 ? Override.Idle : Themes::Manager::GetColor( "Button_Idle"_H );
		case HOVER:
			return Override.Hover != 0 ? Override.Hover : Themes::Manager::GetColor( "Button_Hover"_H );
		case CLICK:
			return Override.Click != 0 ? Override.Click : Themes::Manager::GetColor( "Button_Click"_H );
		default:
			break;
		}

		return 0;
	}

	void Button::Resize()
	{
		if ( Type == ButtonType::Rectangle )
		{
			const ImVec2 TextSize = Renderer::FontManager::FontSize( Label, FontSize, false );
			this->Size.x          = TextSize.x * 1.4f;
			this->Size.y          = TextSize.y * 1.2f;
		}
		else
		{
			const ImVec2 TextSize = Renderer::FontManager::FontSize( Label, FontSize, false );
			this->Radius          = sqrt( TextSize.x * TextSize.x + TextSize.y * TextSize.y );
		}
	}

	void Button::Render() const
	{
		const uint32_t CurrentColor = GetActiveColor();
		const uint32_t BorderColor  = Override.Border != 0 ? Override.Border : Themes::Manager::GetColor( "Button_Outline"_H );

		uint32_t TextColor = this->Font.Idle.value_or( Themes::Manager::GetColor( "Global_Text"_H ) );

		if ( this->Status != IDLE )
		{
			Renderer::Window::SetCursorType( Renderer::Hand );
			if ( this->Font.Active.has_value() ) TextColor = this->Font.Active.value();
		}

		if ( Type == ButtonType::Rectangle )
		{
			const float RoundFactor = this->IsRounded ? this->Rounding : 0.f;
			Graphics::AddFilledRectangle( Position, Size.x, Size.y, CurrentColor, RoundFactor, Flags );
			Graphics::AddRectangle( Position, Size.x, Size.y, BorderColor, 1.f, RoundFactor, Flags );
			const Vector2f Center = Position + Size / 2.f;
			Renderer::FontManager::AddText( Label, FontSize, TextColor, Center, false, DT_CENTER | DT_VCENTER );
		}
		else
		{
			Graphics::AddFilledCircle( Position, Radius, CurrentColor );
			Graphics::AddCircle( Position, Radius, BorderColor );

			Renderer::FontManager::AddText( Label, FontSize, TextColor, Position, false, DT_CENTER | DT_VCENTER );
		}
	}

	bool Button::WndProc( const Vector2f CursorPosition, const uint32_t Message, WPARAM wParam, LPARAM lParam )
	{
		bool IsHovering;

		if ( Type == ButtonType::Circle )
		{
			IsHovering = CursorPosition.Distance( Position ) < Radius;
		}
		else
		{
			IsHovering = CursorPosition.x >= Position.x && CursorPosition.y >= Position.y && CursorPosition.x <= Position.x + Size.x && CursorPosition.y <= Position.y + Size.y;
		}

		if ( !IsHovering )
		{
			Status = IDLE;
			return false;
		}

		if ( Status == IDLE ) Status = HOVER;
		if ( Message == WM_LBUTTONDOWN ) Status = CLICK;
		if ( Message == WM_LBUTTONUP )
		{
			Status = HOVER;
			if ( Callback ) Callback( this );
		}

		return true;
	}

	Button::Button()
	{
		GlobalIDX += 1;
		this->IDX = GlobalIDX;
	}

	Button::Button( std::string Label, const std::function<void( Button* )>& Callback, const ButtonType Type, const OverrideData& ColorData, const FontData& Font ) : Font( Font ), Type( Type ), Override( ColorData ), Label( std::move( Label ) ), Callback( Callback )
	{
		GlobalIDX += 1;
		this->IDX = GlobalIDX;
	}
}
