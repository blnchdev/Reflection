#include "Renderer.h"
#include <d3d11.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>
#include <Definitions/Globals.h>
#include <cstdio>
#include <random>
#include <dwmapi.h>
// #include "RenderStages/RenderMaster.h"
#include <chrono>
#include <thread>
#include <windowsx.h>

// #include "Graphics/Graphics.h"
#include "ProcessIcon.h"
#include "Components/FontManager/FontManager.h"
#include "Components/Graphics/Graphics.h"
#include "Components/Layout/LayoutMaster.h"
#include "Components/Notifications/Notifications.h"
#include "Components/Theme Manager/Theme Manager.h"
#include "Components/Title Bar/TitleBar.h"
#include "Libraries/Vector.h"
#include <gdiplus.h>
#include <print>
#include <Shlwapi.h>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler( HWND, UINT, WPARAM, LPARAM );

namespace Renderer
{
	namespace
	{
		ID3D11Device*           pDevice           = nullptr;
		ID3D11DeviceContext*    pDeviceContext    = nullptr;
		IDXGISwapChain*         pSwapChain        = nullptr;
		ID3D11RenderTargetView* pMainRTV          = nullptr;
		ID3D11Buffer*           pBackBuffer       = nullptr;
		HWND                    hWindow           = nullptr;
		constexpr UINT          CreateDeviceFlags = 0;
		D3D_FEATURE_LEVEL       FeatureLevel;
		WindowData              Data              = { 0.f, 0.f, 0.f, 0.f };
		Vector2f                CursorPos         = Vector2f::Zero;
		bool                    IsWindowMaximized = false;
		bool                    IsDragged         = false;
		std::stop_source        StopSource;

		const HCURSOR hPointer = LoadCursor( nullptr, IDC_ARROW );
		const HCURSOR hHand    = LoadCursor( nullptr, IDC_HAND );

		std::optional<HCURSOR> CurrentCursor = std::nullopt;

		struct ACCENTPOLICY
		{
			int nAccentState;
			int nFlags;
			int nColor;
			int nAnimationId;
		};

		struct WINCOMPATTRDATA
		{
			int   nAttribute;
			PVOID pData;
			ULONG ulDataSize;
		};

		enum ACCENT_STATE
		{
			ACCENT_DISABLED                   = 0,
			ACCENT_ENABLE_GRADIENT            = 1,
			ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
			ACCENT_ENABLE_BLURBEHIND          = 3,
			ACCENT_ENABLE_ACRYLICBLURBEHIND   = 4,
			ACCENT_ENABLE_HOSTBACKDROP        = 5,
			ACCENT_INVALID_STATE              = 6
		};

		void SetTransparency( HWND hWnd )
		{
			ACCENTPOLICY    Policy  = { ACCENT_ENABLE_TRANSPARENTGRADIENT, 2, 0x00000000, 0 };
			WINCOMPATTRDATA WinData = { 19, &Policy, sizeof( Policy ) };

			using SetWindowCompositionAttribute_t      = BOOL(WINAPI*)( HWND, WINCOMPATTRDATA* );
			static auto User32                         = GetModuleHandleW( L"user32.dll" );
			static auto fSetWindowCompositionAttribute = reinterpret_cast<SetWindowCompositionAttribute_t>( GetProcAddress( User32, "SetWindowCompositionAttribute" ) );

			if ( fSetWindowCompositionAttribute ) fSetWindowCompositionAttribute( hWnd, &WinData );
		}

		const wchar_t* CreateRandomName( const uint16_t Length )
		{
			static constexpr wchar_t Characters[ ] = L"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
			constexpr size_t         CharLength    = sizeof( Characters ) / sizeof( wchar_t ) - 1;

			std::random_device                    Device;
			std::mt19937                          Generator( Device() );
			std::uniform_int_distribution<size_t> Dist( 0, CharLength - 1 );

			static std::vector<wchar_t> ToReturn;
			ToReturn.clear();
			ToReturn.reserve( Length + 1 );

			for ( size_t i = 0; i < Length; ++i )
			{
				ToReturn.emplace_back( Characters[ Dist( Generator ) ] );
			}

			ToReturn.emplace_back( L'\0' );

			return ToReturn.data();
		}

