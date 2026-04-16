#include "FontManager.h"
#include "misc/freetype/imgui_freetype.h"
#include <unordered_map>

#include "GeistMono.h"
#include "JetbrainsMono.h"
#include "NotoEmoji.h"
#include "Components/Theme Manager/Theme Manager.h"
#include "Definitions/Globals.h"
#include <string_view>

namespace Renderer
{
	namespace
	{
		std::unordered_map<int, ImFont*> UiFontList;
		std::unordered_map<int, ImFont*> HexFontList;
	}

	void FontManager::AddText( const std::string_view Text, const float Size, const Vector2f Position, const bool CodeFont, const uint32_t Flags, const float WrappingLength )
	{
		const auto TextSize = GetFont( Size, CodeFont )->CalcTextSizeA( Size, FLT_MAX, 0.0f, Text.data(), Text.data() + Text.size() );
		ImVec2     Pos      = { Position.x, Position.y };

		if ( Flags & DT_CENTER )
		{
			Pos.x -= TextSize.x / 2.f;
		}

		if ( Flags & DT_VCENTER )
		{
			Pos.y -= TextSize.y / 2.f;
		}

		ImGui::GetForegroundDrawList()->AddText( GetFont( Size, CodeFont ), Size, Pos, IM_COL32( 255, 255, 255, 255 ), Text.data(), Text.data() + Text.size(), WrappingLength );
	}

	void FontManager::AddText( const std::string_view Text, const float Size, const uint32_t Color, const Vector2f Position, const bool CodeFont, const uint32_t Flags, const float WrappingLength )
	{
		const auto TextSize = GetFont( Size, false )->CalcTextSizeA( Size, FLT_MAX, 0.0f, Text.data(), Text.data() + Text.size() );
		ImVec2     Pos      = { Position.x, Position.y };

		if ( Flags & DT_CENTER )
		{
			Pos.x -= TextSize.x / 2.f;
		}

		if ( Flags & DT_VCENTER )
		{
			Pos.y -= TextSize.y / 2.f;
		}

		ImGui::GetForegroundDrawList()->AddText( GetFont( Size, CodeFont ), Size, Pos, Color, Text.data(), Text.data() + Text.size(), WrappingLength );
	}

