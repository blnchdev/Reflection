#pragma once
#include <vector>
#include <string>
#include <sstream>
#include "xorstr.hpp"

template <class T>
struct ProjectionInfo
{
	bool IsOnSegment;
	T    SegmentPoint;
	T    LinePoint;
};

#define FORCEINLINE		__forceinline
#define EPSILON			(0.00001f)
#define PI 				(3.1415926535897932f)
#define DEGREEEEE		(PI / 180.f)
#define INV_PI			(0.31830988618f)
#define HALF_PI			(1.57079632679f)

class Vector3f;

enum class RotationDirection : uint8_t
{
	Clockwise,
	CounterClockwise
};

namespace MathUtil
{
	template <class T>
	static FORCEINLINE T Clamp( const T X, const T Min, const T Max ) { return X < Min ? Min : X < Max ? X : Max; }

	template <class T>
	static FORCEINLINE T Square( const T A ) { return A * A; }

	static FORCEINLINE float InvSqrt( const float F ) { return 1.0f / sqrtf( F ); }

	static FORCEINLINE void SinCos( float* ScalarSin, float* ScalarCos, const float Value )
	{
		// Map Value to y in [-pi,pi], x = 2*pi*quotient + remainder.
		float quotient = INV_PI * 0.5f * Value;
		if ( Value >= 0.0f ) { quotient = static_cast<float>( static_cast<int>( lround( quotient + 0.5f ) ) ); }
		else { quotient = static_cast<float>( static_cast<int>( quotient - 0.5f ) ); }
		float y = Value - 2.0f * PI * quotient;

		// Map y to [-pi/2,pi/2] with sin(y) = sin(Value).
		float sign;
		if ( y > HALF_PI )
		{
			y    = PI - y;
			sign = -1.0f;
		}
		else if ( y < -HALF_PI )
		{
			y    = -PI - y;
			sign = -1.0f;
		}
		else { sign = +1.0f; }

		const float y2 = y * y;

		// 11-degree minimax approximation
		*ScalarSin = ( ( ( ( ( -2.3889859e-08f * y2 + 2.7525562e-06f ) * y2 - 0.00019840874f ) * y2 + 0.0083333310f ) *
		                 y2 - 0.16666667f ) * y2 + 1.0f ) * y;

		// 10-degree minimax approximation
		const float p = ( ( ( ( -2.6051615e-07f * y2 + 2.4760495e-05f ) * y2 - 0.0013888378f ) * y2 + 0.041666638f ) *
		                  y2 - 0.5f ) * y2 + 1.0f;
		*ScalarCos = sign * p;
	}

	FORCEINLINE float DegreesToRadians( const float deg ) { return deg * ( PI / 180.f ); }

	FORCEINLINE float RadiansToDegrees( const float rad ) { return rad * ( 180.f / PI ); }

	FORCEINLINE float ScaleFloat( const float Value, const float Min, const float Max, const float NewMin,
	                              const float NewMax )
	{
		return NewMin + ( NewMax - NewMin ) * ( Value - Min ) / ( Max - Min );
	}
}

struct Vector2i
{
	int x, y;

	static const Vector2i Zero;
	static const Vector2i One;
	static const Vector2i Up;
	static const Vector2i Down;
	static const Vector2i Right;
	static const Vector2i Left;
	static const Vector2i PositiveInfinity;
	static const Vector2i NegativeInfinity;

	Vector2i()
	{
		x = 0;
		y = 0;
	}

	Vector2i( const int _x, const int _y )
	{
		x = _x;
		y = _y;
	}
};

inline const Vector2i Vector2i::Zero             = Vector2i( 0, 0 );
inline const Vector2i Vector2i::One              = Vector2i( 1, 1 );
inline const Vector2i Vector2i::Up               = Vector2i( 0, 1 );
inline const Vector2i Vector2i::Down             = Vector2i( 0, -1 );
inline const Vector2i Vector2i::Right            = Vector2i( 1, 0 );
inline const Vector2i Vector2i::Left             = Vector2i( -1, 0 );
inline const Vector2i Vector2i::PositiveInfinity = Vector2i( INFINITY, INFINITY );
inline const Vector2i Vector2i::NegativeInfinity = Vector2i( -INFINITY, -INFINITY );

struct Vector2f
{
	float x, y;

	static const Vector2f Zero;
	static const Vector2f One;
	static const Vector2f Up;
	static const Vector2f Down;
	static const Vector2f Right;
	static const Vector2f Left;
	static const Vector2f PositiveInfinity;
	static const Vector2f NegativeInfinity;

	Vector2f()
	{
		x = 0;
		y = 0;
	}

	Vector2f( const float _x, const float _y )
	{
		x = _x;
		y = _y;
	}

	// Get vector with magnitude of 1
	//
	FORCEINLINE Vector2f Normalized() const;

