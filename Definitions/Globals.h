#pragma once
#include <cstdint>
#include <string>
#include <Windows.h>
#include <Libraries/xorstr.hpp>

enum class _ReflectionERR : uint8_t;
class ReflectionError;

enum class _ReflectionStatus : uint8_t;
class ReflectionStatus;

enum DataStructureType : uint8_t
{
	T_x8,
	T_x16,
	T_x32,
	T_x64,

	T_byte,
	T_word,
	T_dword,
	T_qword,

	T_int8,
	T_int16,
	T_int32,
	T_int64,

	T_uint8,
	T_uint16,
	T_uint32,
	T_uint64,

	T_float,
	T_double,

	T_bool,
	T_char_array,
	T_wide_array,

	T_charPtr_array,
	T_widePtr_array,

	T_Vector2f,
	T_Vector3f,
	T_Vector4f,

	T_matrix4x4,

	T_rawPointer,
	T_pClass,
	T_EmbeddedClass,
	T_VMT,
	T_pFunction,

	T_custom,
	T_undefined
};

#define _(arg) xorstr_(arg)

namespace Globals
{
	uint8_t     GetDefaultDataStructureSize( DataStructureType DataStructure );
	const char* GetDataStructureName( DataStructureType DataStructure );
}