	void FontManager::AddSelectableText( std::string_view Text, const float Size, const uint32_t Color, const Vector2f Position, const bool CodeFont, const uint32_t Flags, const float WrappingLength )
	{
		ImFont*      Font     = GetFont( Size, CodeFont );
		const ImVec2 TextSize = Font->CalcTextSizeA( Size, FLT_MAX, WrappingLength > 0.f ? WrappingLength : FLT_MAX, Text.data(), Text.data() + Text.size() );
		ImVec2       Pos      = { Position.x, Position.y };

		if ( Flags & DT_CENTER ) Pos.x -= TextSize.x / 2.f;

		if ( Flags & DT_VCENTER ) Pos.y -= TextSize.y / 2.f;

		ImGui::PushFont( Font );

		ImGui::PushStyleColor( ImGuiCol_FrameBg, ImVec4( 0, 0, 0, 0 ) );
		ImGui::PushStyleColor( ImGuiCol_FrameBgHovered, ImVec4( 0, 0, 0, 0 ) );
		ImGui::PushStyleColor( ImGuiCol_FrameBgActive, ImVec4( 0, 0, 0, 0 ) );
		ImGui::PushStyleColor( ImGuiCol_Border, ImVec4( 0, 0, 0, 0 ) );
		ImGui::PushStyleColor( ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4( Color ) );
		ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 0, 0 ) );
		ImGui::PushStyleVar( ImGuiStyleVar_FrameBorderSize, 0.f );

		ImGui::SetCursorScreenPos( Pos );

		ImGui::PushID( static_cast<int>( Position.x * 6719.f + Position.y * 3541.f ) );
		ImGui::SetNextItemWidth( TextSize.x );
		ImGui::InputText(
		                 _( "##selectable_text" ),
		                 const_cast<char*>( Text.data() ), // const_cast necessary, should not matter considering ImGuiInputTextFlags_ReadOnly
		                 Text.size(),
		                 ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_NoHorizontalScroll
		                );

		ImGui::PopStyleVar( 2 );
		ImGui::PopStyleColor( 5 );
		ImGui::PopFont();
		ImGui::PopID();
	}

	struct ModifiableTextContext
	{
		std::string*                             Text     = nullptr;
		std::function<void( std::string, bool )> Callback = nullptr;
	};

	void FontManager::AddModifiableText( std::string& Text, const float Size, const uint32_t Color, const Vector2f Position, const bool CodeFont, const uint32_t Flags, const float WrappingLength, const ModifiableTextCallback& OnChange )
	{
		ImFont*      Font     = GetFont( Size, CodeFont );
		const ImVec2 TextSize = Font->CalcTextSizeA( Size, FLT_MAX, WrappingLength > 0.f ? WrappingLength : FLT_MAX, Text.c_str() );
		ImVec2       Pos      = { Position.x, Position.y };

		if ( Flags & DT_CENTER ) Pos.x -= TextSize.x / 2.f;
		if ( Flags & DT_VCENTER ) Pos.y -= TextSize.y / 2.f;

		ImGui::PushFont( Font );

		ImGui::PushStyleColor( ImGuiCol_FrameBg, ImVec4( 0, 0, 0, 0 ) );
		ImGui::PushStyleColor( ImGuiCol_FrameBgHovered, ImVec4( 0, 0, 0, 0 ) );
		ImGui::PushStyleColor( ImGuiCol_FrameBgActive, ImVec4( 0, 0, 0, 0 ) );
		ImGui::PushStyleColor( ImGuiCol_Border, ImVec4( 0, 0, 0, 0 ) );
		ImGui::PushStyleColor( ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4( Color ) );
		ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 0, 0 ) );
		ImGui::PushStyleVar( ImGuiStyleVar_FrameBorderSize, 0.f );

		ImGui::SetCursorScreenPos( Pos );

		static ModifiableTextContext Context;
		Context = { .Text = &Text, .Callback = OnChange };

		auto ResizeCallback = [] ( ImGuiInputTextCallbackData* Data ) -> int
		{
			const auto* C        = static_cast<ModifiableTextContext*>( Data->UserData );
			const auto& Callback = C->Callback;

			if ( Data->EventFlag == ImGuiInputTextFlags_CallbackResize )
			{
				C->Text->resize( Data->BufTextLen );
				Data->Buf = C->Text->data();
				if ( Callback ) Callback( *C->Text, false );
			}

			return 0;
		};

		ImGui::PushID( static_cast<int>( Position.x * 6719.f + Position.y * 3541.f ) );
		ImGui::SetNextItemWidth( TextSize.x );
		std::string Cpy = Text;
		ImGui::InputText(
		                 _( "##modifiable_text" ),
		                 Text.data(),
		                 Text.capacity() + 1,
		                 ImGuiInputTextFlags_CallbackResize | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_NoHorizontalScroll,
		                 ResizeCallback,
		                 &Context
		                );

		if ( ImGui::IsItemDeactivatedAfterEdit() && OnChange )
		{
			OnChange( Text, true );
		}

		ImGui::PopStyleVar( 2 );
		ImGui::PopStyleColor( 5 );
		ImGui::PopFont();
		ImGui::PopID();
	}

	void FontManager::AddGradientText( const std::string_view Text, const float Size, const Vector2f Position, const uint32_t ColorStart, const uint32_t ColorEnd, const std::optional<uint32_t> OutlineColor )
	{
		const float Time     = static_cast<float>( ImGui::GetTime() );
		ImFont*     Font     = GetFont( Size, true );
		auto        TextSize = Font->CalcTextSizeA( Size, FLT_MAX, 0.0f, Text.data(), Text.data() + Text.size() );
		ImDrawList* DrawList = ImGui::GetForegroundDrawList();

		const Vector2f Center      = Position - *reinterpret_cast<Vector2f*>( &TextSize ) * .5f;
		const bool     DrawOutline = OutlineColor.has_value();

		float        OffsetX = Center.x;
		const ImVec4 Start   = ImGui::ColorConvertU32ToFloat4( ColorStart );
		const ImVec4 End     = ImGui::ColorConvertU32ToFloat4( ColorEnd );

		for ( int i = 0; Text[ i ] != '\0'; i++ )
		{
			const float Wave = sinf( Time * -1.6f + static_cast<float>( i ) * 0.6f );
			const float t    = Wave * 0.5f + 0.5f;
			const float t2   = t * t;

			auto Lerped = ImVec4
			(
			 Start.x + ( End.x - Start.x ) * t2,
			 Start.y + ( End.y - Start.y ) * t2,
			 Start.z + ( End.z - Start.z ) * t2,
			 Start.w + ( End.w - Start.w ) * t2
			);

			const ImU32 CharColor = ImGui::ColorConvertFloat4ToU32( Lerped );

			const char   c           = Text[ i ];
			const char   String[ 2 ] = { c, '\0' };
			const ImVec2 CharSize    = Font->CalcTextSizeA( Size, FLT_MAX, 0.0f, String );

			if ( DrawOutline )
			{
				constexpr float OutlineOffset = 1.0f;

				DrawList->AddText( Font, Size, ImVec2( OffsetX - OutlineOffset, Center.y ), OutlineColor.value(), String );
				DrawList->AddText( Font, Size, ImVec2( OffsetX + OutlineOffset, Center.y ), OutlineColor.value(), String );
				DrawList->AddText( Font, Size, ImVec2( OffsetX, Center.y - OutlineOffset ), OutlineColor.value(), String );
				DrawList->AddText( Font, Size, ImVec2( OffsetX, Center.y + OutlineOffset ), OutlineColor.value(), String );
			}

			DrawList->AddText( Font, Size, ImVec2( OffsetX, Center.y ), CharColor, String );
			OffsetX += CharSize.x;
		}
	}

	void FontManager::Initialize()
	{
		const ImGuiIO& io           = ImGui::GetIO();
		ImFontConfig   Config       = {};
		Config.FontDataOwnedByAtlas = false;
		Config.FontLoaderFlags      = /*ImGuiFreeTypeBuilderFlags_NoHinting |*/ ImGuiFreeTypeLoaderFlags_MonoHinting; /*| ImGuiFreeTypeBuilderFlags_LoadColor;*/

		ImFontConfig EmojiConfig         = {};
		EmojiConfig.MergeMode            = true;
		EmojiConfig.FontDataOwnedByAtlas = false;
		EmojiConfig.OversampleH          = EmojiConfig.OversampleV = 1;

		static constexpr ImWchar EmojiRanges[ ] =
		{
			0x2000, 0x2BFF,
			0xE000, 0xF8FF,
			0xF0000, 0xFFFFF,
			0,
		};

		io.Fonts->FontLoader = ImGuiFreeType::GetFontLoader();
		io.Fonts->AddFontFromMemoryTTF( Font::GeistMono, sizeof( Font::GeistMono ), 14.f );
		io.Fonts->AddFontFromMemoryTTF( Font::SymbolsNerdFont, sizeof( Font::SymbolsNerdFont ), 14, &EmojiConfig, EmojiRanges );

		for ( int i = 8; i <= 32; i += 4 )
		{
			ImFont* Font = io.Fonts->AddFontFromMemoryTTF( Font::GeistMono, sizeof( Font::GeistMono ), i, &Config );
			Font         = io.Fonts->AddFontFromMemoryTTF( Font::SymbolsNerdFont, sizeof( Font::SymbolsNerdFont ), i, &EmojiConfig, EmojiRanges );
			UiFontList.emplace( i, Font );

			Font = io.Fonts->AddFontFromMemoryTTF( Font::JetbrainsMono, sizeof( Font::JetbrainsMono ), i, &Config );
			HexFontList.emplace( i, Font );
		}

		ImFont* Font = io.Fonts->AddFontFromMemoryTTF( Font::GeistMono, sizeof( Font::GeistMono ), 64.f, &Config );
		io.Fonts->AddFontFromMemoryTTF( Font::SymbolsNerdFont, sizeof( Font::SymbolsNerdFont ), 64.f, &EmojiConfig, EmojiRanges );
		UiFontList.emplace( 64.f, Font );
		Font = io.Fonts->AddFontFromMemoryTTF( Font::JetbrainsMono, sizeof( Font::JetbrainsMono ), 64.f, &Config );
		HexFontList.emplace( 64.f, Font );

		io.Fonts->Build();
	}

	ImFont* FontManager::GetFont( float Size, const bool CodeFont )
	{
		auto&      List     = CodeFont ? HexFontList : UiFontList;
		const auto Iterator = std::ranges::min_element( List, [Size] ( const std::pair<const int, ImFont*> a, const std::pair<const int, ImFont*> b )
		{
			return std::abs( static_cast<int>( Size ) - a.first ) < std::abs( static_cast<int>( Size ) - b.first );
		} );

		if ( Iterator == List.end() )
		{
			return nullptr;
		}

		return Iterator->second;
	}

	ImVec2 FontManager::FontSize( const std::string_view Text, const float Size, const bool CodeFont, const float WrappingLength )
	{
		ImFont* Font = GetFont( Size, CodeFont );
		return Font->CalcTextSizeA( Size, FLT_MAX, WrappingLength, Text.data(), Text.data() + Text.size() );
	}
}
