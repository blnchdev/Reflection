#include "ClassBrowser.h"

#include <algorithm>
#include <cstring>
#include <numbers>
#include <print>
#include <windowsx.h>

#include "Components/Class Manager/ClassManager.h"
#include "Components/Graphics/Graphics.h"
#include "Components/Memory Manager/MemoryStructs.h"

namespace Renderer::Layout
{
	namespace
	{
		const auto FolderIcon  = "\xEE\xAB\xB7";
		const auto CompareIcon = "\xEE\xAB\xBD";
		const auto ClassIcon   = "\xEE\xAD\x9B";

		constexpr float SCROLL_SPEED     = 60.f;
		constexpr float SCROLL_DECAY     = 12.f;
		constexpr float SCROLL_SNAP_DIST = 0.5f;
		constexpr float LINE_HEIGHT      = 32.f;
		constexpr float FONT_SIZE        = 16.f;
		constexpr float ICON_SIZE        = 17.f;

		uint64_t GlobalIDX     = 0;
		int32_t  SelectedIndex = -1; // -1 = nothing selected

		float                                          ViewableHeight = 0.f;
		float                                          ContentHeight  = 0.f;
		float                                          ScrollTarget   = 0.f;
		float                                          ScrollOffset   = 0.f;
		std::chrono::high_resolution_clock::time_point LastFrameTime  = std::chrono::high_resolution_clock::now();

		Vector2f LastOrigin = Vector2f::Zero;
		Vector2f LastSize   = Vector2f::Zero;

		std::shared_ptr<Memory::Info> SelectedTemplate = nullptr;

		enum class LineType : uint8_t
		{
			Directory,
			DirectoryUp,
			File
		};

		struct BrowserLine
		{
			LineType              Type;
			std::filesystem::path Path;
			std::string           Label;
			uint64_t              IDX = 0;

			BrowserLine() = delete;

			BrowserLine( const LineType Type, std::filesystem::path Path, std::string Label ) : Type( Type ), Path( std::move( Path ) ), Label( std::move( Label ) )
			{
				++GlobalIDX;
				this->IDX = GlobalIDX;
			}
		};

		BrowserLine              UpDirectory = { LineType::DirectoryUp, {}, _( "..." ) };
		std::vector<BrowserLine> Lines       = {};
		std::filesystem::path    CurrentDirectory;

		bool IsAtRoot()
		{
			return std::filesystem::weakly_canonical( CurrentDirectory ) == std::filesystem::weakly_canonical( ClassManager::GetRootPath() );
		}

		void PopulateList()
		{
			Lines.clear();

			for ( const auto& File : std::filesystem::directory_iterator{ CurrentDirectory } )
			{
				const bool IsDirectory = File.is_directory();
				const bool IsJSON      = File.is_regular_file() && File.path().extension() == ".json";

				if ( !IsDirectory && !IsJSON ) continue;

				const auto Type  = IsDirectory ? LineType::Directory : LineType::File;
				const auto Label = IsDirectory ? File.path().filename().string() : File.path().stem().string();

				Lines.emplace_back( Type, File.path(), Label );
			}

			// Folders first, then files, both alphabetically (case-insensitive)
			std::ranges::sort( Lines, [] ( const BrowserLine& A, const BrowserLine& B )
			{
				if ( A.Type != B.Type ) return A.Type == LineType::Directory;

				return _stricmp( A.Label.c_str(), B.Label.c_str() ) < 0;
			} );

			SelectedIndex = -1;
			ScrollTarget  = 0.f;
			ScrollOffset  = 0.f;
		}

		void NavigateTo( const std::filesystem::path& Path )
		{
			CurrentDirectory = Path;
			PopulateList();
		}

		void NavigateUp()
		{
			if ( IsAtRoot() ) return;

			NavigateTo( CurrentDirectory.parent_path() );
		}

		void ActivateLine( const BrowserLine& Line )
		{
			switch ( Line.Type )
			{
			case LineType::DirectoryUp:
				NavigateUp();
				break;

			case LineType::Directory:
				NavigateTo( Line.Path );
				break;

			case LineType::File:
				{
					const auto RootPath     = ClassManager::GetRootPath();
					const auto RelativePath = Line.Path.lexically_relative( RootPath ).string();

					if ( auto Instance = ClassManager::GetInstanceFromPath( RelativePath ) )
					{
						SelectedTemplate = std::move( Instance );
					}

					break;
				}
			}
		}

