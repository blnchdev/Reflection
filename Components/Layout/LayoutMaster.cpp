#include "LayoutMaster.h"

#include <array>

#include "Dissector/Dissector.h"

#define IF_VISIBLE(Layer) Opacity = ManagedOpacities[static_cast<uint8_t>(Layer)]; if (Opacity > EPSILON)

namespace Renderer
{
	namespace
	{
		std::array<float, static_cast<size_t>( LayoutStage::SIZE_ )> ManagedOpacities = {};
	}

	void LayoutMaster::Present()
	{
		float Opacity;
		ManagedOpacities[ static_cast<uint8_t>( LayoutStage::Dissector ) ] = 1.f;
		IF_VISIBLE( LayoutStage::Dissector ) Layout::Dissector::Present( Opacity );
	}

	bool LayoutMaster::WndProc( const Vector2f Position, const uint32_t Message, const WPARAM wParam, const LPARAM lParam )
	{
		float Opacity;

		IF_VISIBLE( LayoutStage::Dissector )
		{
			if ( Layout::Dissector::WndProc( Position, Message, wParam, lParam ) ) return true;
		}

		return false;
	}

	void LayoutMaster::SetOpacity( LayoutStage Layer, float Opacity )
	{
	}
}
