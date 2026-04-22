#include "MemoryStructs.h"

namespace Memory
{
	void Info::RefreshMaxWidth()
	{
		float szLabel = 0.f;
		float szType  = 0.f;

		szLabel = Renderer::FontManager::FontSize( _( "Pad_000" ), 16.f, false ).x;

		for ( const auto& Field : Fields )
		{
			szLabel = max( szLabel, Renderer::FontManager::FontSize(Field.Name, 16.f, false).x );

			const bool ContainsData = std::holds_alternative<std::shared_ptr<Info>>( Field.Embedded );

			if ( ContainsData )
			{
				auto TemporaryLabel = "<" + std::get<std::shared_ptr<Info>>( Field.Embedded )->Label + ">"; // This is pretty expensive but shoooould be okay?
				szType              = max( szType, Renderer::FontManager::FontSize(TemporaryLabel, 16.f, false).x );
			}

			szType = max( szType, Renderer::FontManager::FontSize(Globals::GetDataStructureName(Field.Type), 16.f, false).x );
		}

		this->RenderMetrics.LabelWidth = szLabel;
		this->RenderMetrics.TypeWidth  = szType;
	}
}
