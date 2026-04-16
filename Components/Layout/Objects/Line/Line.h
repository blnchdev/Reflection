#pragma once
#include "Components/Memory Manager/MemoryStructs.h"

namespace Renderer::Layout
{
	struct RenderParameters;
	struct LineSlot;
}

namespace Objects
{
	enum LineWndProcResult : uint8_t
	{
		NOT_CONSUMED,
		BUTTON_CLICKED,
		LINE_HEIGHT_MATCHES
	};

	class Line
	{
	public:
		explicit Line( Renderer::Layout::LineSlot* Owner ) : Owner( Owner )
		{
		}

		friend Renderer::Layout::LineSlot;

		Line( const Line& )            = delete;
		Line& operator=( const Line& ) = delete;
		Line( Line&& )                 = default;
		Line& operator=( Line&& )      = default;

		void              Render( Vector2f& Cursor, Renderer::Layout::RenderParameters* RenderData, Memory::Structs::Info* Structure, Memory::Structs::Field* Field, size_t& i, bool IsSelected );
		LineWndProcResult WndProc( const Vector2f& CursorPos, uint32_t Message, WPARAM wParam, LPARAM lParam ) const;
		void              EnsureExpandButton();
		void              ClearExpandButton() { ExpandButton = nullptr; }

		Vector2f                    Origin = Vector2f::Zero;
		Renderer::Layout::LineSlot* Owner  = nullptr;

	private:
		// Only present when the entry has a child view (e.g. pointer, function...)
		std::shared_ptr<Button> ExpandButton = nullptr;
	};
}
