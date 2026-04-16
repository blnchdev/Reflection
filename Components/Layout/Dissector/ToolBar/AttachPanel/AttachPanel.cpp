#include "AttachPanel.h"

#undef UNICODE
#include <tlhelp32.h>
#define UNICODE 1

#include <algorithm>
#include <chrono>
#include <d3d11.h>
#include <future>
#include <ranges>
#include <numbers>
#include <print>
#include <stack>
#include <utility>

#include "imgui.h"
#include "Components/FontManager/FontManager.h"
#include "Components/Graphics/Graphics.h"
#include "Components/Layout/Dissector/Grid/Grid.h"
#include "Components/Layout/Objects/Button/Button.h"
#include "Components/Notifications/Notifications.h"
#include "Components/Process Manager/Process.h"
#include "Components/Renderer/Renderer.h"
#include "Components/Theme Manager/Theme Manager.h"

namespace Renderer::Layout
{
	namespace
	{
		constexpr float SCROLL_SPEED     = 60.f;
		constexpr float SCROLL_DECAY     = 12.f;
		constexpr float SCROLL_SNAP_DIST = 0.5f;

		float ScrollOffset   = 0.f;
		float ScrollTarget   = 0.f;
		float ContentHeight  = 0.f;
		float ViewableHeight = 0.f;
		int   SelectedIndex  = -1;

		Vector2f LastOrigin = Vector2f::Zero;
		Vector2f LastSize   = Vector2f::Zero;

		std::chrono::high_resolution_clock::time_point LastFrameTime = std::chrono::high_resolution_clock::now();
		float                                          SpinnerTime   = 0.f;

		Objects::SearchBar SearchProcess( _( "\xF3\xB0\x8D\x89 Search..." ), &AttachPanel::OnChange, &AttachPanel::OnConfirm, {}, {} );
		Objects::Button    RefreshProcessesButton( _( "\xEE\xAC\xB7" ), &AttachPanel::OnButtonTrigger, Objects::ButtonType::Rectangle, {}, {} );
		Objects::Button    AttachButton( _( "Attach" ), &AttachPanel::OnButtonTrigger, Objects::ButtonType::Rectangle, {}, {} );

		struct IconTexture
		{
			ID3D11ShaderResourceView* SRV    = nullptr;
			int                       Width  = 0;
			int                       Height = 0;

			void Release()
			{
				if ( SRV )
				{
					SRV->Release();
					SRV = nullptr;
				}
			}
		};

		struct PendingEntry
		{
			uint32_t             PID;
			std::string          Name;
			std::vector<uint8_t> Pixels;
			int32_t              Width, Height;
		};

		std::future<std::vector<PendingEntry>> RefreshFuture = {};

		struct ProcessEntry
		{
			uint32_t    PID  = 0;
			std::string Name = {};
			IconTexture Icon;
		};

		[[nodiscard]] HICON GetProcessIcon( const uint32_t PID )
		{
			const HANDLE Process = OpenProcess( PROCESS_QUERY_LIMITED_INFORMATION, FALSE, PID );
			if ( !Process ) return nullptr;

			char  Path[ MAX_PATH ] = {};
			DWORD Size             = MAX_PATH;

			const bool HasPath = QueryFullProcessImageNameA( Process, 0, Path, &Size );
			CloseHandle( Process );

			if ( !HasPath ) return nullptr;

			SHFILEINFOA Info = {};
			SHGetFileInfoA( Path, 0, &Info, sizeof( Info ), SHGFI_ICON | SHGFI_SMALLICON );

			return Info.hIcon;
		}