		const BrowserLine* LineAtIndex( const int32_t Index )
		{
			if ( Index == 0 ) return &UpDirectory;

			const size_t RealIndex = static_cast<size_t>( Index - 1 );

			if ( RealIndex >= Lines.size() ) return nullptr;

			return &Lines[ RealIndex ];
		}
	}

	void ClassBrowser::Render( Vector2f Cursor, Vector2f AvailableSize )
	{
		const uint32_t Background = Themes::Manager::GetColor( "Global_WindowBackground"_H );
		const uint32_t Popup      = Themes::Manager::GetColor( "Global_PopupBackground"_H );
		const uint32_t Border     = Themes::Manager::GetColor( "Global_Border"_H );
		const uint32_t Text       = Themes::Manager::GetColor( "Global_Text"_H );
		const uint32_t Selection  = Themes::Manager::GetColor( "Global_Selection"_H );

		const Vector2f Original = Cursor;

		const float Width = AvailableSize.x * 0.9f;

		Cursor.x = Original.x + AvailableSize.x * 0.05f;
		Cursor.y += 10.f;

		const float HeightForButtons = 10.f + 20.f; // AttachButton.Size.y;
		const float Height           = AvailableSize.y - 10.f - ( Cursor.y - Original.y ) - HeightForButtons;

		ViewableHeight = Height;
		ContentHeight  = LINE_HEIGHT * static_cast<float>( Lines.size() + 1 );

		const float MaxScroll = max( 0.f, ContentHeight - ViewableHeight );
		ScrollTarget          = std::clamp( ScrollTarget, 0.f, MaxScroll );
		ScrollOffset          = std::clamp( ScrollOffset, 0.f, MaxScroll );

		const auto  Now       = std::chrono::high_resolution_clock::now();
		const float DeltaTime = std::chrono::duration<float>( Now - LastFrameTime ).count();
		LastFrameTime         = Now;

		if ( fabsf( ScrollTarget - ScrollOffset ) > SCROLL_SNAP_DIST ) ScrollOffset += ( ScrollTarget - ScrollOffset ) * ( 1.f - std::expf( -SCROLL_DECAY * DeltaTime ) );
		else ScrollOffset                                                           = ScrollTarget;

		Vector2f ScrolledCursor = Cursor;
		ScrolledCursor.y        -= ScrollOffset;

		ImVec2 ClipMin( Cursor.x, Cursor.y );
		ImVec2 ClipMax( Cursor.x + Width, Cursor.y + Height );

		LastOrigin = Cursor;
		LastSize   = Vector2f{ Width, Height };

		ImGui::GetBackgroundDrawList()->PushClipRect( ClipMin, ClipMax, true );
		ImGui::GetForegroundDrawList()->PushClipRect( ClipMin, ClipMax, true );
		ImGui::GetWindowDrawList()->PushClipRect( ClipMin, ClipMax, true );

		constexpr float FontSize = 16.f;
		const auto      szIcon   = FontManager::FontSize( ClassIcon, FontSize, false );

		const float oX = ScrolledCursor.x;

		const auto RenderLine = [&] ( const BrowserLine& Line, const int32_t Index )
		{
			const float LineTop    = ScrolledCursor.y;
			const float LineBottom = LineTop + LINE_HEIGHT;

			const bool LineVisible = LineBottom >= ClipMin.y && LineTop <= ClipMin.y + ViewableHeight;

			if ( LineVisible )
			{
				if ( Index == SelectedIndex )
				{
					ImGui::GetWindowDrawList()->AddRectFilled( ImVec2( ClipMin.x, LineTop ), ImVec2( ClipMax.x, LineBottom ), Selection );
				}

				ScrolledCursor.x += LINE_HEIGHT * 0.2f;
				ScrolledCursor.y += LINE_HEIGHT / 2.f;

				const char* Icon = Line.Type == LineType::File ? ClassIcon : FolderIcon;

				FontManager::AddText( Icon, FontSize - 2.f, Text, ScrolledCursor, false, DT_VCENTER );
				ScrolledCursor.x += szIcon.x + 6.f;
				FontManager::AddText( Line.Label, FontSize, Text, ScrolledCursor, false, DT_VCENTER );

				ScrolledCursor.x = oX;
				ScrolledCursor.y += LINE_HEIGHT / 2.f;
			}
			else
			{
				ScrolledCursor.y += LINE_HEIGHT;
			}
		};

		RenderLine( UpDirectory, 0 );

		int32_t Index = 1;
		for ( const auto& Line : Lines )
		{
			RenderLine( Line, Index );
			++Index;
		}

		ImGui::GetBackgroundDrawList()->PopClipRect();
		ImGui::GetForegroundDrawList()->PopClipRect();
		ImGui::GetWindowDrawList()->PopClipRect();

		constexpr float Rounding = Themes::ROUNDING;
		const auto      DrawList = ImGui::GetForegroundDrawList();

		auto DrawCorner = [DrawList, Rounding] ( Vector2f Point, const float X, const float Y, const float A0, const float A1, const uint32_t Color )
		{
			auto C = Vector2f( Point.x + Rounding * X, Point.y + Rounding * Y );

			DrawList->PathLineTo( *reinterpret_cast<ImVec2*>( &Point ) );
			DrawList->PathArcTo( *reinterpret_cast<ImVec2*>( &C ), Rounding, A0, A1 );

			DrawList->PathFillConvex( Color );
		};

		DrawCorner( *reinterpret_cast<Vector2f*>( &ClipMin ), 1.f, 1.f, std::numbers::pi_v<float>, std::numbers::pi_v<float> * 1.5f, Popup ); // TL
		DrawCorner( Vector2f( ClipMax.x, ClipMin.y ), -1.f, 1.f, std::numbers::pi_v<float> * 1.5f, std::numbers::pi_v<float> * 2.f, Popup );  // TR
		DrawCorner( Vector2f( ClipMin.x, ClipMax.y ), 1.f, -1.f, std::numbers::pi_v<float> * .5f, std::numbers::pi_v<float>, Popup );         // BL
		DrawCorner( *reinterpret_cast<Vector2f*>( &ClipMax ), -1.f, -1.f, 0.f, std::numbers::pi_v<float> * .5f, Popup );                      // BR

		DrawList->AddRect( ClipMin, ClipMax, Border, Themes::ROUNDING, 0, 1.f );

		Cursor   = Original;
		Cursor.y = ClipMax.y;
	}

