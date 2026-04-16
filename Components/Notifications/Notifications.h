#pragma once
#include <string>

#include "Components/Graphics/Graphics.h"

namespace Renderer
{
	enum NotificationColor : uint32_t
	{
		Success = Graphics::RGBA( 148, 226, 213, 200 ),
		Warning = Graphics::RGBA( 250, 179, 135, 200 ),
		Error   = Graphics::RGBA( 243, 139, 168, 200 )
	};

	class NotificationManager
	{
	public:
		static void AddNotification( const std::string& Label, const std::string& Description, uint32_t Color = Success, bool Persistent = false );
		static void Render();
	};
}
