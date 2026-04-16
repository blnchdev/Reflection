#include "Globals.h"

#include "NtDefs.h"
#include "Libraries/Aether/Aether.h"
#include "Libraries/Wraith/Wraith.hpp"

enum class _ReflectionERR : uint8_t
{
	SUCCESS                   = 0x0,
	RENDERER_D3D_INIT_FAILURE = 0x1,
};

class ReflectionError
{
public:
	_ReflectionERR ErrorCode = _ReflectionERR::SUCCESS;

	std::string what() const noexcept
	{
		switch ( ErrorCode )
		{
		case _ReflectionERR::SUCCESS:
			return "No Error";
		case _ReflectionERR::RENDERER_D3D_INIT_FAILURE:
			return "Reflection: Renderer Initialization Error (D3D)";
		}

		return "Unknown Error";
	}
};

enum class _ReflectionStatus : uint8_t
{
	S_OKAY,
	E_NO_HANDLE,
	E_HIJACK_UNSUCCESSFUL,
	E_HANDLE_NO_ACCESS
};

class ReflectionStatus
{
public:
	_ReflectionStatus Code = _ReflectionStatus::S_OKAY;

	const char* what() const noexcept
	{
		using enum _ReflectionStatus;

		switch ( Code )
		{
		case S_OKAY:
			return _( "Success" );
		case E_NO_HANDLE:
			return _( "Failed to obtain a handle to the process; is it protected by the trusted computing base?" );
		case E_HIJACK_UNSUCCESSFUL:
			return _( "No hijackable handle was found. Try using a different method to attach to the target process!" );
		case E_HANDLE_NO_ACCESS:
			return _( "The handle's access mask doesn't match the allowed actions; is the process protected by the trusted computing base?" );
		}

		return _( "Unknown Error" );
	}

	ReflectionStatus() = default;
};

namespace Globals
{
	uint8_t GetDefaultDataStructureSize( DataStructureType DataStructure )
	{
		switch ( DataStructure )
		{
		case T_x8:
		case T_byte:
		case T_int8:
		case T_uint8:
		case T_bool:
			return 1;

		case T_x16:
		case T_word:
		case T_int16:
		case T_uint16:
			return 2;

		case T_x32:
		case T_dword:
		case T_int32:
		case T_uint32:
		case T_float:
			return 4;

		case T_x64:
		case T_qword:
		case T_int64:
		case T_uint64:
		case T_rawPointer:
		case T_double:
		case T_Vector2f:
		case T_pClass:
		case T_EmbeddedClass:
		case T_VMT:
			return 8;

		case T_Vector3f:
			return 12;

		case T_Vector4f:
			return 16;

		case T_char_array:
		case T_wide_array:
			return 24;

		case T_matrix4x4:
			return 64;

		default:
			return 0; // Handle Custom/Dynamic Size somewhere else
		}
	}

	const char* GetDataStructureName( const DataStructureType DataStructure )
	{
		using enum DataStructureType;

		switch ( DataStructure )
		{
		case T_byte:
			return "x8";
		case T_word:
			return "x16";
		case T_dword:
			return "x32";
		case T_qword:
			return "x64";
		case T_int8:
			return "int8_t";
		case T_int16:
			return "int16_t";
		case T_int32:
			return "int32_t";
		case T_int64:
			return "int64_t";

		case T_uint8:
			return "uint8_t";
		case T_uint16:
			return "uint16_t";
		case T_uint32:
			return "uint32_t";
		case T_uint64:
			return "uint64_t";

		case T_float:
			return "float";
		case T_double:
			return "double";

		case T_bool:
			return "bool";
		case T_char_array:
			return "char[]";
		case T_wide_array:
			return "wchar_t[]";

		case T_Vector2f:
			return "Vector2f";
		case T_Vector3f:
			return "Vector3f";
		case T_Vector4f:
			return "Vector4f";

		case T_matrix4x4:
			return "Matrix 4x4";

		case T_rawPointer:
			return "void*";
		case T_pClass:
			return "Class*";
		case T_EmbeddedClass:
			return "Class";
		case T_VMT:
			return "VMT";

		case T_custom:
			return "Custom";

		default:
			return "Unknown";
		}
	}
}