		[[nodiscard]] std::vector<uint8_t> HIconToPixels( const HICON Icon, int32_t& OutWidth, int32_t& OutHeight )
		{
			if ( !Icon ) return {};

			ICONINFO Info = {};
			if ( !GetIconInfo( Icon, &Info ) ) return {};

			const auto CleanupBitmaps = [&]
			{
				if ( Info.hbmColor ) DeleteObject( Info.hbmColor );
				if ( Info.hbmMask ) DeleteObject( Info.hbmMask );
			};

			BITMAP BM = {};
			GetObject( Info.hbmColor ? Info.hbmColor : Info.hbmMask, sizeof( BM ), &BM );

			OutWidth  = BM.bmWidth;
			OutHeight = Info.hbmColor ? BM.bmHeight : BM.bmHeight / 2;

			const HDC ScreenDC = GetDC( nullptr );
			const HDC MemDC    = CreateCompatibleDC( ScreenDC );

			BITMAPINFOHEADER BIH = {};
			BIH.biSize           = sizeof( BIH );
			BIH.biWidth          = OutWidth;
			BIH.biHeight         = -OutHeight;
			BIH.biPlanes         = 1;
			BIH.biBitCount       = 32;
			BIH.biCompression    = BI_RGB;

			std::vector<uint8_t> Pixels( static_cast<size_t>( OutWidth * OutHeight * 4L ) );

			const HBITMAP DIBSection = CreateDIBSection( MemDC, reinterpret_cast<BITMAPINFO*>( &BIH ), DIB_RGB_COLORS, nullptr, nullptr, 0 );
			const HGDIOBJ OldBitmap  = SelectObject( MemDC, DIBSection );

			DrawIconEx( MemDC, 0, 0, Icon, OutWidth, OutHeight, 0, nullptr, DI_NORMAL );
			GetDIBits( MemDC, DIBSection, 0, OutHeight, Pixels.data(), reinterpret_cast<BITMAPINFO*>( &BIH ), DIB_RGB_COLORS );

			SelectObject( MemDC, OldBitmap );
			DeleteObject( DIBSection );
			DeleteDC( MemDC );
			ReleaseDC( nullptr, ScreenDC );
			CleanupBitmaps();

			for ( size_t i{ 0l }; std::cmp_less( i, OutWidth * OutHeight ); ++i )
			{
				uint8_t* P = Pixels.data() + i * 4ull;
				std::swap( P[ 0 ], P[ 2 ] );
			}

			return Pixels;
		}

		[[nodiscard]] IconTexture UploadTexture( const std::vector<uint8_t>& Pixels, int32_t Width, int32_t Height )
		{
			if ( Pixels.empty() || Width <= 0 || Height <= 0 ) return {};

			D3D11_TEXTURE2D_DESC Desc = {};
			Desc.Width                = Width;
			Desc.Height               = Height;
			Desc.MipLevels            = 1;
			Desc.ArraySize            = 1;
			Desc.Format               = DXGI_FORMAT_R8G8B8A8_UNORM;
			Desc.SampleDesc.Count     = 1;
			Desc.Usage                = D3D11_USAGE_DEFAULT;
			Desc.BindFlags            = D3D11_BIND_SHADER_RESOURCE;

			D3D11_SUBRESOURCE_DATA InitData = {};
			InitData.pSysMem                = Pixels.data();
			InitData.SysMemPitch            = Width * 4;

			ID3D11Texture2D* Texture = nullptr;
			if ( FAILED( Window::GetDevice()->CreateTexture2D(&Desc, &InitData, &Texture) ) ) return {};

			D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
			SRVDesc.Format                          = DXGI_FORMAT_R8G8B8A8_UNORM;
			SRVDesc.ViewDimension                   = D3D11_SRV_DIMENSION_TEXTURE2D;
			SRVDesc.Texture2D.MipLevels             = 1;
			SRVDesc.Texture2D.MostDetailedMip       = 0;

			IconTexture Result;
			Result.Width  = Width;
			Result.Height = Height;

			if ( FAILED( Window::GetDevice()->CreateShaderResourceView(Texture, &SRVDesc, &Result.SRV) ) )
			{
				Texture->Release();
				return {};
			}

			Texture->Release();
			return Result;
		}