	FORCEINLINE ProjectionInfo<Vector2f> ProjectOn( const Vector2f& SegmentStart, const Vector2f& SegmentEnd ) const;
	FORCEINLINE Vector2f                 Closest( const std::vector<Vector2f>& v ) const;

	FORCEINLINE std::string ToString() const;
	FORCEINLINE Vector3f    To3D( float y = 0.f ) const;

	FORCEINLINE bool     IsValid() const;
	FORCEINLINE bool     IsZero() const;
	FORCEINLINE bool     IsNonZeroValid() const;
	FORCEINLINE bool     Equals( const Vector2f& b, float Tolerance ) const;
	FORCEINLINE bool     IsInBounds( Vector2f Bounds ) const;
	FORCEINLINE float    DistanceSquared( const Vector2f& b ) const;
	FORCEINLINE float    Distance( const Vector2f& b ) const;
	FORCEINLINE Vector2f Extend( const Vector2f& b, float Length ) const;
	FORCEINLINE Vector2f RotatedAroundPoint( const Vector2f& b, float AngleDeg ) const;
	FORCEINLINE Vector2f Interpolate( const Vector2f& b, float Alpha ) const;

	FORCEINLINE float Dot( const Vector2f& b ) const { return x * b.x + y * b.y; }
	FORCEINLINE float Cross( const Vector2f& b ) const { return this->x * b.y - this->y * b.x; }

	FORCEINLINE Vector2f Rotated( float Angle ) const;

	FORCEINLINE Vector2f Perpendicular() const;

	// Arithmetic operators with Vector
	//
	FORCEINLINE Vector2f operator+( const Vector2f& b ) const { return { this->x + b.x, this->y + b.y }; }
	FORCEINLINE Vector2f operator-( const Vector2f& b ) const { return { this->x - b.x, this->y - b.y }; }
	FORCEINLINE Vector2f operator*( const Vector2f& b ) const { return { this->x * b.x, this->y * b.y }; }
	FORCEINLINE Vector2f operator/( const Vector2f& b ) const { return { this->x / b.x, this->y / b.y }; }

	// op= operators
	//
	FORCEINLINE Vector2f& operator+=( const Vector2f& b )
	{
		x += b.x;
		y += b.y;
		return *this;
	}

	FORCEINLINE Vector2f& operator-=( const Vector2f& b )
	{
		x -= b.x;
		y -= b.y;
		return *this;
	}

	FORCEINLINE Vector2f& operator*=( const Vector2f& b )
	{
		x *= b.x;
		y *= b.y;
		return *this;
	}

	FORCEINLINE Vector2f& operator/=( const Vector2f& b )
	{
		x /= b.x;
		y /= b.y;
		return *this;
	}

	// Arithmetic operators with float
	//
	FORCEINLINE Vector2f operator+( const float b ) const { return { x + b, y + b }; }
	FORCEINLINE Vector2f operator-( const float b ) const { return { x - b, y - b }; }
	FORCEINLINE Vector2f operator*( const float b ) const { return { x * b, y * b }; }
	FORCEINLINE Vector2f operator/( const float b ) const { return { x / b, y / b }; }

	// op= operators with float
	//
	FORCEINLINE Vector2f& operator+=( const float b )
	{
		x += b;
		y += b;
		return *this;
	}

	FORCEINLINE Vector2f& operator-=( const float b )
	{
		x -= b;
		y -= b;
		return *this;
	}

	FORCEINLINE Vector2f& operator*=( const float b )
	{
		x *= b;
		y *= b;
		return *this;
	}

	FORCEINLINE Vector2f& operator/=( const float b )
	{
		x /= b;
		y /= b;
		return *this;
	}

	FORCEINLINE bool operator==( const Vector2f& b ) const
	{
		return std::fabs( x - b.x ) < EPSILON && std::fabs( y - b.y ) < EPSILON;
	}
};

FORCEINLINE Vector2f operator+( const float a, const Vector2f& b ) { return b + a; }
FORCEINLINE Vector2f operator-( const float a, const Vector2f& b ) { return Vector2f( a, a ) - b; }
FORCEINLINE Vector2f operator*( const float a, const Vector2f& b ) { return b * a; }
FORCEINLINE Vector2f operator/( const float a, const Vector2f& b ) { return Vector2f( a, a ) / b; }

FORCEINLINE Vector2f Vector2f::Normalized() const
{
	const float SquareSum = x * x + y * y;
	if ( SquareSum > EPSILON )
	{
		const float Scale = MathUtil::InvSqrt( SquareSum );
		return { x * Scale, y * Scale };
	}
	return PositiveInfinity;
}