		void CreateRenderTarget()
		{
			ID3D11Texture2D* BackBuffer = nullptr;
			( void )pSwapChain->GetBuffer( 0, IID_PPV_ARGS( &BackBuffer ) );
			( void )pDevice->CreateRenderTargetView( BackBuffer, nullptr, &pMainRTV );
			BackBuffer->Release();
		}

		void CleanupRenderTarget()
		{
			if ( pMainRTV )
			{
				pMainRTV->Release();
				pMainRTV = nullptr;
			}
		}

		LRESULT WINAPI WndProc( const HWND hWnd, const UINT Message, const WPARAM wParam, const LPARAM lParam )
		{
			// Handle this before imgui because we want authority over the cursor (we use our own buttons!)
			if ( Message == WM_SETCURSOR && LOWORD( lParam ) == HTCLIENT )
			{
				return true;
			}

			if ( ImGui_ImplWin32_WndProcHandler( hWnd, Message, wParam, lParam ) ) return true;

			POINT Point;
			GetCursorPos( &Point );
			ScreenToClient( hWindow, &Point );

			CursorPos = Vector2f{ static_cast<float>( Point.x ), static_cast<float>( Point.y ) };

			// TODO: Handle this in LayoutMaster
			Titlebar::WndProc( Message );

			switch ( Message )
			{
			case WM_KEYDOWN:
				{
					if ( wParam == VK_RIGHT )
					{
						// TODO: Remove this once Toolbar is done, this is just a debug swap between themes
						Themes::ThemeID Theme = Themes::Manager::GetTheme();
						uint8_t         NewID = ( static_cast<uint8_t>( Theme ) + 1 );
						if ( NewID == ( uint8_t )Themes::ThemeID::Count ) NewID = 0;
						Themes::Manager::SetTheme( static_cast<Themes::ThemeID>( NewID ) );
					}
				}
				break;
			case WM_SIZE:
				{
					if ( pDevice != nullptr && wParam != SIZE_MINIMIZED )
					{
						CleanupRenderTarget();
						( void )pSwapChain->ResizeBuffers( 0, LOWORD( lParam ), HIWORD( lParam ), DXGI_FORMAT_UNKNOWN, 0 );
						CreateRenderTarget();
					}
				}
				break;
			case WM_CLOSE:
				PostQuitMessage( 0 );
				break;
			case WM_SYSCOMMAND:
				if ( ( wParam & 0xFFF0 ) == SC_MOVE ) IsDragged = true;
				break;
			case WM_EXITSIZEMOVE:
				IsDragged = false;
				break;
			case WM_NCHITTEST:
				return Titlebar::HandleNcHitTest( lParam );
			}

			if ( LayoutMaster::WndProc( CursorPos, Message, wParam, lParam ) ) return true;

			return ::DefWindowProcW( hWnd, Message, wParam, lParam );
		}

		HICON LoadEmbeddedIcon()
		{
			IStream* Stream = SHCreateMemStream( Icons::PNG, Icons::Length );

			const Gdiplus::GdiplusStartupInput GDIPInput;
			ULONG_PTR                          GDIPToken;

			Gdiplus::GdiplusStartup( &GDIPToken, &GDIPInput, nullptr );

			auto* Bitmap = Gdiplus::Bitmap::FromStream( Stream );
			Stream->Release();

			HICON hIcon = nullptr;
			Bitmap->GetHICON( &hIcon );
			delete Bitmap;

			Gdiplus::GdiplusShutdown( GDIPToken );
			return hIcon;
		}
	}

	void Window::Close()
	{
		WndProc( hWindow, WM_CLOSE, 0, 0 );
	}

	void Window::Minimize()
	{
		ShowWindow( hWindow, SW_MINIMIZE );
	}

	Vector2f Window::GetCursorPosition()
	{
		return CursorPos;
	}

	void Window::SetCursorType( const CursorTypes Type )
	{
		switch ( Type )
		{
		case Hand:
			CurrentCursor = hHand;
			break;
		case Pointer:
			CurrentCursor = std::nullopt; // This is default anyways so
			break;
		default:
			break;
		}
	}

	ID3D11Device* Window::GetDevice()
	{
		return pDevice;
	}

