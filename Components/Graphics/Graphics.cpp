#include "Graphics.h"

#include <algorithm>
#include <imgui.h>

uint32_t Graphics::LerpRGBA( const uint32_t A, const uint32_t B, float T )
{
	T = std::clamp( T, 0.0f, 1.0f );

	const uint8_t aA = ( A >> 24 ) & 0xFF;
	const uint8_t rA = ( A >> 16 ) & 0xFF;
	const uint8_t gA = ( A >> 8 ) & 0xFF;
	const uint8_t bA = ( A >> 0 ) & 0xFF;

	const uint8_t aB = ( B >> 24 ) & 0xFF;
	const uint8_t rB = ( B >> 16 ) & 0xFF;
	const uint8_t gB = ( B >> 8 ) & 0xFF;
	const uint8_t bB = ( B >> 0 ) & 0xFF;

	auto LerpChannel = [T] ( const uint8_t C0, const uint8_t C1 )
	{
		return static_cast<uint8_t>(
			C0 + ( C1 - C0 ) * T
		);
	};

	const uint8_t a = LerpChannel( aA, aB );
	const uint8_t r = LerpChannel( rA, rB );
	const uint8_t g = LerpChannel( gA, gB );
	const uint8_t b = LerpChannel( bA, bB );

	return ( static_cast<uint32_t>( a ) << 24 ) |
	       ( static_cast<uint32_t>( r ) << 16 ) |
	       ( static_cast<uint32_t>( g ) << 8 ) |
	       ( static_cast<uint32_t>( b ) << 0 );
}

void Graphics::AddRectangle( Vector2f Position, const float Width, const float Height, const uint32_t Color, const float Thickness, const float Rounding, const ImDrawFlags Flags )
{
	ImDrawList*  DrawList = ImGui::GetBackgroundDrawList();
	const ImVec2 Start    = *reinterpret_cast<ImVec2*>( &Position );
	const ImVec2 End      = { Position.x + Width, Position.y + Height };

	DrawList->AddRect( Start, End, Color, Rounding, Flags, Thickness );
}

void Graphics::AddFilledRectangle( Vector2f Position, const float Width, const float Height, const uint32_t Color, const float Rounding, const ImDrawFlags Flags )
{
	ImDrawList*  DrawList = ImGui::GetBackgroundDrawList();
	const ImVec2 Start    = *reinterpret_cast<ImVec2*>( &Position );
	const ImVec2 End      = { Position.x + Width, Position.y + Height };

	DrawList->AddRectFilled( Start, End, Color, Rounding, Flags );
}

void Graphics::AddLine( Vector2f PositionA, Vector2f PositionB, const uint32_t Color, const float Thickness )
{
	ImDrawList* DrawList = ImGui::GetBackgroundDrawList();

	DrawList->AddLine( *reinterpret_cast<ImVec2*>( &PositionA ), *reinterpret_cast<ImVec2*>( &PositionB ), Color, Thickness );
}

void Graphics::AddCircle( Vector2f Center, const float Radius, const uint32_t Color )
{
	ImDrawList* DrawList = ImGui::GetBackgroundDrawList();

	DrawList->AddCircle( *reinterpret_cast<ImVec2*>( &Center ), Radius, Color, 100 );
}

void Graphics::AddFilledCircle( Vector2f Center, const float Radius, const uint32_t Color )
{
	ImDrawList* DrawList = ImGui::GetBackgroundDrawList();

	DrawList->AddCircleFilled( *reinterpret_cast<ImVec2*>( &Center ), Radius, Color, 100 );
}

void Graphics::AddImageRounded( void* Texture, Vector2f Position, const Vector2f Size, const uint32_t Color, const float RoundingFactor, const ImDrawFlags Flags )
{
	if ( !Texture )
	{
		return;
	}

	ImDrawList* DrawList = ImGui::GetBackgroundDrawList();

	DrawList->PushTextureID( Texture );
	DrawList->AddImageRounded( Texture, *reinterpret_cast<ImVec2*>( &Position ), ImVec2( Position.x + Size.x, Position.y + Size.y ), ImVec2( 0, 0 ), ImVec2( 1, 1 ), Color, RoundingFactor, Flags );
	DrawList->PopTextureID();
}

void Graphics::AddImage( void* Texture, Vector2f Position, const Vector2f Size, const uint32_t Color )
{
	if ( !Texture )
	{
		return;
	}

	ImDrawList* DrawList = ImGui::GetBackgroundDrawList();

	DrawList->PushTextureID( Texture );
	DrawList->AddImage( Texture, *reinterpret_cast<ImVec2*>( &Position ), ImVec2( Position.x + Size.x, Position.y + Size.y ), ImVec2( 0, 0 ), ImVec2( 1, 1 ), Color );
	DrawList->PopTextureID();
}