FORCEINLINE ProjectionInfo<Vector2f> Vector2f::ProjectOn( const Vector2f& SegmentStart, const Vector2f& SegmentEnd ) const
{
	const auto cx = this->x;
	const auto cy = this->y;
	const auto ax = SegmentStart.x;
	const auto ay = SegmentStart.y;
	const auto bx = SegmentEnd.x;
	const auto by = SegmentEnd.y;

	const auto rL = ( ( cx - ax ) * ( bx - ax ) + ( cy - ay ) * ( by - ay ) )
	                / ( static_cast<float>( pow( bx - ax, 2 ) ) + static_cast<float>( pow( by - ay, 2 ) ) );
	const auto pointLine = Vector2f( ax + rL * ( bx - ax ), ay + rL * ( by - ay ) );
	float      rS;
	if ( rL < 0 ) { rS = 0; }
	else if ( rL > 1 ) { rS = 1; }
	else { rS = rL; }

	const auto isOnSegment  = std::abs( rS - rL ) < EPSILON;
	const auto pointSegment = isOnSegment ? pointLine : Vector2f( ax + rS * ( bx - ax ), ay + rS * ( by - ay ) );
	return ProjectionInfo{ isOnSegment, pointSegment, pointLine };
}

FORCEINLINE Vector2f Vector2f::Rotated( const float Angle ) const
{
	float c, s;
	MathUtil::SinCos( &s, &c, MathUtil::DegreesToRadians( Angle ) );

	return { this->x * c - this->y * s, this->y * c + this->x * s };
}

FORCEINLINE Vector2f Vector2f::Perpendicular() const { return { -this->y, this->x }; }

FORCEINLINE Vector2f Vector2f::Closest( const std::vector<Vector2f>& v ) const
{
	auto result   = Zero;
	auto distance = FLT_MAX;

	for ( auto vec : v )
	{
		const auto tempD = this->Distance( vec );
		if ( distance > tempD )
		{
			distance = tempD;
			result   = vec;
		}
	}

	return result;
}

FORCEINLINE std::string Vector2f::ToString() const
{
	std::ostringstream os;
	os << "[X: ";
	os << x;
	os << ( ", Y:" );
	os << y;
	os << ( "]" );
	return os.str();
}

FORCEINLINE Vector2f Vector2f::Extend( const Vector2f& b, const float Length ) const
{
	return *this + Length * ( b - *this ).Normalized();
}

FORCEINLINE Vector2f Vector2f::RotatedAroundPoint( const Vector2f& b, const float AngleDeg ) const
{
	const float Radians = AngleDeg * PI / 180.0f;
	Vector2f    Copy    = *this;

	Copy.x -= b.x;
	Copy.y -= b.y;

	const float NewX = Copy.x * cos( Radians ) - Copy.y * sin( Radians );
	const float NewY = Copy.x * sin( Radians ) + Copy.y * cos( Radians );

	Copy.x = NewX + b.x;
	Copy.y = NewY + b.y;

	return Copy;
}

FORCEINLINE Vector2f Vector2f::Interpolate( const Vector2f& b, const float Alpha ) const
{
	return { this->x + Alpha * ( b.x - this->x ), this->y + Alpha * ( b.y - this->y ) };
}

FORCEINLINE bool Vector2f::IsValid() const
{
	if ( this->x > -INFINITY && this->x < INFINITY ) { return this->y > -INFINITY && this->y < INFINITY; }

	return true;
}

FORCEINLINE bool Vector2f::IsZero() const { return fabsf( x ) < EPSILON && fabsf( y ) < EPSILON; }

FORCEINLINE bool Vector2f::IsNonZeroValid() const { return IsValid() && !IsZero(); }

FORCEINLINE bool Vector2f::Equals( const Vector2f& b, const float Tolerance ) const
{
	return fabsf( this->x - b.x ) <= Tolerance && fabsf( this->y - b.y ) <= Tolerance;
}

FORCEINLINE bool Vector2f::IsInBounds( const Vector2f Bounds ) const
{
	return this->x >= 0 && this->x <= Bounds.x && this->y >= 0 && this->y <= Bounds.y;
}

FORCEINLINE float Vector2f::DistanceSquared( const Vector2f& b ) const
{
	return MathUtil::Square( b.x - this->x ) + MathUtil::Square( b.y - this->y );
}

FORCEINLINE float Vector2f::Distance( const Vector2f& b ) const { return sqrtf( this->DistanceSquared( b ) ); }

inline const Vector2f Vector2f::Zero             = Vector2f( 0, 0 );
inline const Vector2f Vector2f::One              = Vector2f( 1, 1 );
inline const Vector2f Vector2f::Up               = Vector2f( 0, 1 );
inline const Vector2f Vector2f::Down             = Vector2f( 0, -1 );
inline const Vector2f Vector2f::Right            = Vector2f( 1, 0 );
inline const Vector2f Vector2f::Left             = Vector2f( -1, 0 );
inline const Vector2f Vector2f::PositiveInfinity = Vector2f( INFINITY, INFINITY );
inline const Vector2f Vector2f::NegativeInfinity = Vector2f( -INFINITY, -INFINITY );