		class ProcessList
		{
		public:
			void RefreshFiltered( const std::string& Filter )
			{
				if ( Filter.empty() )
				{
					Filtered = Entries;
					return;
				}

				std::string LowerFilter = Filter;
				ToLower( LowerFilter );

				Filtered = Entries | std::views::filter( [&] ( const ProcessEntry& Entry )
				{
					std::string Lower = Entry.Name;
					ToLower( Lower );
					return Lower.contains( LowerFilter );
				} ) | std::ranges::to<std::vector>(); // Love me some C++23 soup
			}

			size_t Size() const { return Entries.size(); }

			void clear()
			{
				for ( auto& Entry : Entries ) Entry.Icon.Release();
				Entries.clear();
			}

			ProcessEntry& emplace_back( uint32_t PID, std::string Name )
			{
				return Entries.emplace_back( PID, std::move( Name ) );
			}

			std::vector<ProcessEntry>* GetFiltered() { return &Filtered; }

		private:
			std::vector<ProcessEntry> Entries;
			std::vector<ProcessEntry> Filtered; // Non Owner

			static void ToLower( std::string& String )
			{
				std::ranges::transform( String, String.begin(), ::tolower );
			}
		};

		ProcessList Entries = {};

		std::vector<PendingEntry> GatherProcessEntries()
		{
			std::vector<PendingEntry> Result;

			const HANDLE Snapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );

			if ( Snapshot == INVALID_HANDLE_VALUE ) return Result;

			PROCESSENTRY32 PE = { .dwSize = sizeof( PROCESSENTRY32 ) };

			if ( !Process32First( Snapshot, &PE ) )
			{
				CloseHandle( Snapshot );
				return Result;
			}

			do
			{
				if ( PE.th32ProcessID == 0 ) continue; // Skip system process

				auto& Entry = Result.emplace_back();
				Entry.PID   = PE.th32ProcessID;
				Entry.Name  = PE.szExeFile;

				const HICON hIcon = GetProcessIcon( PE.th32ProcessID );
				Entry.Pixels      = HIconToPixels( hIcon, Entry.Width, Entry.Height );
				DestroyIcon( hIcon );
			} while ( Process32Next( Snapshot, &PE ) );

			CloseHandle( Snapshot );

			std::ranges::sort( Result, [] ( const PendingEntry& A, const PendingEntry& B )
			{
				return std::ranges::lexicographical_compare( A.Name, B.Name,
				                                             [] ( const unsigned char C1, const unsigned char C2 )
				                                             {
					                                             return std::tolower( C1 ) < std::tolower( C2 );
				                                             } );
			} );

			return Result;
		}

		void OnMouseWheel( const WPARAM wParam )
		{
			const float Delta = -static_cast<float>( GET_WHEEL_DELTA_WPARAM( wParam ) ) / WHEEL_DELTA;
			ScrollTarget += Delta * SCROLL_SPEED;

			const float MaxScroll = max( 0.f, ContentHeight - ViewableHeight );
			ScrollTarget          = std::clamp( ScrollTarget, 0.f, MaxScroll );
		}