	bool Window::Initialize()
	{
		const auto nScreenWidth  = static_cast<float>( GetSystemMetrics( SM_CXSCREEN ) );
		const auto nScreenHeight = static_cast<float>( GetSystemMetrics( SM_CYSCREEN ) );

		Data.Width  = 0.8f * nScreenWidth;
		Data.Height = 0.8f * nScreenHeight;
		Data.X      = nScreenWidth * 0.5f - Data.Width * 0.5f;
		Data.Y      = nScreenHeight * 0.5f - Data.Height * 0.5f;

		const wchar_t* WindowName = CreateRandomName( 15 );

		const int X      = static_cast<int>( Data.X );
		const int Y      = static_cast<int>( Data.Y );
		const int Width  = static_cast<int>( Data.Width );
		const int Height = static_cast<int>( Data.Height );

		const HICON hIcon = LoadEmbeddedIcon();

		const auto NormalCursor = LoadCursorW( nullptr, IDC_ARROW );

		const WNDCLASSEX ClassEx = { sizeof( WNDCLASSEX ), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle( nullptr ), hIcon, NormalCursor, nullptr, nullptr, WindowName, hIcon };

		RegisterClassEx( &ClassEx );

		hWindow = CreateWindowExW(
		                          0,
		                          ClassEx.lpszClassName,
		                          WindowName,
		                          WS_POPUP | WS_MINIMIZEBOX | WS_SYSMENU,
		                          X, Y,
		                          Width,
		                          Height,
		                          nullptr, nullptr,
		                          ClassEx.hInstance,
		                          nullptr
		                         );

		ShowWindow( hWindow, SW_SHOWDEFAULT );
		UpdateWindow( hWindow );
		SetTransparency( hWindow );

		DXGI_SWAP_CHAIN_DESC sc;
		ZeroMemory( &sc, sizeof sc );

		sc.BufferCount                        = 2;
		sc.BufferDesc.Width                   = 0;
		sc.BufferDesc.Height                  = 0;
		sc.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
		sc.BufferDesc.RefreshRate.Numerator   = 30;
		sc.BufferDesc.RefreshRate.Denominator = 1;
		sc.Flags                              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		sc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sc.OutputWindow                       = hWindow;
		sc.SampleDesc.Count                   = 1;
		sc.SampleDesc.Quality                 = 0;
		sc.Windowed                           = TRUE;
		sc.SwapEffect                         = DXGI_SWAP_EFFECT_SEQUENTIAL;

		HRESULT Result = D3D11CreateDeviceAndSwapChain( nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, CreateDeviceFlags, nullptr, 0, D3D11_SDK_VERSION, &sc, &pSwapChain, &pDevice, &FeatureLevel, &pDeviceContext );

		if ( !SUCCEEDED( Result ) )
		{
			std::println( "[-] CreateDeviceAndSwapChain Failure: 0x{:X}", Result );
			return false;
		}

		if ( Result = pSwapChain->GetBuffer( 0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>( &pBackBuffer ) ); Result != S_OK )
		{
			std::println( "[-] pSwapChain->GetBuffer Failure: 0x{:X}", Result );
			return false;
		}

		if ( Result = pDevice->CreateRenderTargetView( pBackBuffer, nullptr, &pMainRTV ); Result != S_OK )
		{
			std::println( "[-] pDevice->CreateRenderTargetView Failure: 0x{:X}", Result );
			return false;
		}

		pBackBuffer->Release();

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io    = ImGui::GetIO();
		io.IniFilename = nullptr;
		io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
		ImGui_ImplWin32_Init( hWindow );
		ImGui_ImplDX11_Init( pDevice, pDeviceContext );
		//ImGui::StyleColorsDark();
		return true;
	}