class Vector3f
{
public:
	static const Vector3f Max;
	static const Vector3f Zero;
	static const Vector3f One;
	static const Vector3f Up;
	static const Vector3f Down;
	static const Vector3f Forward;
	static const Vector3f Back;
	static const Vector3f Right;
	static const Vector3f Left;
	static const Vector3f PositiveInfinity;
	static const Vector3f NegativeInfinity;

	FORCEINLINE Vector3f()
	{
		this->x = 0;
		this->y = 0;
		this->z = 0;
	}

	FORCEINLINE Vector3f( const float x, const float y, const float z )
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	FORCEINLINE Vector3f( const float x, const float y )
	{
		this->x = x;
		this->y = 0;
		this->z = y;
	}

	// Get the length of the vector
	//
	[[nodiscard]] float Magnitude() const;
	[[nodiscard]] float Length() const;
	[[nodiscard]] float Length2D() const;

	// Squared length of the vector
	//
	[[nodiscard]] float LengthSquared() const;
	[[nodiscard]] float LengthSquared2D() const;
	[[nodiscard]] float MagnitudeSquared() const;

	// Get the dot product
	//
	[[nodiscard]] float Dot( const Vector3f& b ) const;

	// Get the cross product
	//
	[[nodiscard]] Vector3f Cross( const Vector3f& b ) const;
	FORCEINLINE float      Cross2D( const Vector3f& b ) const { return ( this->x * b.z ) - ( this->z * b.x ); }

	// Get vector with magnitude of 1
	//
	[[nodiscard]] Vector3f Normalized() const;

	// Check equality within given tolerance
	//
	[[nodiscard]] bool Equals( const Vector3f& b, float Tolerance ) const;

	// Get string representation of vector
	//
	[[nodiscard]] std::string ToString() const;

	// Euclidian distance
	//
	[[nodiscard]] float Distance( const Vector3f& b ) const;

	// Euclidian distance ignoring Y
	//
	[[nodiscard]] float DistanceXY( const Vector3f& b ) const;

	// Euclidian distance ignoring Z
	//
	[[nodiscard]] float Distance2D( const Vector3f& b ) const;

	// Squared distance
	//
	[[nodiscard]] float DistanceSquared( const Vector3f& b ) const;

	// Squared distance ignoring Y
	//
	[[nodiscard]] float DistanceSquaredXY( const Vector3f& b ) const;

	// Squared distance ignoring Z
	//
	[[nodiscard]] float DistanceSquared2D( const Vector3f& b ) const;

	// Gets the angle in degrees
	//
	[[nodiscard]] float Angle( const Vector3f& b ) const;

	// Rotates by an angle in degrees
	//
	[[nodiscard]] Vector3f Rotated( float Angle ) const;

	// Rotates around a point by an angle in degrees
	//
	[[nodiscard]] Vector3f RotatedAroundPoint( const Vector3f& b, float Angle ) const;

	// Rotates around unit length axis
	//
	[[nodiscard]] Vector3f RotateAngleAxis( float Degrees, const Vector3f& Axis ) const;

	// Creates a copy with the length clamped between min and max
	//
	[[nodiscard]] Vector3f ClampLength( float Min, float Max ) const;

	// Creates a copy of the perpendicular vector
	//
	[[nodiscard]] Vector3f Perpendicular1() const;

	// Creates a copy of the perpendicular vector
	//
	[[nodiscard]] Vector3f Perpendicular2() const;

	// Checks if all axes are valid
	//
	[[nodiscard]] bool IsValid() const;

	// Checks if all axes are 0
	//
	[[nodiscard]] bool IsZero() const;

	// Checks if all axes are non 0 and valid
	//
	[[nodiscard]] bool IsNonZeroValid() const;

	// Checks if the vector is a unit vector, within tolerance
	//
	[[nodiscard]] bool IsUnit( float Tolerance = EPSILON ) const;

	// Checks if the axes are equal, within tolerance
	//
	[[nodiscard]] bool IsUniform( float Tolerance = EPSILON ) const;

	// Creates a copy with the length extended to the given value towards the given vector
	// 
	[[nodiscard]] Vector3f Extend( const Vector3f& b, float Length ) const;

	// Linearaly interpolate between vectors
	//
	[[nodiscard]] Vector3f Lerp( const Vector3f& b, float Alpha ) const;

	// Projects copy of the vector onto a given input vector
	//
	[[nodiscard]] Vector3f ProjectOnTo( const Vector3f& b ) const;

	// Projects copy of the vector onto a given input vector with unit length
	//
	[[nodiscard]] Vector3f ProjectOnToNormal( const Vector3f& Normal ) const;

	// Find closest point online segment
	//
	[[nodiscard]] Vector3f ClosestPointOnLineSegment( const Vector3f& Start, const Vector3f& End ) const;

	// Gets a copy of the vector rotated 90 degrees
	[[nodiscard]] Vector3f Rotate90( RotationDirection Direction ) const;