	bool ClassBrowser::WndProc( Vector2f CursorPosition, UINT Message, WPARAM wParam, LPARAM lParam )
	{
		const bool InBounds = CursorPosition.x > LastOrigin.x
		                      && CursorPosition.y > LastOrigin.y
		                      && CursorPosition.x < LastOrigin.x + LastSize.x
		                      && CursorPosition.y < LastOrigin.y + LastSize.y;

		switch ( Message )
		{
		case WM_MOUSEWHEEL:
			if ( InBounds )
			{
				const float Notches = static_cast<float>( GET_WHEEL_DELTA_WPARAM( wParam ) ) / WHEEL_DELTA;
				ScrollTarget        -= Notches * SCROLL_SPEED;
				return true;
			}
			break;

		case WM_LBUTTONDOWN:
			if ( InBounds )
			{
				const float   RelativeY = CursorPosition.y - LastOrigin.y + ScrollOffset;
				const int32_t HitIndex  = static_cast<int32_t>( RelativeY / LINE_HEIGHT );

				if ( LineAtIndex( HitIndex ) ) SelectedIndex = HitIndex;

				return true;
			}
			break;

		case WM_LBUTTONDBLCLK:
			if ( InBounds )
			{
				const float   RelativeY = CursorPosition.y - LastOrigin.y + ScrollOffset;
				const int32_t HitIndex  = static_cast<int32_t>( RelativeY / LINE_HEIGHT );

				if ( const auto* Line = LineAtIndex( HitIndex ) ) ActivateLine( *Line );

				return true;
			}
			break;

		case WM_KEYDOWN:
			// ActiveTab->RootView->OnKeyDown(wParam);
			return false;

		default:
			break;
		}

		return false;
	}

	void ClassBrowser::OnButtonTrigger( const Objects::Button* Instance )
	{
	}

	std::shared_ptr<Memory::Info> ClassBrowser::GetSelectedTemplate()
	{
		return SelectedTemplate;
	}

	void ClassBrowser::Initialize()
	{
		CurrentDirectory = ClassManager::GetRootPath();

		PopulateList();
	}
}