		std::string Ellipsize( std::string_view String, const size_t MaxLength )
		{
			if ( String.size() <= MaxLength ) return std::string( String );
			return std::format( "{:.{}}...", String, MaxLength - 3 );
		}
	}


	void AttachPanel::Render( Vector2f Cursor, const Vector2f AvailableSize )
	{
		DrainPending();

		const uint32_t Background     = Themes::Manager::GetColor( "Global_WindowBackground"_H );
		const uint32_t Popup          = Themes::Manager::GetColor( "Global_PopupBackground"_H );
		const uint32_t Border         = Themes::Manager::GetColor( "Global_Border"_H );
		const uint32_t Text           = Themes::Manager::GetColor( "Global_Text"_H );
		SearchProcess.Override.Border = Border;
		SearchProcess.Font.Idle       = Text;

		const Vector2f Original = Cursor;

		const float Width = AvailableSize.x * 0.9f;

		SearchProcess.Resize();
		SearchProcess.Size.x = Width;
		Cursor.x += AvailableSize.x / 2.f - SearchProcess.Size.x / 2.f;
		Cursor.y += 10.f;
		SearchProcess.Position = Cursor;
		SearchProcess.Render();
		Cursor.x = Original.x + AvailableSize.x * 0.05f;
		Cursor.y += SearchProcess.Size.y + 10.f;

		RefreshProcessesButton.Position.x      = Cursor.x + Width - SearchProcess.Size.y;
		RefreshProcessesButton.Position.y      = SearchProcess.Position.y;
		RefreshProcessesButton.Size            = Vector2f( SearchProcess.Size.y, SearchProcess.Size.y );
		RefreshProcessesButton.Override.Border = Graphics::RGBA( 255, 0, 0, 0 );
		RefreshProcessesButton.Override.Idle   = Graphics::RGBA( 255, 0, 0, 0 );
		RefreshProcessesButton.IsRounded       = true;
		RefreshProcessesButton.FontSize        = 10.f;
		RefreshProcessesButton.Rounding        = Themes::ROUNDING;
		RefreshProcessesButton.Flags           = ImDrawFlags_RoundCornersRight;
		RefreshProcessesButton.Render();

		AttachButton.FontSize        = 18.f;
		AttachButton.IsRounded       = true;
		AttachButton.Override.Border = Border;
		AttachButton.Resize();

		const float HeightForAttachButton = 10.f + AttachButton.Size.y;
		const float Height                = AvailableSize.y - 10.f - ( Cursor.y - Original.y ) - HeightForAttachButton;

		Graphics::AddFilledRectangle( Cursor, Width, Height, Background, Themes::ROUNDING );

		ViewableHeight = Height;

		auto* Filtered = Entries.GetFiltered();

		constexpr float LineHeight = 20.f;
		ContentHeight              = LineHeight * static_cast<float>( Filtered->size() );

		const float MaxScroll = max( 0.f, ContentHeight - ViewableHeight );
		ScrollTarget          = std::clamp( ScrollTarget, 0.f, MaxScroll );
		ScrollOffset          = std::clamp( ScrollOffset, 0.f, MaxScroll );

		const auto  Now       = std::chrono::high_resolution_clock::now();
		const float DeltaTime = std::chrono::duration<float>( Now - LastFrameTime ).count();
		LastFrameTime         = Now;

		SpinnerTime += DeltaTime;

		if ( fabsf( ScrollTarget - ScrollOffset ) > SCROLL_SNAP_DIST ) ScrollOffset += ( ScrollTarget - ScrollOffset ) * ( 1.f - std::expf( -SCROLL_DECAY * DeltaTime ) );
		else ScrollOffset = ScrollTarget;

		Vector2f ScrolledCursor = Cursor;
		ScrolledCursor.y -= ScrollOffset;

		ImVec2 ClipMin( Cursor.x, Cursor.y );
		ImVec2 ClipMax( Cursor.x + Width, Cursor.y + Height );

		LastOrigin = Cursor;
		LastSize   = Vector2f{ Width, Height };

		ImGui::GetBackgroundDrawList()->PushClipRect( ClipMin, ClipMax, true );
		ImGui::GetForegroundDrawList()->PushClipRect( ClipMin, ClipMax, true );
		ImGui::GetWindowDrawList()->PushClipRect( ClipMin, ClipMax, true );

		const auto     szPID       = FontManager::FontSize( _( "55555" ), 16.f, true );
		const auto     szSeparator = FontManager::FontSize( _( " | " ), 16.f, true );
		const Vector2f szImage     = { LineHeight * 0.8f, LineHeight * 0.8f };

		const float oX         = ScrolledCursor.x;
		float       DelimitorX = 0.f;

		if ( Entries.Size() == 0 )
		{
			const Vector2f Center = { LastOrigin.x + Width / 2.f, LastOrigin.y + Height / 2.f };

			constexpr int   NumDots   = 8;
			constexpr float Radius    = 8.f;
			constexpr float Speed     = 10.f;
			constexpr float DotRadius = 2.f;

			const int ActiveDot = static_cast<int>( SpinnerTime * Speed ) % NumDots;

			for ( int i = 0; i < NumDots; ++i )
			{
				const float t = std::numbers::pi_v<float> * 2.f / NumDots * i;

				const Vector2f DotPosition =
				{
					Center.x + cosf( t ) * Radius,
					Center.y + sinf( t ) * Radius
				};

				const int      Distance = ( ActiveDot - i + NumDots ) % NumDots;
				const float    Alpha    = 1.f - static_cast<float>( Distance ) / NumDots;
				const uint32_t DotColor = Text & ~IM_COL32_A_MASK | static_cast<ImU32>( Alpha * 255.f ) << IM_COL32_A_SHIFT;

				Graphics::AddFilledCircle( DotPosition, DotRadius, DotColor );
			}

			FontManager::AddText( _( "Gathering processes..." ), 14.f, Text, { Center.x, Center.y + Radius + 10.f }, false, DT_CENTER, 0.f );
		}
		else
		{
			for ( const auto&& [ Index, Entry ] : *Filtered | std::views::enumerate )
			{
				const bool IsSelected = Index == SelectedIndex;

				const float LineTop    = ScrolledCursor.y;
				const float LineBottom = LineTop + LineHeight;

				const bool LineVisible = LineBottom >= ClipMin.y && LineTop <= ClipMin.y + ViewableHeight;

				if ( LineVisible )
				{
					if ( IsSelected )
					{
						const uint32_t Selected = Themes::Manager::GetColor( "MemoryGrid_Selected"_H );
						Graphics::AddFilledRectangle( ScrolledCursor, Width, LineHeight, Selected );
					}

					ScrolledCursor.x += 8.f;
					ScrolledCursor.y += LineHeight / 2.f;

					ScrolledCursor.y -= szImage.y / 2.f;

					Graphics::AddImage( Entry.Icon.SRV, ScrolledCursor, szImage, Graphics::RGBA( 255, 255, 255, 255 ) );
					// Graphics::AddRectangle( ScrolledCursor, szImage.x, szImage.y, Border, 1.f, Themes::ROUNDING );

					ScrolledCursor.x += szImage.x + 5.f;
					ScrolledCursor.y += szImage.y / 2.f;

					std::string PID = std::to_string( Entry.PID );
					FontManager::AddText( PID, 16.f, Text, ScrolledCursor, true, DT_VCENTER );
					ScrolledCursor.x += szPID.x;

					DelimitorX = ScrolledCursor.x + szSeparator.x / 2.f;

					ScrolledCursor.x += szSeparator.x;

					std::string ProcessName = Ellipsize( Entry.Name, 19 );
					FontManager::AddText( ProcessName, 16.f, Text, ScrolledCursor, false, DT_VCENTER );

					ScrolledCursor.x = oX;
					ScrolledCursor.y += LineHeight / 2.f;
				}
				else
				{
					ScrolledCursor.y += LineHeight;
				}
			}
		}

		Graphics::AddLine( Vector2f( DelimitorX, ClipMin.y ), Vector2f( DelimitorX, ClipMax.y ), Border, 1.f );

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

		const float RemainingHeight = AvailableSize.y - ( Cursor.y - Original.y );

		AttachButton.Size.x *= 2.f;
		const auto Size       = AttachButton.Size;
		AttachButton.Position = Vector2f( Cursor.x + AvailableSize.x / 2.f - Size.x / 2.f, Cursor.y + RemainingHeight / 2.f - Size.y / 2.f );
		AttachButton.Render();

		LastFrameTime = std::chrono::high_resolution_clock::now();
	}

	bool AttachPanel::WndProc( const Vector2f CursorPosition, const UINT Message, const WPARAM wParam, const LPARAM lParam )
	{
		switch ( Message )
		{
		case WM_MOUSEWHEEL:
			if ( CursorPosition.x > LastOrigin.x && CursorPosition.y > LastOrigin.y && CursorPosition.x < LastOrigin.x + LastSize.x && CursorPosition.y < LastOrigin.y + LastSize.y )
			{
				OnMouseWheel( wParam );
				return true;
			}
			break;
		case WM_LBUTTONDOWN:
			{
				const bool InBounds = CursorPosition.x > LastOrigin.x
				                      && CursorPosition.y > LastOrigin.y
				                      && CursorPosition.x < LastOrigin.x + LastSize.x
				                      && CursorPosition.y < LastOrigin.y + LastSize.y;

				if ( InBounds )
				{
					const float RelativeY = CursorPosition.y - LastOrigin.y + ScrollOffset;
					const int   HitIndex  = static_cast<int>( RelativeY / 20.f ); // LineHeight
					const auto* Filtered  = Entries.GetFiltered();

					if ( HitIndex >= 0 && std::cmp_less( HitIndex, Filtered->size() ) ) SelectedIndex = HitIndex;

					return true;
				}
				break;
			}
		case WM_KEYDOWN:
			// ActiveTab->RootView->OnKeyDown(wParam);
			return false;
		default:
			break;
		}

		if ( AttachButton.WndProc( CursorPosition, Message, wParam, lParam ) ) return true;
		if ( RefreshProcessesButton.WndProc( CursorPosition, Message, wParam, lParam ) ) return true;
		if ( SearchProcess.WndProc( CursorPosition, Message, wParam, lParam ) ) return true;

		return false;
	}

	void AttachPanel::OnSelect()
	{
		RefreshFuture = std::async( std::launch::async, GatherProcessEntries );
		Entries.RefreshFiltered( SearchProcess.Query );
	}

	void AttachPanel::DrainPending()
	{
		if ( !RefreshFuture.valid() ) return;
		if ( RefreshFuture.wait_for( std::chrono::seconds( 0 ) ) != std::future_status::ready ) return;

		auto Pending = RefreshFuture.get();

		Entries.clear();

		for ( const auto& [ PID, Name, Pixels, Width, Height ] : Pending )
		{
			auto& E = Entries.emplace_back( PID, Name );
			E.Icon  = UploadTexture( Pixels, Width, Height ); // We do pixels to D3D11Texture in main thread to avoid racing conditions(?) with ID3D11Device
		}

		Pending.clear();
		Entries.RefreshFiltered( SearchProcess.Query );
	}

	void AttachPanel::OnChange( const Objects::SearchBar* Instance )
	{
		Entries.RefreshFiltered( Instance->Query );
	}

	void AttachPanel::OnConfirm( const Objects::SearchBar* Instance )
	{
		Entries.RefreshFiltered( Instance->Query );
	}

	void AttachPanel::OnButtonTrigger( const Objects::Button* Instance )
	{
		if ( Instance->IDX == RefreshProcessesButton.IDX )
		{
			RefreshFuture = std::async( std::launch::async, GatherProcessEntries );
			return;
		}

		if ( Instance->IDX == AttachButton.IDX )
		{
			const auto* Filtered = Entries.GetFiltered();

			if ( SelectedIndex < 0 || std::cmp_greater_equal( SelectedIndex, Filtered->size() ) ) return;

			const auto& Entry = Filtered->at( SelectedIndex );

			const auto Status = Process::Attach( Entry.PID );
			Grid::OnAttachProcess( Entry.PID, Status );

			const auto Label = std::string( Process::StatusWhat( Status ) );

			// static void AddNotification( const std::string& Label, const std::string& Description, uint32_t Color = Success, bool Persistent = false );

			switch ( Status )
			{
			case ProcessStatus::S_ALREADY_ATTACHED:
			case ProcessStatus::S_ATTACHED:
				{
					const std::string Body = std::format( "Successfully attached to process '{}' (PID: {})", Entry.Name, Entry.PID );
					NotificationManager::AddNotification( Label, Body, Success, false );
				}
				break;
			case ProcessStatus::E_ACCESS_DENIED:
			case ProcessStatus::E_NO_HIJACK:
				{
					const std::string Body = std::format( "Failed to attach to process '{}' (PID: {})", Entry.Name, Entry.PID );
					NotificationManager::AddNotification( Label, Body, Error, false );
				}
				break;
			}
		}
	}
}