	void Window::Execute()
	{
		MSG Message;
		ZeroMemory( &Message, sizeof(Message) );

		float ClearColor[ 4 ] = { 0.f, 0.f, 0.f, 0.f };
		RECT  WindowRect;

		using namespace std::chrono;
		constexpr auto FrameDuration = milliseconds( 17 ); // ~60fps (maybe export this to config?)

		IsWindowMaximized               = false;
		WINDOWPLACEMENT WindowPlacement = { sizeof( WindowPlacement ) };

		constexpr auto Flags = ImGuiWindowFlags_NoTitleBar |
		                       ImGuiWindowFlags_NoResize |
		                       ImGuiWindowFlags_NoMove |
		                       ImGuiWindowFlags_NoScrollbar |
		                       ImGuiWindowFlags_NoScrollWithMouse |
		                       ImGuiWindowFlags_NoCollapse |
		                       ImGuiWindowFlags_NoBackground |
		                       ImGuiWindowFlags_NoSavedSettings |
		                       ImGuiWindowFlags_NoBringToFrontOnFocus;

		while ( Message.message != WM_QUIT && !StopSource.stop_requested() )
		{
			auto FrameStart = high_resolution_clock::now();
			CurrentCursor   = std::nullopt;

			while ( PeekMessage( &Message, nullptr, 0U, 0U, PM_REMOVE ) )
			{
				TranslateMessage( &Message );
				DispatchMessage( &Message );
			}

			if ( Message.message == WM_QUIT ) break;

			GetWindowPlacement( hWindow, &WindowPlacement );
			IsWindowMaximized = WindowPlacement.showCmd == SW_MAXIMIZE;

			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			GetWindowRect( hWindow, &WindowRect );
			Data.X      = static_cast<float>( WindowRect.left );
			Data.Y      = static_cast<float>( WindowRect.top );
			Data.Width  = static_cast<float>( WindowRect.right ) - Data.X;
			Data.Height = static_cast<float>( WindowRect.bottom ) - Data.Y;

			auto& IO         = ImGui::GetIO();
			IO.DisplaySize.x = Data.Width;
			IO.DisplaySize.y = Data.Height;

			ImGui::SetNextWindowPos( { 0, 0 }, ImGuiCond_Always );
			ImGui::SetNextWindowSize( IO.DisplaySize, ImGuiCond_Always );

			ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0, 0 ) );
			ImGui::PushStyleVar( ImGuiStyleVar_WindowBorderSize, 0.f );
			ImGui::PushStyleColor( ImGuiCol_WindowBg, ImVec4( 0, 0, 0, 0 ) );

			ImGui::Begin( _( "##main" ), nullptr, Flags );

			const bool Round = !Maximized();

			if ( Round )
			{
				Graphics::AddFilledRectangle( Vector2f::Zero, Data.Width, Data.Height, Themes::Manager::GetColor( "Global_WindowBackground"_H ), Themes::ROUNDING );
			}
			else
			{
				Graphics::AddFilledRectangle( Vector2f::Zero, Data.Width, Data.Height, Themes::Manager::GetColor( "Global_WindowBackground"_H ), 0 );
			}

			LayoutMaster::Present();

			NotificationManager::Render();
			Titlebar::Render();

			ImGui::End();

			ImGui::PopStyleColor();
			ImGui::PopStyleVar( 2 );

			pDeviceContext->OMSetRenderTargets( 1, &pMainRTV, nullptr );
			pDeviceContext->ClearRenderTargetView( pMainRTV, reinterpret_cast<FLOAT*>( &ClearColor ) );
			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData() );
			( void )pSwapChain->Present( 1, 0 );

			SetCursor( CurrentCursor.value_or( hPointer ) );

			auto FrameEnd = high_resolution_clock::now();
			auto Elapsed  = duration_cast<milliseconds>( FrameEnd - FrameStart );

			if ( Elapsed < FrameDuration ) std::this_thread::sleep_for( FrameDuration - Elapsed );
		}

		Destroy();
	}

	WindowData* Window::GetWindowData()
	{
		return &Data;
	}

	void Window::ApplyWindowData()
	{
		MoveWindow( hWindow, static_cast<int>( Data.X ), static_cast<int>( Data.Y ), static_cast<int>( Data.Width ), static_cast<int>( Data.Height ), TRUE );
		ImGuiIO& IO    = ImGui::GetIO();
		IO.DisplaySize = ImVec2( Data.Width, Data.Height );
	}

	bool Window::Maximized()
	{
		return IsWindowMaximized;
	}

	void Window::Destroy()
	{
	}

	void Window::RequestStop()
	{
		StopSource.request_stop();
	}

	HWND Window::GetHandle()
	{
		return hWindow;
	}
}
