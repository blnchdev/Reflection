#include "TypePicker.h"

#include <algorithm>
#include <array>
#include <span>

#include "Components/FontManager/FontManager.h"
#include "Components/Graphics/Graphics.h"
#include "Components/Layout/Dissector/Dissector.h"
#include "Components/Layout/Dissector/Grid/Grid.h"

namespace Renderer::Layout
{
	namespace
	{
		class TypedButton : public Objects::Button
		{
		public:
			DataStructureType BoundType = T_x64;
		};

		TypedButton MakeTypeButton( const char* Label, const Objects::Button::OverrideData& Data, const DataStructureType Type )
		{
			TypedButton Button;
			Button.Label     = Label;
			Button.FontSize  = 16.f;
			Button.IsRounded = true;
			Button.BoundType = Type;

			Button.Override = Data;
			Button.Callback = &TypePicker::OnTypedButtonTrigger;
			Button.Type     = Objects::ButtonType::Rectangle;

			return Button;
		}

		// TODO: Somehow export this to theme
		namespace
		{
			constexpr Objects::Button::OverrideData PaddingData =
			{
				.Idle = Graphics::RGBA( 128, 128, 128, 20 ),
				.Hover = Graphics::RGBA( 128, 128, 128, 38 ),
				.Click = Graphics::RGBA( 128, 128, 128, 55 ),
				.Border = Graphics::RGBA( 128, 128, 128, 50 ),
			};

			Objects::Button::OverrideData HexData =
			{
				.Idle = Graphics::RGBA( 80, 120, 255, 25 ),
				.Hover = Graphics::RGBA( 80, 120, 255, 45 ),
				.Click = Graphics::RGBA( 80, 120, 255, 65 ),
				.Border = Graphics::RGBA( 80, 120, 255, 60 ),
			};

			Objects::Button::OverrideData UIntData =
			{
				.Idle = Graphics::RGBA( 80, 200, 80, 25 ),
				.Hover = Graphics::RGBA( 80, 200, 80, 45 ),
				.Click = Graphics::RGBA( 80, 200, 80, 65 ),
				.Border = Graphics::RGBA( 80, 200, 80, 60 ),
			};

			Objects::Button::OverrideData IntData =
			{
				.Idle = Graphics::RGBA( 220, 110, 80, 25 ),
				.Hover = Graphics::RGBA( 220, 110, 80, 45 ),
				.Click = Graphics::RGBA( 220, 110, 80, 65 ),
				.Border = Graphics::RGBA( 220, 110, 80, 60 ),
			};

			Objects::Button::OverrideData FloatData =
			{
				.Idle = Graphics::RGBA( 170, 100, 230, 25 ),
				.Hover = Graphics::RGBA( 170, 100, 230, 45 ),
				.Click = Graphics::RGBA( 170, 100, 230, 65 ),
				.Border = Graphics::RGBA( 170, 100, 230, 60 ),
			};

			Objects::Button::OverrideData PtrData =
			{
				.Idle = Graphics::RGBA( 60, 200, 200, 25 ),
				.Hover = Graphics::RGBA( 60, 200, 200, 45 ),
				.Click = Graphics::RGBA( 60, 200, 200, 65 ),
				.Border = Graphics::RGBA( 60, 200, 200, 60 ),
			};

			Objects::Button::OverrideData StrData =
			{
				.Idle = Graphics::RGBA( 230, 130, 140, 25 ),
				.Hover = Graphics::RGBA( 230, 130, 140, 45 ),
				.Click = Graphics::RGBA( 230, 130, 140, 65 ),
				.Border = Graphics::RGBA( 230, 130, 140, 60 ),
			};

			Objects::Button::OverrideData VecData =
			{
				.Idle = Graphics::RGBA( 220, 170, 50, 25 ),
				.Hover = Graphics::RGBA( 220, 170, 50, 45 ),
				.Click = Graphics::RGBA( 220, 170, 50, 65 ),
				.Border = Graphics::RGBA( 220, 170, 50, 60 ),
			};

			Objects::Button::OverrideData ArrData =
			{
				.Idle = Graphics::RGBA( 210, 100, 210, 25 ),
				.Hover = Graphics::RGBA( 210, 100, 210, 45 ),
				.Click = Graphics::RGBA( 210, 100, 210, 65 ),
				.Border = Graphics::RGBA( 210, 100, 210, 60 ),
			};
		}

		std::array Paddings  = { MakeTypeButton( _( "x8" ), PaddingData, T_x8 ), MakeTypeButton( _( "x16" ), PaddingData, T_x16 ), MakeTypeButton( _( "x32" ), PaddingData, T_x32 ), MakeTypeButton( _( "x64" ), PaddingData, T_x64 ) };
		std::array UInts     = { MakeTypeButton( _( "u8" ), UIntData, T_uint8 ), MakeTypeButton( _( "u16" ), UIntData, T_uint16 ), MakeTypeButton( _( "u32" ), UIntData, T_uint32 ), MakeTypeButton( _( "u64" ), UIntData, T_uint64 ) };
		std::array SInts     = { MakeTypeButton( _( "i8" ), IntData, T_int8 ), MakeTypeButton( _( "i16" ), IntData, T_int16 ), MakeTypeButton( _( "i32" ), IntData, T_int32 ), MakeTypeButton( _( "i64" ), IntData, T_int64 ) };
		std::array Hex       = { MakeTypeButton( _( "BYTE" ), HexData, T_byte ), MakeTypeButton( _( "WORD" ), HexData, T_word ), MakeTypeButton( _( "DWORD" ), HexData, T_dword ), MakeTypeButton( _( "QWORD" ), HexData, T_qword ) };
		std::array Floating  = { MakeTypeButton( _( "f32" ), FloatData, T_float ), MakeTypeButton( _( "f64" ), FloatData, T_double ) };
		std::array Pointers  = { MakeTypeButton( _( "void*" ), PtrData, T_rawPointer ), MakeTypeButton( _( "Cls*" ), PtrData, T_pClass ), MakeTypeButton( _( "[Cls]" ), PtrData, T_EmbeddedClass ), MakeTypeButton( _( "VMT" ), PtrData, T_VMT ) };
		std::array Pointers2 = { MakeTypeButton( _( "F()*" ), PtrData, T_pFunction ) };
		std::array CharPtr   = { MakeTypeButton( _( "char*" ), StrData, T_charPtr_array ), MakeTypeButton( _( "wide*" ), StrData, T_widePtr_array ) };

