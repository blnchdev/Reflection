#pragma once
#include <functional>
#include <imgui.h>
#include <optional>
#include <Components/Renderer/Renderer.h>

namespace Renderer
{
	// String = Modified Buffer / Bool = Completed Operation
	using ModifiableTextCallback = std::function<void( std::string, bool )>;

	class FontManager
	{
	public:
		static void    Initialize();
		static ImFont* GetFont( float Size, bool CodeFont );
		static ImVec2  FontSize( std::string_view Text, float Size, bool CodeFont, float WrappingLength = 0.f );
		static void    AddText( std::string_view Text, float Size, Vector2f Position, bool CodeFont = false, uint32_t Flags = 0, float WrappingLength = 0.f );
		static void    AddText( std::string_view Text, float Size, uint32_t Color, Vector2f Position, bool CodeFont = false, uint32_t Flags = 0, float WrappingLength = 0.f );
		static void    AddSelectableText( std::string_view Text, float Size, uint32_t Color, Vector2f Position, bool CodeFont, uint32_t Flags = 0, float WrappingLength = 0.f );
		static void    AddModifiableText( std::string& Text, float Size, uint32_t Color, Vector2f Position, bool CodeFont, uint32_t Flags = 0, float WrappingLength = 0.f, const ModifiableTextCallback& OnChange = nullptr );
		static void    AddGradientText( std::string_view Text, float Size, Vector2f Position, uint32_t ColorStart, uint32_t ColorEnd, std::optional<uint32_t> OutlineColor = std::nullopt );
	};
}