	// Gets a copy of the vector rotated
	[[nodiscard]] Vector3f RotatedAngle( float angle ) const;

	// Checks for intersection between two segments
	//
	bool SegmentIntersection2D( const Vector3f& SegmentEndA, const Vector3f& SegmentStartB, const Vector3f& SegmentEndB,
	                            _Out_ Vector3f& IntersectionPoint ) const;

	// Generate B�zier curve defined by 4 controlpoints
	//
	static float GenerateBezier( const Vector3f* ControlPoints, int NumPoints, _Out_ std::vector<Vector3f>& OutPoints );

	// Creates a copy of the vector in 2D space
	//
	[[nodiscard]] Vector2f To2D() const;

	// Creates a copy of the vector in 3D space
	//
	[[nodiscard]] Vector3f To3D( float y ) const;

	[[nodiscard]] ProjectionInfo<Vector2f> ProjectOn( const Vector3f& SegmentStart, const Vector3f& SegmentEnd ) const;

	[[nodiscard]] float AngleBetween( const Vector3f& a, const Vector3f& b ) const;
	[[nodiscard]] float AngleBetween( const Vector3f& a, const Vector3f& b, bool Full ) const;

	[[nodiscard]] float Polar() const;

	[[nodiscard]] std::vector<Vector3f> CreateCircle( float Radius, float SegmentCount = 30.f ) const;

	// Arithmetic operators with Vector
	//
	FORCEINLINE Vector3f operator+( const Vector3f& b ) const { return { this->x + b.x, this->y + b.y, this->z + b.z }; }

	FORCEINLINE Vector3f operator-( const Vector3f& b ) const { return { this->x - b.x, this->y - b.y, this->z - b.z }; }

	FORCEINLINE Vector3f operator*( const Vector3f& b ) const { return { this->x * b.x, this->y * b.y, this->z * b.z }; }

	FORCEINLINE Vector3f operator/( const Vector3f& b ) const { return { this->x / b.x, this->y / b.y, this->z / b.z }; }

	FORCEINLINE float operator|( const Vector3f& b ) const { return x * b.x + y * b.y + z * b.z; }

	FORCEINLINE Vector3f operator^( const Vector3f& b ) const
	{
		return {
			y * b.z - z * b.y,
			z * b.x - x * b.z,
			x * b.y - y * b.x
		};
	}

	// op= operators
	//
	FORCEINLINE Vector3f& operator+=( const Vector3f& b )
	{
		x += b.x;
		y += b.y;
		z += b.z;
		return *this;
	}

	FORCEINLINE Vector3f& operator-=( const Vector3f& b )
	{
		x -= b.x;
		y -= b.y;
		z -= b.z;
		return *this;
	}

	FORCEINLINE Vector3f& operator*=( const Vector3f& b )
	{
		x *= b.x;
		y *= b.y;
		z *= b.z;
		return *this;
	}

	FORCEINLINE Vector3f& operator/=( const Vector3f& b )
	{
		x /= b.x;
		y /= b.y;
		z /= b.z;
		return *this;
	}

	// Arithmetic operators with float
	//
	FORCEINLINE Vector3f operator+( const float b ) const { return { x + b, y + b, z + b }; }
	FORCEINLINE Vector3f operator-( const float b ) const { return { x - b, y - b, z - b }; }
	FORCEINLINE Vector3f operator*( const float b ) const { return { x * b, y * b, z * b }; }
	FORCEINLINE Vector3f operator/( const float b ) const { return { x / b, y / b, z / b }; }

	// op= operators with float
	//
	FORCEINLINE Vector3f& operator+=( const float b )
	{
		x += b;
		y += b;
		z += b;
		return *this;
	}

	FORCEINLINE Vector3f& operator-=( const float b )
	{
		x -= b;
		y -= b;
		z -= b;
		return *this;
	}

	FORCEINLINE Vector3f& operator*=( const float b )
	{
		x *= b;
		y *= b;
		z *= b;
		return *this;
	}

	FORCEINLINE Vector3f& operator/=( const float b )
	{
		x /= b;
		y /= b;
		z /= b;
		return *this;
	}

	FORCEINLINE bool operator==( const Vector3f& b ) const { return x == b.x && y == b.y && z == b.z; }

	float x, y, z;
};

FORCEINLINE Vector3f operator+( const float a, const Vector3f& b ) { return b + a; }
FORCEINLINE Vector3f operator-( const float a, const Vector3f& b ) { return Vector3f( a, a, a ) - b; }
FORCEINLINE Vector3f operator*( const float a, const Vector3f& b ) { return b * a; }
FORCEINLINE Vector3f operator/( const float a, const Vector3f& b ) { return Vector3f( a, a, a ) / b; }

FORCEINLINE float Vector3f::Magnitude() const { return sqrtf( x * x + y * y + z * z ); }

FORCEINLINE float Vector3f::Length() const { return sqrtf( x * x + y * y + z * z ); }

