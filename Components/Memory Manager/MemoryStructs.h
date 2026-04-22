#pragma once
#include <format>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "Components/FontManager/FontManager.h"
#include "Components/Layout/Objects/Button/Button.h"
#include "Definitions/Globals.h"
#include "Libraries/nlohmann/json.hpp"

namespace Renderer::Layout
{
	class View;
}

namespace Memory
{
	class Info;

	struct Field
	{
		std::string       Name;
		size_t            Offset  = 0;
		size_t            Size    = 0;
		bool              IsNamed = false;
		DataStructureType Type    = T_x64;

		std::variant<std::shared_ptr<Info>, std::string> Embedded = _( "RESERVED_NONE" );

		bool IsPadding() const { return Type == T_x8 || Type == T_x16 || Type == T_x32 || Type == T_x64; }
		bool IsPointer() const { return Type == T_pClass || Type == T_pFunction || Type == T_rawPointer || Type == T_VMT || Type == T_charPtr_array || Type == T_widePtr_array; }
		bool IsPOD() const { return !IsPointer() && Type != T_EmbeddedClass && Type != T_custom; }

		Field()  = default;
		~Field() = default;

		Field( const Field& Other ) : Name( Other.Name ), Offset( Other.Offset ), Size( Other.Size ), IsNamed( Other.IsNamed ), Type( Other.Type ), Embedded( Other.Embedded )
		{
		}

		Field& operator=( const Field& Other )
		{
			if ( this == &Other ) return *this;

			Name     = Other.Name;
			Offset   = Other.Offset;
			Size     = Other.Size;
			IsNamed  = Other.IsNamed;
			Type     = Other.Type;
			Embedded = Other.Embedded;

			return *this;
		}

		Field( Field&& ) noexcept            = default;
		Field& operator=( Field&& ) noexcept = default;

		Field( const DataStructureType FieldType, const size_t Offset, const std::string& Label = {} ) : Offset( Offset ), Type( FieldType )
		{
			Name    = Label.empty() ? std::format( "Off_{:X}", Offset ) : Label;
			IsNamed = !Label.empty();
			Size    = Globals::GetDefaultDataStructureSize( Type );
		}

		Field( const size_t Size, const size_t Offset ) : Offset( Offset ), Size( Size )
		{
			IsNamed = false;

			switch ( Size )
			{
			case 1:
				Type = T_x8;
				break;
			case 2:
				Type = T_x16;
				break;
			case 4:
				Type = T_x32;
				break;
			case 8:
				Type = T_x64;
				break;
			default:
				break;
			}

			Name = std::format( "Pad_{:X}", Offset );
		}
	};

	class Info
	{
	public:
		std::string        Label;
		size_t             Size = 0;
		uint64_t           ID   = 0;
		std::vector<Field> Fields{};

		struct
		{
			float LabelWidth = 0.f;
			float TypeWidth  = 0.f;
		} RenderMetrics;

		Info( const Info& )            = default;
		Info& operator=( const Info& ) = default;
		Info( Info&& )                 = default;
		Info& operator=( Info&& )      = default;
		~Info()                        = default;

		Info( std::string Name, const std::vector<Field>& Fields ) : Label( std::move( Name ) ), Fields( Fields )
		{
			if ( !Fields.empty() )
			{
				auto& LastField = Fields.back();
				Size            = LastField.Offset + LastField.Size;
			}
		}

		void RefreshMaxWidth();
	};
}
