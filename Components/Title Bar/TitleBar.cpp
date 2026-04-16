#include "TitleBar.h"

#include <cstdint>
#include <windowsx.h>

#include "Components/FontManager/FontManager.h"
#include "Components/Graphics/Graphics.h"
#include "Components/Renderer/Renderer.h"
#include "Components/Theme Manager/Theme Manager.h"
#include "Definitions/Globals.h"
#include "Libraries/Aether/Aether.h"

namespace Renderer
{
	namespace
	{
		struct Dot
		{
			float    x     = 0.f;
			uint32_t Color = 0;
			bool     Hit   = false;
		};

		uint32_t MutedColor = 0;
		Vector2f Size       = Vector2f::Zero;

		Dot Close    = {};
		Dot Maximize = {};
		Dot Minimize = {};

		Dot* List[ 3 ] = { &Close, &Maximize, &Minimize };

		float DistanceRatio( const float Distance, const float MinDistance )
		{
			const float MaxDistance = MinDistance * 2.5f;

			if ( Distance <= MinDistance ) return 1.0f;

			if ( Distance >= MaxDistance ) return 0.0f;

			const float Range = MaxDistance - MinDistance;
			const float T     = ( Distance - MinDistance ) / Range;

			return 1.0f - T;
		}
	}

	void Titlebar::Render()
	{
		const auto Data = Window::GetWindowData();

		Size = { Data->Width, 30.f };

		const bool Round = !Window::Maximized();

		if ( Round )
		{
			Graphics::AddFilledRectangle( Vector2f::Zero, Size.x, Size.y, Themes::Manager::GetColor( "TitleBar_Background"_H ), Themes::ROUNDING, ImDrawFlags_RoundCornersTop );
			Graphics::AddLine( Vector2f( 1.f + Themes::ROUNDING, 0.f ), Vector2f( Data->Width - 2.f - Themes::ROUNDING, 0.f ), Themes::Manager::GetColor( "Global_Border"_H ), 1.f );
		}
		else
		{
			Graphics::AddFilledRectangle( Vector2f::Zero, Size.x, Size.y, Themes::Manager::GetColor( "TitleBar_Background"_H ), 0.f, 0 );
			Graphics::AddRectangle( Vector2f::Zero, Size.x, Size.y, Themes::Manager::GetColor( "Global_Border"_H ), 1.f );
		}


		const float CircleOffsetBase = Size.y / 2.f;
		const float CircleRadius     = Size.y / 5.f;
		const float CirclePadding    = CircleRadius * 0.75f;

		const float DistanceBetweenCenters = CircleRadius * 2.f + CirclePadding;

		Close.x    = Size.x - CircleOffsetBase - CircleRadius;
		Maximize.x = Close.x - DistanceBetweenCenters;
		Minimize.x = Maximize.x - DistanceBetweenCenters;

		Close.Color    = Themes::Manager::GetColor( "TitleBar_Button_Close"_H );
		Maximize.Color = Themes::Manager::GetColor( "TitleBar_Button_Maximize"_H );
		Minimize.Color = Themes::Manager::GetColor( "TitleBar_Button_Minimize"_H );

		const float MuteDisableRadius = fabsf( Close.x - Maximize.x + CircleRadius ) * 5.f;

		const auto  CursorPosition = Window::GetCursorPosition();
		const float Distance       = Vector2f{ Maximize.x, CircleOffsetBase }.Distance( CursorPosition );
		const float Ratio          = DistanceRatio( Distance, MuteDisableRadius );

		Vector2f Vector = {};

		MutedColor = Themes::Manager::GetColor( "TitleBar_Button_Background"_H );

		for ( const auto& D : List )
		{
			Vector = Vector2f{ D->x, CircleOffsetBase };
			D->Hit = CursorPosition.Distance( Vector ) <= CircleRadius;

			Graphics::AddFilledCircle( Vector, CircleRadius, Graphics::LerpRGBA( MutedColor, D->Color, Ratio ) );

			if ( D->Hit )
			{
				Graphics::AddFilledCircle( Vector, CircleRadius, Themes::Manager::GetColor( "TitleBar_Button_Hover"_H ) );
			}
		}

		FontManager::AddGradientText( _( "Reflection" ), 18.f, Vector2f{ Size.x / 2.f, 15.f }, Themes::Manager::GetColor( "TitleBar_WatermarkA"_H ), Themes::Manager::GetColor( "TitleBar_WatermarkB"_H ) );
	}

	LRESULT Titlebar::HandleNcHitTest( const LPARAM lParam )
	{
		const POINT Cursor = { GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) };
		RECT        WindowRect;
		GetWindowRect( Window::GetHandle(), &WindowRect );

		constexpr int BorderWidth = 8;

		const bool OnLeft   = Cursor.x < WindowRect.left + BorderWidth;
		const bool OnRight  = Cursor.x >= WindowRect.right - BorderWidth;
		const bool OnTop    = Cursor.y < WindowRect.top + BorderWidth;
		const bool OnBottom = Cursor.y >= WindowRect.bottom - BorderWidth;

		if ( OnTop && OnLeft ) return HTTOPLEFT;
		if ( OnTop && OnRight ) return HTTOPRIGHT;
		if ( OnBottom && OnLeft ) return HTBOTTOMLEFT;
		if ( OnBottom && OnRight ) return HTBOTTOMRIGHT;
		if ( OnTop ) return HTTOP;
		if ( OnBottom ) return HTBOTTOM;
		if ( OnLeft ) return HTLEFT;
		if ( OnRight ) return HTRIGHT;

		const bool InTitleBar = Cursor.y < WindowRect.top + 30.f && Cursor.x > WindowRect.left + 120;
		if ( InTitleBar ) return HTCAPTION;

		return HTCLIENT;
	}

	void Titlebar::WndProc( UINT Message )
	{
		if ( Message != WM_NCLBUTTONDOWN ) return;

		const auto hWindow = Window::GetHandle();

		if ( Close.Hit ) Window::RequestStop(); // Stop execution
		if ( Maximize.Hit ) Window::Maximized() ? ShowWindow( hWindow, SW_RESTORE ) : ShowWindow( hWindow, SW_MAXIMIZE );
		if ( Minimize.Hit ) ShowWindow( hWindow, SW_MINIMIZE );
	}

	Vector2f Titlebar::GetSize()
	{
		return Size;
	}
}