FORCEINLINE float Vector3f::Length2D() const { return sqrtf( x * x + z * z ); }

FORCEINLINE float Vector3f::LengthSquared() const { return x * x + y * y + z * z; }

FORCEINLINE float Vector3f::LengthSquared2D() const { return x * x + z * z; }

FORCEINLINE float Vector3f::MagnitudeSquared() const { return x * x + y * y + z * z; }

FORCEINLINE float Vector3f::Dot( const Vector3f& b ) const { return *this | b; }

FORCEINLINE Vector3f Vector3f::Cross( const Vector3f& b ) const { return *this ^ b; }

FORCEINLINE Vector3f Vector3f::Normalized() const
{
	const float SquareSum = x * x + y * y + z * z;

	if ( SquareSum > EPSILON )
	{
		const float Scale = MathUtil::InvSqrt( SquareSum );
		return { x * Scale, y * Scale, z * Scale };
	}
	return PositiveInfinity;
}

FORCEINLINE bool Vector3f::Equals( const Vector3f& b, const float Tolerance ) const
{
	return fabsf( this->x - b.x ) <= Tolerance && fabsf( this->y - b.y ) <= Tolerance && fabsf( this->z - b.z ) <= Tolerance;
}

FORCEINLINE std::string Vector3f::ToString() const
{
	std::ostringstream os;
	os << xorstr_( "[ " );
	os << x;
	os << xorstr_( ", " );
	os << y;
	os << xorstr_( ", " );
	os << z;
	os << xorstr_( " ]" );
	return os.str();
}

FORCEINLINE float Vector3f::Distance( const Vector3f& b ) const { return sqrtf( this->DistanceSquared( b ) ); }

FORCEINLINE float Vector3f::DistanceXY( const Vector3f& b ) const { return sqrtf( this->DistanceSquaredXY( b ) ); }

FORCEINLINE float Vector3f::Distance2D( const Vector3f& b ) const { return sqrtf( this->DistanceSquared2D( b ) ); }

FORCEINLINE float Vector3f::DistanceSquared( const Vector3f& b ) const
{
	return MathUtil::Square( b.x - this->x ) + MathUtil::Square( b.y - this->y ) + MathUtil::Square( b.z - this->z );
}

FORCEINLINE float Vector3f::DistanceSquaredXY( const Vector3f& b ) const
{
	return MathUtil::Square( b.x - this->x ) + MathUtil::Square( b.y - this->y );
}

FORCEINLINE float Vector3f::DistanceSquared2D( const Vector3f& b ) const
{
	return MathUtil::Square( b.x - this->x ) + MathUtil::Square( b.z - this->z );
}

FORCEINLINE float Vector3f::Angle( const Vector3f& b ) const
{
	const float angleCos = this->Dot( b ) / ( this->Length() * b.Length() );
	const float angleRad = acosf( ( angleCos < -1.f ) ? -1.f : ( ( angleCos < 1.f ) ? angleCos : 1.f ) );
	return MathUtil::RadiansToDegrees( angleRad );
}

inline Vector3f Vector3f::Rotated( const float Angle ) const
{
	float S, C;
	MathUtil::SinCos( &S, &C, MathUtil::DegreesToRadians( Angle ) );

	return {
		this->x * C - this->z * S,
		this->y,
		this->z * C + this->x * S
	};
}

inline Vector3f Vector3f::RotatedAroundPoint( const Vector3f& b, const float Angle ) const
{
	return ( *this - b ).Rotated( Angle ) + b;
}

inline Vector3f Vector3f::RotateAngleAxis( const float Degrees, const Vector3f& Axis ) const
{
	float S, C;
	MathUtil::SinCos( &S, &C, MathUtil::DegreesToRadians( Degrees ) );

	const float XX = Axis.x * Axis.x;
	const float YY = Axis.y * Axis.y;
	const float ZZ = Axis.z * Axis.z;

	const float XY = Axis.x * Axis.x;
	const float YZ = Axis.y * Axis.y;
	const float ZX = Axis.z * Axis.z;

	const float XS = Axis.x * S;
	const float YS = Axis.y * S;
	const float ZS = Axis.z * S;

	const float OMC = 1.f - C;

	return {
		( OMC * XX + C ) * x + ( OMC * XY - ZS ) * y + ( OMC * ZX + YS ) * z,
		( OMC * XY + ZS ) * x + ( OMC * YY + C ) * y + ( OMC * YZ - XS ) * z,
		( OMC * ZX - YS ) * x + ( OMC * YZ + XS ) * y + ( OMC * ZZ + C ) * z
	};
}

FORCEINLINE Vector3f Vector3f::ClampLength( const float Min, const float Max ) const
{
	const float    Length    = this->Length();
	const Vector3f Direction = ( Length > EPSILON ) ? ( *this / Length ) : Zero;

	const auto Size = Length < Min ? Min : Length < Max ? Length : Max;

	return Direction * Size;
}

