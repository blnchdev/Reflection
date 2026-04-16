#pragma once
#include <d3d11.h>
#include <stop_token>

#include "Libraries/Vector.h"

#include "Windows.h"

namespace Renderer
{
	struct WindowData
	{
		float X,     Y;
		float Width, Height;
	};

	enum CursorTypes : uint8_t
	{
		Pointer,
		Hand
	};

	class Window
	{
	public:
		static Vector2f GetCursorPosition();
		static void     SetCursorType( CursorTypes Type );

		static ID3D11Device* GetDevice();
		static bool          Initialize();
		static void          Execute();
		static void          Destroy();
		static void          RequestStop();

		static HWND        GetHandle();
		static WindowData* GetWindowData();
		static void        ApplyWindowData();
		static bool        Maximized();

		static void Close();
		static void Minimize();
	};
}