		std::array Vectors = { MakeTypeButton( _( "Vec2f" ), VecData, T_Vector2f ), MakeTypeButton( _( "Vec3f" ), VecData, T_Vector3f ), MakeTypeButton( _( "Vec4f" ), VecData, T_Vector4f ), MakeTypeButton( _( "M4x4" ), VecData, T_matrix4x4 ) };
		std::array Arrays  = { MakeTypeButton( _( "arr[]" ), ArrData, T_undefined ), MakeTypeButton( _( "char[]" ), ArrData, T_char_array ), MakeTypeButton( _( "wide[]" ), ArrData, T_wide_array ) };
	}

	void TypePicker::OnSelect( Vector2f AvailableSize )
	{
	}

	void TypePicker::OnTypedButtonTrigger( Objects::Button* Instance )
	{
		// This is guaranteed to be a TypedButton* as only TypedButtons set this as a callback
		// **IF** this is ever changed, this static_cast is dangerous as we'll be reading garbage data
		const auto Typed = static_cast<TypedButton*>( Instance ); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)

		Grid::ChangeSelected( Typed->BoundType );
	}

	void TypePicker::Render( Vector2f Cursor, const Vector2f AvailableSize )
	{
		const uint32_t MutedText = Themes::Manager::GetColor( "Global_MutedText"_H );

		const Vector2f Copy             = Cursor;
		const auto     Size             = Vector2f{ AvailableSize.x / 5.f, AvailableSize.x / 5.f };
		const auto     Padding          = Size.x / 4.f;
		const auto     DelimiterSize    = AvailableSize.x * 0.9f;
		const auto     DelimitedPadding = ( AvailableSize.x - DelimiterSize ) / 2.f;
		const auto     HeaderSize       = FontManager::FontSize( _( "HEX" ), 14.f, false );

		const float ButtonOffset = AvailableSize.x / 30.f;

		auto DrawButtonPack = [&] ( const std::span<TypedButton>& Buttons )
		{
			float StartX    = Cursor.x;
			float MaxHeight = 0.f;

			for ( auto& Button : Buttons )
			{
				Button.Resize();
				Button.Size.x   = Size.x;
				Button.Rounding = 5.f;
				MaxHeight       = max( MaxHeight, Button.Size.y );
				Button.Position = { StartX, Cursor.y };
				Button.Render();

				StartX += Size.x + ButtonOffset;
			}

			Cursor.y += MaxHeight + Padding;
		};

		auto DrawLabel = [&] ( const std::string& Label )
		{
			Cursor.x = Copy.x + DelimitedPadding;
			Cursor.y += HeaderSize.y / 2.f;
			FontManager::AddText( Label, 14.f, MutedText, Cursor, false, DT_VCENTER );
			Cursor.y += HeaderSize.y / 2.f + Padding / 2.f;
		};

		auto DrawDelimiter = [&]()
		{
			Cursor.x = Copy.x + DelimitedPadding;
			Graphics::AddLine( Cursor, Vector2f( Cursor.x + DelimiterSize, Cursor.y ), MutedText, 0.5f );
			Cursor.x = Copy.x;
			Cursor.y += Padding;
		};

		Cursor.y += Padding;

		DrawLabel( _( "PADDING" ) );
		DrawButtonPack( Paddings );

		DrawDelimiter();

		DrawLabel( _( "HEX" ) );
		DrawButtonPack( Hex );
		DrawLabel( _( "UNSIGNED INT" ) );
		DrawButtonPack( UInts );
		DrawLabel( _( "SIGNED INT" ) );
		DrawButtonPack( SInts );
		DrawLabel( _( "FLOAT" ) );
		DrawButtonPack( Floating );

		DrawDelimiter();

		DrawLabel( _( "POINTERS & CLASS" ) );
		DrawButtonPack( Pointers );
		DrawButtonPack( Pointers2 );

		DrawLabel( _( "CHAR POINTERS" ) );
		DrawButtonPack( CharPtr );

		DrawDelimiter();

		DrawLabel( _( "VECTOR / MATRIX" ) );
		DrawButtonPack( Vectors );

		DrawDelimiter();

		DrawLabel( _( "ARRAYS" ) );
		DrawButtonPack( Arrays );
	}

	bool TypePicker::WndProc( const Vector2f CursorPosition, const UINT Message, const WPARAM wParam, const LPARAM lParam )
	{
		bool ReturnValue = false;

		auto IterateButtons = [&] ( const std::span<TypedButton>& Buttons )
		{
			std::ranges::for_each( Buttons, [&] ( auto& Button ) { ReturnValue |= Button.WndProc( CursorPosition, Message, wParam, lParam ); } );
		};

		IterateButtons( Hex );
		IterateButtons( Paddings );
		IterateButtons( UInts );
		IterateButtons( SInts );
		IterateButtons( Floating );
		IterateButtons( Pointers );
		IterateButtons( Pointers2 );
		IterateButtons( CharPtr );
		IterateButtons( Vectors );
		IterateButtons( Arrays );

		return ReturnValue;
	}
}