inline Vector3f Vector3f::Perpendicular1() const { return { this->z, this->y, -this->x }; }

inline Vector3f Vector3f::Perpendicular2() const { return { -this->z, this->y, this->x }; }

FORCEINLINE bool Vector3f::IsValid() const
{
	if ( this->x > -INFINITY && this->x < INFINITY )
	{
		if ( this->y > -INFINITY && this->y < INFINITY ) { return this->z > -INFINITY && this->z < INFINITY; }
	}

	return true;
}

FORCEINLINE bool Vector3f::IsZero() const { return fabsf( x ) < EPSILON && fabsf( y ) < EPSILON && fabsf( z ) < EPSILON; }

FORCEINLINE bool Vector3f::IsNonZeroValid() const { return IsValid() && !IsZero(); }

FORCEINLINE bool Vector3f::IsUnit( const float Tolerance ) const { return fabsf( 1.0f - Length() ) < Tolerance; }

FORCEINLINE bool Vector3f::IsUniform( const float Tolerance ) const
{
	return fabsf( x - y ) <= Tolerance && fabsf( x - z ) <= Tolerance && fabsf( y - z ) <= Tolerance;
}

FORCEINLINE Vector3f Vector3f::Extend( const Vector3f& b, const float Length ) const
{
	return *this + Length * ( b - *this ).Normalized();
}

FORCEINLINE Vector3f Vector3f::Lerp( const Vector3f& b, const float Alpha ) const
{
	return *this + Alpha * ( b - *this );
}

FORCEINLINE Vector3f Vector3f::ProjectOnTo( const Vector3f& b ) const { return b * ( ( *this | b ) / ( b | b ) ); }

FORCEINLINE Vector3f Vector3f::ProjectOnToNormal( const Vector3f& Normal ) const
{
	return ( Normal * ( *this | Normal ) );
}

inline Vector3f Vector3f::ClosestPointOnLineSegment( const Vector3f& Start, const Vector3f& End ) const
{
	// Solve to find alpha along line that is the closest point
	// Weisstein, Eric W. "Point-Line Distance--3-Dimensional." From MathWorld--A Switchram Web Resource. http://mathworld.wolfram.com/Point-LineDistance3-Dimensional.html 
	const float A = ( Start - *this ) | ( End - Start );
	const float B = ( End - Start ).LengthSquared();
	// This should be robust to B == 0 (resulting in NaN) because clamp should return 1.
	const float T = MathUtil::Clamp( -A / B, 0.f, 1.f );

	// Generate closest point
	const Vector3f ClosestPoint = Start + ( T * ( End - Start ) );

	return ClosestPoint;
}

FORCEINLINE Vector3f Vector3f::Rotate90( const RotationDirection Direction ) const
{
	if ( Direction == RotationDirection::Clockwise ) { return { this->z, this->y, -this->x }; }

	return { -this->z, this->y, this->x };
}

inline Vector3f Vector3f::RotatedAngle( const float angle ) const
{
	float Sin, Cos;
	MathUtil::SinCos( &Sin, &Cos, angle );
	return Vector3f{ this->x * Cos + this->z * Sin, this->y, this->z * Cos - this->x * Sin };
}

FORCEINLINE bool Vector3f::SegmentIntersection2D( const Vector3f& SegmentEndA, const Vector3f& SegmentStartB,
                                                  const Vector3f& SegmentEndB, Vector3f&       IntersectionPoint ) const
{
	IntersectionPoint = Zero;

	const Vector3f VectorA = SegmentEndA - *this;
	const Vector3f VectorB = SegmentEndB - SegmentStartB;

	const float S = ( -VectorA.z * ( this->x - SegmentStartB.x ) + VectorA.x * ( this->z - SegmentStartB.z ) ) / ( -
	                                                                                                               VectorB.x * VectorA.z + VectorA.x * VectorB.z );
	const float T = ( VectorB.x * ( this->z - SegmentStartB.z ) - VectorB.z * ( this->x - SegmentStartB.x ) ) / ( -
	                                                                                                              VectorB.x * VectorA.z + VectorA.x * VectorB.z );

	const bool bIntersects = ( S >= 0 && S <= 1 && T >= 0 && T <= 1 );

	if ( bIntersects )
	{
		IntersectionPoint.x = this->x + ( T * VectorA.x );
		IntersectionPoint.y = 0;
		IntersectionPoint.z = this->z + ( T * VectorA.z );
	}

	return bIntersects;
}

