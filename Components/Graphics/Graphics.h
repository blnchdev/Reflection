#pragma once
#include "cstdint"
#include <imgui.h>

#include "Libraries/Vector.h"

class Graphics
{
public:
	static constexpr uint32_t RGBA( const uint32_t R, const uint32_t G, const uint32_t B, const uint32_t A )
	{
		return IM_COL32( R, G, B, A );
	}

	static uint32_t LerpRGBA( uint32_t A, uint32_t B, float T );

	static void AddRectangle( Vector2f Position, float Width, float Height, uint32_t Color, float Thickness = 1.f, float Rounding = 0.f, ImDrawFlags Flags = 0 );
	static void AddFilledRectangle( Vector2f Position, float Width, float Height, uint32_t Color, float Rounding = 0.f, ImDrawFlags Flags = 0 );
	static void AddLine( Vector2f PositionA, Vector2f PositionB, uint32_t Color, float Thickness = 1.f );
	static void AddCircle( Vector2f Center, float Radius, uint32_t Color );
	static void AddFilledCircle( Vector2f Center, float Radius, uint32_t Color );
	static void AddImageRounded( void* Texture, Vector2f Position, const Vector2f Size, const uint32_t Color, float RoundingFactor, const ImDrawFlags Flags = ImDrawFlags_RoundCornersAll );
	static void AddImage( void* Texture, Vector2f Position, Vector2f Size, uint32_t Color );
};
