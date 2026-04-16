#include "Notifications.h"

#include <algorithm>
#include <chrono>

#include "Components/FontManager/FontManager.h"

namespace Renderer
{
	namespace
	{
		struct Notification
		{
			std::chrono::high_resolution_clock::time_point Start;

			Vector2f    Size;
			std::string Label, Description;
			uint32_t    Color;
			bool        Persistent = false;

			static constexpr float TotalLifeSeconds = 5.f;
			static constexpr float FadeDuration     = 0.5f;

			float GetAlpha() const
			{
				using namespace std::chrono;
				const float Elapsed = duration<float>( high_resolution_clock::now() - Start ).count();

				if ( Elapsed < FadeDuration ) return Elapsed / FadeDuration;
				if ( Elapsed > TotalLifeSeconds - FadeDuration ) return ( TotalLifeSeconds - Elapsed ) / FadeDuration;
				return 1.f;
			}

			bool IsStillValid() const
			{
				return std::chrono::high_resolution_clock::now() - Start < std::chrono::seconds( 5 ) && !Persistent;
			}

			void RemovePersistency()
			{
				if ( !Persistent ) return;

				Start      = std::chrono::high_resolution_clock::now() - std::chrono::milliseconds( 4500 ); // Hack to still have the fade out
				Persistent = false;
			}

			void Render( Vector2f& Origin, const Vector2f& CurrentSize ) const
			{
				const ImVec2 LabelSize       = FontManager::FontSize( Label, 18.f, false );
				const ImVec2 DescriptionSize = FontManager::FontSize( Description, 14.f, false, CurrentSize.x - 20.f );

				const float NecessaryHeight = 10.f + LabelSize.y + 20.f + DescriptionSize.y;

				Origin.y -= NecessaryHeight;

				const float   Alpha     = GetAlpha();
				const uint8_t BaseAlpha = Color >> 24 & 0xFF;
				const uint8_t NewAlpha  = static_cast<uint8_t>( static_cast<float>( BaseAlpha ) * Alpha );

				const uint32_t NewColor  = NewAlpha << 24 | Color & 0x00FFFFFF;
				const uint32_t TextColor = Graphics::RGBA( 255, 255, 255, min( 255, NewAlpha + 30 ) );

				Graphics::AddFilledRectangle( Origin, CurrentSize.x, NecessaryHeight, NewColor, 0.f, 0 );
				Graphics::AddRectangle( Origin, CurrentSize.x, NecessaryHeight, NewColor, 1.f );

				FontManager::AddText( Label, 18.f, TextColor, { Origin.x + 10.f, Origin.y + 10.f } );
				FontManager::AddText( Description, 14.f, TextColor, { Origin.x + 10.f, Origin.y + 20.f + LabelSize.y }, false, 0, CurrentSize.x - 20.f );
			}

			Notification( std::string Label, std::string Description, const uint32_t Color, const bool Persistent = false ) : Label( std::move( Label ) ), Description( std::move( Description ) ), Color( Color ), Persistent( Persistent )
			{
				Start = std::chrono::high_resolution_clock::now();
			}
		};

		std::vector<Notification> Notifications;
	}

#pragma push_macro("max")
#undef max
	void NotificationManager::Render()
	{
		std::erase_if( Notifications, [] ( const Notification& N ) { return !N.IsStillValid(); } );
		const WindowData* Data = Window::GetWindowData();

		const Vector2f NotificationSize = { Data->Width / 3.f, Data->Height / 10.f };
		Vector2f       Cursor           = { Data->Width - NotificationSize.x - 5.f, Data->Height - 5.f };

		auto RenderNotification = [&] ( const Notification& N )
		{
			N.Render( Cursor, NotificationSize );
			Cursor.y -= N.Size.y + 5.f;
		};

		std::ranges::for_each( Notifications, RenderNotification );
	}

	void NotificationManager::AddNotification( const std::string& Label, const std::string& Description, uint32_t Color, bool Persistent )
	{
		Notifications.emplace_back( Label, Description, Color, Persistent );
	}
}

#pragma pop_macro("max")