FORCEINLINE float Vector3f::GenerateBezier( const Vector3f*        ControlPoints, const int NumPoints,
                                            std::vector<Vector3f>& OutPoints )
{
	if ( !ControlPoints ) { return 0.f; }

	if ( NumPoints < 2 ) { return 0.f; }

	// q is the change in t between successive evaluations.
	const float q = 1.f / ( NumPoints - 1 ); // q is dependent on the number of GAPS = POINTS-1

	// recreate the names used in the derivation
	const Vector3f& P0 = ControlPoints[ 0 ];
	const Vector3f& P1 = ControlPoints[ 1 ];
	const Vector3f& P2 = ControlPoints[ 2 ];
	const Vector3f& P3 = ControlPoints[ 3 ];

	// coefficients of the cubic polynomial that we're FDing -
	const Vector3f a = P0;
	const Vector3f b = 3 * ( P1 - P0 );
	const Vector3f c = 3 * ( P2 - 2 * P1 + P0 );
	const Vector3f d = P3 - 3 * P2 + 3 * P1 - P0;

	// initial values of the poly and the 3 diffs -
	Vector3f       S = a;                                 // the poly value
	Vector3f       U = b * q + c * q * q + d * q * q * q; // 1st order diff (quadratic)
	Vector3f       V = 2 * c * q * q + 6 * d * q * q * q; // 2nd order diff (linear)
	const Vector3f W = 6 * d * q * q * q;                 // 3rd order diff (constant)

	// Path length.
	float Length = 0.f;

	Vector3f OldPos = P0;
	OutPoints.push_back( P0 ); // first point on the curve is always P0.

	for ( int i = 1; i < NumPoints; ++i )
	{
		// calculate the next value and update the deltas
		S += U; // update poly value
		U += V; // update 1st order diff value
		V += W; // update 2st order diff value
		// 3rd order diff is constant => no update needed.

		// Update Length.
		Length += S.Distance( OldPos );
		OldPos = S;

		OutPoints.push_back( S );
	}

	// Return path length as experienced in sequence (linear interpolation between points).
	return Length;
}

FORCEINLINE Vector2f Vector3f::To2D() const { return { x, z }; }

FORCEINLINE Vector3f Vector3f::To3D( float y ) const { return { x, y, this->y }; }

inline ProjectionInfo<Vector2f> Vector3f::ProjectOn( const Vector3f& SegmentStart, const Vector3f& SegmentEnd ) const
{
	return this->To2D().ProjectOn( SegmentStart.To2D(), SegmentEnd.To2D() );
}

inline float Vector3f::AngleBetween( const Vector3f& a, const Vector3f& b ) const
{
	return this->AngleBetween( a, b, false );
}

inline float Vector3f::AngleBetween( const Vector3f& a, const Vector3f& b, bool Full ) const
{
	const auto Negative = *this * -1;
	const auto p1       = Negative + a;
	const auto p2       = Negative + b;

	auto theta = p1.Polar() - p2.Polar();

	if ( theta < 0 ) { theta += 360; }

	if ( theta > 180 ) { theta = 360 - theta; }

	return theta;
}

inline float Vector3f::Polar() const
{
	if ( std::fabsf( this->x - 0 ) <= EPSILON )
	{
		if ( this->z > 0 ) { return 90; }
		if ( this->z < 0 ) { return 270; }

		return 0;
	}

	auto theta = MathUtil::RadiansToDegrees( std::atanf( this->z / this->x ) );

	if ( this->x < 0 ) { theta += 180; }

	if ( theta < 0 ) { theta += 360; }

	return theta;
}

inline std::vector<Vector3f> Vector3f::CreateCircle( const float Radius, const float SegmentCount ) const
{
	std::vector<Vector3f> Result = {};

	for ( auto i = 1.f; i <= SegmentCount; i += 1.f )
	{
		const auto angle = i * 2.f * PI / SegmentCount;
		auto       point = Vector3f{ this->x + Radius * std::cosf( angle ), this->y, this->z + Radius * std::sinf( angle ) };
		Result.push_back( point );
	}

	return Result;
}

FORCEINLINE Vector3f Vector2f::To3D( float y ) const { return { x, y, this->y }; }

inline const Vector3f Vector3f::Max              = Vector3f( FLT_MAX, FLT_MAX, FLT_MAX );
inline const Vector3f Vector3f::Zero             = Vector3f( 0, 0, 0 );
inline const Vector3f Vector3f::One              = Vector3f( 1, 1, 1 );
inline const Vector3f Vector3f::Up               = Vector3f( 0, 1, 0 );
inline const Vector3f Vector3f::Down             = Vector3f( 0, -1, 0 );
inline const Vector3f Vector3f::Forward          = Vector3f( 0, 0, 1 );
inline const Vector3f Vector3f::Back             = Vector3f( 0, 0, -1 );
inline const Vector3f Vector3f::Right            = Vector3f( 1, 0, 0 );
inline const Vector3f Vector3f::Left             = Vector3f( -1, 0, 0 );
inline const Vector3f Vector3f::PositiveInfinity = Vector3f( INFINITY, INFINITY, INFINITY );
inline const Vector3f Vector3f::NegativeInfinity = Vector3f( -INFINITY, -INFINITY, -INFINITY );

struct Vector4f
{
	float x, y, z, w;
};
