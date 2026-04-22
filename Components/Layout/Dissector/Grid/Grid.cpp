#include "Grid.h"

#include <algorithm>
#include <ranges>
#include <unordered_set>
#include <utility>
#include <variant>
#include <Zydis/Zydis.h>

#include "Components/Class Manager/ClassManager.h"
#include "Components/Config/Config.h"
#include "Components/Graphics/Graphics.h"
#include "Components/Layout/Dissector/Dissector.h"
#include "Components/Layout/Objects/Line/Line.h"
#include "Components/Memory Manager/Memory.h"
#include "Components/Parser/Parser.h"
#include "Components/Title Bar/TitleBar.h"

namespace Renderer::Layout
{
	constexpr float SCROLL_SPEED     = 60.f;
	constexpr float SCROLL_DECAY     = 12.f;
	constexpr float SCROLL_SNAP_DIST = 0.5f;

	// Helpers & Data
	namespace
	{
		// Config Members
		int64_t ReadInterval   = 1'000;
		int64_t DefaultTabSize = 64;
		int64_t MaxDepth       = 4;

		struct ClassInstance;

		struct
		{
			Vector2f Origin = Vector2f::Zero;
			Vector2f Size   = Vector2f::Zero;
		} GridData;

		uint32_t GlobalIDX           = 0;
		uint32_t SelectCursorViewIDX = 0;

		uint32_t                                    ActiveTabIDX = 0;
		std::vector<std::unique_ptr<ClassInstance>> Tabs         = {};

		namespace Helpers
		{
			std::string FormatPointer( Process::PointerData* Data, uintptr_t VA )
			{
				if ( !Data->Valid ) return _( "→ ???" );
				if ( Data->InModule ) return std::format( "\xF3\xB0\xBD\x9E {}+0x{:X} (0x{:X})", Data->ModuleName, Data->Offset, VA );
				return std::format( "\xF3\xB0\x91\x83 0x{:X}", VA );
			}

			std::string HandleCharArray( const void* Buffer )
			{
				auto       Pointer = static_cast<const char*>( Buffer );
				const auto MaxLen  = strnlen( Pointer, 64 );
				const auto LineEnd = std::find_if( Pointer, Pointer + MaxLen, [] ( const char C ) { return C == '\r' || C == '\n'; } );
				const auto Found   = static_cast<std::size_t>( LineEnd - Pointer );
				return { Pointer, Found < 64 ? Found : 24 };
			}

			std::string HandleWideCharArray( const void* Buffer )
			{
				std::string  String     = {};
				const auto   WideString = static_cast<const wchar_t*>( Buffer );
				const size_t Length     = wcsnlen( WideString, 24 );
				String.append( WideString, WideString + Length );
				return String;
			}

			std::string GetFormattedData( const DataStructureType Type, void* Buffer )
			{
				auto AsFloat = [&] { return *static_cast<float*>( Buffer ); };

				switch ( Type )
				{
				case T_x8:
					return std::format( "[{} | 0x{:X}]", *static_cast<uint8_t*>( Buffer ), *static_cast<uint8_t*>( Buffer ) );
				case T_x16:
					return std::format( "[{} | 0x{:X}]", *static_cast<uint16_t*>( Buffer ), *static_cast<uint16_t*>( Buffer ) );
				case T_x32:
					{
						if ( fabsf( AsFloat() ) < 100'000 ) return std::format( "({:.6f}) [{} | 0x{:X}]", AsFloat(), *static_cast<uint32_t*>( Buffer ), *static_cast<uint32_t*>( Buffer ) );
						return std::format( "[{} | 0x{:X}]", *static_cast<uint32_t*>( Buffer ), *static_cast<uint32_t*>( Buffer ) );
					}
				case T_x64:
					{
						if ( fabsf( AsFloat() ) < 100'000 ) return std::format( "({:.6f}) [{} | 0x{:X}]", AsFloat(), *static_cast<uint64_t*>( Buffer ), *static_cast<uint64_t*>( Buffer ) );
						return std::format( "[{} | 0x{:X}]", *static_cast<uint64_t*>( Buffer ), *static_cast<uint64_t*>( Buffer ) );
					}
				case T_byte:
					return std::format( "0x{:X}", *static_cast<uint8_t*>( Buffer ) );
				case T_word:
					return std::format( "0x{:X}", *static_cast<uint16_t*>( Buffer ) );
				case T_dword:
					return std::format( "0x{:X}", *static_cast<uint32_t*>( Buffer ) );
				case T_qword:
					return std::format( "0x{:X}", *static_cast<uint64_t*>( Buffer ) );
				case T_int8:
					return std::format( "{}", *static_cast<int8_t*>( Buffer ) );
				case T_int16:
					return std::format( "{}", *static_cast<int16_t*>( Buffer ) );
				case T_int32:
					return std::format( "{}", *static_cast<int32_t*>( Buffer ) );
				case T_int64:
					return std::format( "{}", *static_cast<int64_t*>( Buffer ) );
				case T_uint8:
					return std::format( "{}", *static_cast<uint8_t*>( Buffer ) );
				case T_uint16:
					return std::format( "{}", *static_cast<uint16_t*>( Buffer ) );
				case T_uint32:
					return std::format( "{}", *static_cast<uint32_t*>( Buffer ) );
				case T_uint64:
					return std::format( "{}", *static_cast<uint64_t*>( Buffer ) );
				case T_float:
					return std::format( "{:.6f}", *static_cast<float*>( Buffer ) );
				case T_double:
					return std::format( "{:.10f}", *static_cast<double*>( Buffer ) );
				case T_bool:
					return *static_cast<bool*>( Buffer ) ? _( "True" ) : _( "False" );
				case T_char_array:
					return HandleCharArray( Buffer );
				case T_wide_array:
					return HandleWideCharArray( Buffer );
				case T_Vector2f:
					{
						auto* V = static_cast<float*>( Buffer );
						return std::format( "({:.4f}, {:.4f})", V[ 0 ], V[ 1 ] );
					}
				case T_Vector3f:
					{
						auto* V = static_cast<float*>( Buffer );
						return std::format( "({:.4f}, {:.4f}, {:.4f})", V[ 0 ], V[ 1 ], V[ 2 ] );
					}
				case T_Vector4f:
					{
						auto* V = static_cast<float*>( Buffer );
						return std::format( "({:.4f}, {:.4f}, {:.4f}, {:.4f})", V[ 0 ], V[ 1 ], V[ 2 ], V[ 3 ] );
					}
				case T_matrix4x4:
					{
						auto* M = static_cast<float*>( Buffer );
						return std::format(
						                   "[{:.4f}, {:.4f}, {:.4f}, {:.4f}]\n"
						                   "[{:.4f}, {:.4f}, {:.4f}, {:.4f}]\n"
						                   "[{:.4f}, {:.4f}, {:.4f}, {:.4f}]\n"
						                   "[{:.4f}, {:.4f}, {:.4f}, {:.4f}]",
						                   M[ 0 ], M[ 1 ], M[ 2 ], M[ 3 ],
						                   M[ 4 ], M[ 5 ], M[ 6 ], M[ 7 ],
						                   M[ 8 ], M[ 9 ], M[ 10 ], M[ 11 ],
						                   M[ 12 ], M[ 13 ], M[ 14 ], M[ 15 ] );
					}
				case T_custom:
				case T_undefined:
				default:
					return _( "???" );
				}
			}

			char SanitizeByte( const uint8_t Value )
			{
				return Value >= 32 && Value <= 126 ? static_cast<char>( Value ) : '.';
			}

			Objects::Button MakeTransparentButton( const Objects::ButtonType Type )
			{
				Objects::Button Button;
				Button.Type            = Type;
				Button.Override.Border = Graphics::RGBA( 255, 255, 255, 0 );
				Button.Override.Hover  = Graphics::RGBA( 255, 255, 255, 0 );
				Button.Override.Click  = Graphics::RGBA( 255, 255, 255, 0 );
				Button.Override.Idle   = Graphics::RGBA( 255, 255, 255, 0 );
				return Button;
			}

			void ReadConfig()
			{
				ReadInterval   = Config::GetOrSet( _( "Dissector" ), _( "ReadIntervalMS" ), 1'000ll );
				DefaultTabSize = Config::GetOrSet( _( "Dissector" ), _( "DefaultTabSizeBytes" ), 64ll );
				MaxDepth       = Config::GetOrSet( _( "Dissector" ), _( "MaxViewDepth" ), 4ll );
			}
		}
	}

	// Structs 
	namespace
	{
		struct ClassInstance
		{
			std::string                        Label = {};
			std::unique_ptr<Memory::ReadGroup> Group;

			struct
			{
				Memory::InstanceData* Node = nullptr;
				std::unique_ptr<View> View = nullptr;
			} RootData;

			std::vector<std::unique_ptr<View>> Children = {};

			explicit ClassInstance( const uintptr_t BaseAddress, const size_t Size, const uint32_t PID )
			{
				Group         = std::make_unique<Memory::ReadGroup>();
				RootData.Node = Group->AddRoot( BaseAddress, Size, PID );
			}

			// Defined lower because we need View definition for this
			View* ExpandNode( Memory::InstanceData* ParentNode, size_t OffsetInParent, size_t ReadSize, uint32_t Depth );

			void ScheduleRead( Memory::Manager& Manager ) const
			{
				Manager.Schedule( Group.get() );
			}
		};
	}

	// Forward Declarations
	namespace
	{
		void Disassemble( LineData& LD );
	}

	class View
	{
	public:
		template <class... Ts>
		struct Overloads : Ts...
		{
			using Ts::operator()...;
		};

		using ViewClock = std::chrono::high_resolution_clock;

		// Necessary for ExpandNode
		friend ClassInstance;

		View() = default;

		explicit View( ClassInstance* Owner, Memory::InstanceData* Node, const uint32_t Depth = 0, const std::optional<std::string>& AddressExpression = std::nullopt ) : Owner( Owner ), Node( Node ), Depth( Depth )
		{
			++GlobalIDX;
			this->IDX = GlobalIDX;

			this->Expanded         = Depth == 0;
			this->Data.BaseAddress = Node->GetAddress();
			this->Data.PID         = Node->GetPID();

			this->Buttons.Header          = Helpers::MakeTransparentButton( Objects::ButtonType::Circle );
			this->Buttons.Header.Callback = nullptr; // Handled directly in WndProc
			this->Buttons.Header.FontSize = 8.f;
			this->Buttons.Header.Size.x   = LINE_HEIGHT * 0.8f;
			this->Buttons.Header.Size.y   = LINE_HEIGHT * 0.8f;

			this->AddressString = AddressExpression.value_or( std::format( "0x{:X}", Node->GetAddress() ) );
			this->SizeString    = std::format( "{}", Node->GetSize() );
		}

		// TODO: This is pretty unsafe right now
		Memory::Field* GetField( LineData& LD )
		{
			if ( LD.IsSynthetic() ) return &LD.PaddingField.value();
			return &BackingData->Fields[ static_cast<size_t>( LD.SourceIndex ) ];
		}

		size_t GetOffset( const LineData& LD ) const
		{
			return LD.IsSynthetic() ? LD.PaddingField->Offset : BackingData->Fields[ LD.SourceIndex ].Offset;
		}

		void Render( Vector2f& Cursor, size_t& i, bool JustConsumedSnapshot = false )
		{
			if ( !this->BackingData )
			{
				return;
			}

			if ( Depth == 0 )
			{
				const bool Consumed = Owner->Group->TryConsume( Node, Snapshot );

				if ( Consumed )
				{
					JustConsumedSnapshot = true;
					Owner->Group->MarkConsumed();
					NextRead = ViewClock::now() + std::chrono::seconds( 1 );
				}
			}
			else
			{
				const std::unique_lock Lock( Owner->Group->Mutex, std::try_to_lock );

				if ( Lock )
				{
					Snapshot.Buffer    = Node->FrontBuffer.Buffer;
					Snapshot.Timestamp = Node->FrontBuffer.Timestamp;
				}
			}

			const auto Now = ViewClock::now();
			if ( Now > NextRead ) ScheduleRead();

			if ( RecalculationScheduled )
			{
				Recalculate();
				this->RecalculationScheduled = false;
			}

			if ( this->IDX != SelectCursorViewIDX ) this->SelectedOffsets.clear();

			// Zydis pass
			if ( JustConsumedSnapshot )
			{
				auto HandleZydis = [&] ( LineSlot& S )
				{
					auto& LD = S.Data;
					if ( !LD.Embedded.Child || GetField( LD )->Type != T_pFunction ) return;

					const auto& [ Buffer, Timestamp ] = LD.Embedded.Child->Snapshot;
					if ( Buffer.empty() || LD.Embedded.Disassembly.has_value() ) return;

					Disassemble( LD );
				};

				std::ranges::for_each( DefinedLines | std::views::values, HandleZydis );
			}

			const float DeltaTime = std::chrono::duration<float>( Now - LastFrameTime ).count();
			LastFrameTime         = Now;

			if ( fabsf( ScrollTarget - ScrollOffset ) > SCROLL_SNAP_DIST ) ScrollOffset += ( ScrollTarget - ScrollOffset ) * ( 1.f - std::expf( -SCROLL_DECAY * DeltaTime ) );
			else ScrollOffset = ScrollTarget;

			this->BackingData->RefreshMaxWidth();

			const auto WindowData = Window::GetWindowData();

			Data.Owner       = this;
			Data.WindowSize  = Vector2f( WindowData->Width, WindowData->Height );
			Data.BaseAddress = Node->GetAddress();
			Data.Buffer      = Snapshot.Buffer.data();

			const auto HexSize  = FontManager::FontSize( _( "FF" ), 16.f, true, 0 );
			const auto CharSize = FontManager::FontSize( _( "." ), 16.f, true, 0 );

			if ( !Data.Buffer || Snapshot.Buffer.empty() )
			{
				if ( Depth > 0 ) Cursor.y += LINE_HEIGHT;
				return;
			}

			const Vector2f ClipOrigin = Cursor;
			ViewableHeight            = WindowData->Height - Cursor.y;

			const ImVec2 ClipMin( 0.f, Cursor.y );
			const ImVec2 ClipMax( WindowData->Width, WindowData->Height );

			ImGui::GetBackgroundDrawList()->PushClipRect( ClipMin, ClipMax, true );
			ImGui::GetForegroundDrawList()->PushClipRect( ClipMin, ClipMax, true );
			ImGui::GetWindowDrawList()->PushClipRect( ClipMin, ClipMax, true );

			ContentHeight = ComputeContentHeight();

			const float MaxScroll = max( 0.f, ContentHeight - ViewableHeight );
			ScrollTarget          = std::clamp( ScrollTarget, 0.f, MaxScroll );
			ScrollOffset          = std::clamp( ScrollOffset, 0.f, MaxScroll );

			this->LastOrigin = { GridData.Origin.x, Cursor.y };

			Vector2f ScrolledCursor = Cursor;
			ScrolledCursor.y -= ScrollOffset;

			// Header
			{
				const Vector2f Original = ScrolledCursor;

				ScrolledCursor.x -= TAB_WIDTH / 2.f;
				i++;

				// Alternating Color
				if ( i % 2 == 0 )
				{
					const uint32_t Color = Themes::Manager::GetColor( "MemoryGrid_ColorA"_H );
					Graphics::AddFilledRectangle( Vector2f( 1.f, Cursor.y ), Data.WindowSize.x - 2.f, LINE_HEIGHT, Color );
				}

				ScrolledCursor.y += LINE_HEIGHT / 2.f;

				// Header Button (expand view)
				{
					Buttons.Header.Label    = this->Expanded ? "\xF3\xB0\x81\x86" : "\xF3\xB0\x81\x95";
					Buttons.Header.Position = ScrolledCursor;
					Buttons.Header.Render();
				}

				ScrolledCursor.x               = Original.x;
				constexpr float HEADER_PADDING = 5.f;

				// Address Field
				{
					std::string& String = this->AddressString;

					if ( this->Depth == 0 )
					{
						auto OnChange = [this] ( const std::string& NewString, const bool WasCompleted )
						{
							this->AddressString = NewString;
							if ( !WasCompleted ) return;

							// On empty string, default to 0x0 so we don't softlock the user if they accidentally empty the label
							if ( this->AddressString.empty() )
							{
								this->AddressString = _( "0x0" );
							}

							const std::optional<uintptr_t> ParentAddress = this->Node->Parent ? std::optional( this->Node->Parent->GetAddress() ) : std::nullopt;

							const uintptr_t NewAddress = Parser::ParseComplex( NewString, this->Node->GetPID(), ParentAddress );
							this->Node->SetAddress( NewAddress );
							this->ScheduleRead();
						};

						FontManager::AddModifiableText( String, 16.f, Themes::Manager::GetColor( "MemoryGrid_Offset"_H ), ScrolledCursor, true, DT_VCENTER, 0, OnChange );
					}
					else
					{
						FontManager::AddSelectableText( String, 16.f, Themes::Manager::GetColor( "MemoryGrid_Offset"_H ), ScrolledCursor, true, DT_VCENTER );
					}

					ScrolledCursor.x += FontManager::FontSize( String, 16.f, true ).x + HEADER_PADDING;
				}

				// Label
				{
					const auto Color  = Themes::Manager::GetColor( "MemoryGrid_Char"_H );
					const auto Prefix = FontManager::FontSize( "Class '", 16.f, false );
					const auto Suffix = FontManager::FontSize( "'", 16.f, false );

					auto OnChange = [this] ( const std::string& NewString, const bool WasCompleted )
					{
						this->BackingData->Label = NewString;

						if ( WasCompleted && this->BackingData->Label.empty() ) this->BackingData->Label = _( "Unnamed" );
					};

					FontManager::AddSelectableText( "Class '", 16.f, Color, ScrolledCursor, false, DT_VCENTER );
					ScrolledCursor.x += Prefix.x;
					FontManager::AddModifiableText( this->BackingData->Label, 16.f, Color, ScrolledCursor, false, DT_VCENTER, 0, OnChange );
					ScrolledCursor.x += FontManager::FontSize( this->BackingData->Label, 16.f, false ).x;
					FontManager::AddSelectableText( "'", 16.f, Color, ScrolledCursor, false, DT_VCENTER );
					ScrolledCursor.x += Suffix.x + HEADER_PADDING;
				}

				// Size
				{
					std::string& String = this->SizeString;
					const auto   Color  = Themes::Manager::GetColor( "MemoryGrid_Formatted"_H );
					const auto   Prefix = FontManager::FontSize( "Size [", 16.f, false );

					auto OnChange = [this] ( const std::string& NewString, const bool WasCompleted )
					{
						this->SizeString = NewString;
						if ( !WasCompleted ) return;

						if ( this->SizeString.empty() )
						{
							this->SizeString = _( "64" );
						}

						const uintptr_t NewSize = Parser::ParseComplex( this->SizeString, this->Node->GetPID() );
						this->Node->SetSize( NewSize );
						this->BackingData->Size      = NewSize;
						this->RecalculationScheduled = true;
						this->ScheduleRead();
					};

					FontManager::AddSelectableText( "Size [", 16.f, Color, ScrolledCursor, false, DT_VCENTER );
					ScrolledCursor.x += Prefix.x;
					FontManager::AddModifiableText( String, 16.f, Color, ScrolledCursor, false, DT_VCENTER, 0, OnChange );
					ScrolledCursor.x += FontManager::FontSize( String, 16.f, false ).x;
					FontManager::AddSelectableText( " bytes]", 16.f, Color, ScrolledCursor, false, DT_VCENTER );
				}

				ScrolledCursor.y += LINE_HEIGHT / 2.f;
				ScrolledCursor.x = Original.x;
			}

			if ( !this->Expanded )
			{
				ImGui::GetBackgroundDrawList()->PopClipRect();
				ImGui::GetForegroundDrawList()->PopClipRect();
				ImGui::GetWindowDrawList()->PopClipRect();
				Cursor = ScrolledCursor;
				return;
			}

			float        FloatBuffer         = 0.f;
			const size_t MaxAddress          = this->GetAddress() + this->GetSize();
			std::string  MaxAddressFormatted = std::format( "0x{:X}", MaxAddress );
			const float  MaxAddressSize      = FontManager::FontSize( MaxAddressFormatted, 16.f, true ).x + ELEMENT_PADDING;

			const auto Visitors = Overloads
			{
				[] ( const std::reference_wrapper<const std::string> String ) { return FontManager::FontSize( String.get(), 16.f, true ).x; },
				[] ( const float Value ) { return Value; },
				[] ( const char* String ) { return FontManager::FontSize( String, 16.f, true ).x; }
			};

			auto CalculateAndSetOffset = [&] ( float& Destination, std::variant<std::reference_wrapper<const std::string>, const char*, float> Variable )
			{
				FloatBuffer += std::visit( Visitors, Variable ) + ELEMENT_PADDING;
				Destination = FloatBuffer;
			};

			auto CalculateOffset = [&] ( float& Destination, std::variant<std::reference_wrapper<const std::string>, const char*, float> Variable )
			{
				Destination = std::visit( Visitors, Variable ) + ELEMENT_PADDING;
			};

			const float HexRepresentationSize  = ( HexSize.x + ELEMENT_PADDING ) * 8.f - ELEMENT_PADDING;
			const float CharRepresentationSize = CharSize.x * 8.f + ELEMENT_PADDING;
			float       AddressStringSize;

			Data.Columns.Offset = 0.f;
			CalculateAndSetOffset( Data.Columns.Address, _( "00000" ) );
			// Since this is user-modifiable we need a fallback while they're editing if the address string is too small
			{
				CalculateOffset( AddressStringSize, this->AddressString );
				FloatBuffer += max( AddressStringSize, MaxAddressSize );
				Data.Columns.NamedTypeOrLabel = FloatBuffer;
			}
			CalculateAndSetOffset( Data.Columns.HexOrLabel, max( BackingData->RenderMetrics.TypeWidth, BackingData->RenderMetrics.LabelWidth ) );
			CalculateAndSetOffset( Data.Columns.Chars, max( HexRepresentationSize, BackingData->RenderMetrics.LabelWidth ) );
			CalculateAndSetOffset( Data.Columns.Formatted, CharRepresentationSize );

			Data.HexWidth  = HexSize.x;
			Data.CharWidth = CharSize.x;

			auto RenderSlot = [&] ( LineSlot* S )
			{
				auto&          LD    = S->Data;
				Memory::Field* Field = this->GetField( LD );

				const float LineTop     = ScrolledCursor.y;
				const float LineBottom  = LineTop + LINE_HEIGHT;
				const bool  LineVisible = LineBottom >= ClipOrigin.y && LineTop <= ClipOrigin.y + ViewableHeight;

				if ( LineVisible )
				{
					const size_t LineOffset = GetOffset( LD );
					const bool   IsSelected = SelectedOffsets.contains( LineOffset );
					S->Line.Render( ScrolledCursor, &Data, BackingData.get(), Field, i, IsSelected );
				}
				else
				{
					i++;
					ScrolledCursor.y += LINE_HEIGHT;

					if ( LD.Embedded.Child )
					{
						if ( LD.Embedded.Expanded )
						{
							ScrolledCursor.x += TAB_WIDTH;
							if ( Field->Type != T_pFunction ) LD.Embedded.Child->Render( ScrolledCursor, i, JustConsumedSnapshot );
							else Internals::RenderFunction( ScrolledCursor, &Data, LD, i );
							ScrolledCursor.x -= TAB_WIDTH;
						}
					}
				}
			};

			std::ranges::for_each( RenderOrder, RenderSlot );

			if ( Data.ScheduleRecalculate )
			{
				Data.ScheduleRecalculate     = false;
				this->RecalculationScheduled = true;
			}

			ImGui::GetBackgroundDrawList()->PopClipRect();
			ImGui::GetForegroundDrawList()->PopClipRect();
			ImGui::GetWindowDrawList()->PopClipRect();

			Cursor = ScrolledCursor;
		}

		void OnMouseWheel( const WPARAM wParam )
		{
			const float Delta     = -static_cast<float>( GET_WHEEL_DELTA_WPARAM( wParam ) ) / WHEEL_DELTA;
			const float MaxScroll = max( 0.f, ContentHeight - ViewableHeight );
			ScrollTarget          = std::clamp( ScrollTarget + Delta * SCROLL_SPEED, 0.f, MaxScroll );
		}

		void OnKeyDown( const WPARAM wParam )
		{
			const float MaxScroll = max( 0.f, ContentHeight - ViewableHeight );

			switch ( wParam )
			{
			case VK_UP:
				ScrollTarget -= LINE_HEIGHT;
				break;
			case VK_DOWN:
				ScrollTarget += LINE_HEIGHT;
				break;
			case VK_PRIOR:
				ScrollTarget -= ViewableHeight;
				break;
			case VK_NEXT:
				ScrollTarget += ViewableHeight;
				break;
			case VK_HOME:
				ScrollTarget = 0.f;
				break;
			case VK_END:
				ScrollTarget = MaxScroll;
				break;
			case VK_ESCAPE:
				SelectedOffsets.clear();
				AnchorOffset = SIZE_MAX;
				return;
			default:
				return;
			}

			ScrollTarget = std::clamp( ScrollTarget, 0.f, MaxScroll );
		}

		bool WndProc( const Vector2f CursorPosition, const uint32_t Message, const WPARAM wParam, const LPARAM lParam )
		{
			if ( this->LastOrigin.IsZero() ) return false;

			if ( Buttons.Header.WndProc( CursorPosition, Message, wParam, lParam ) )
			{
				if ( Message == WM_LBUTTONUP ) this->Expanded = !this->Expanded;
				return true;
			}

			for ( const auto&& [ Index, Slot ] : RenderOrder | std::views::enumerate )
			{
				const auto LineResult = Slot->Line.WndProc( CursorPosition, Message, wParam, lParam );

				switch ( LineResult )
				{
				case Objects::NOT_CONSUMED:
					break;
				case Objects::LINE_HEIGHT_MATCHES:
					{
						const auto&  LD     = Slot->Data;
						const size_t Offset = GetOffset( LD );

						return HandleLineSelection( CursorPosition, static_cast<size_t>( Index ), Offset );
					}
				case Objects::BUTTON_CLICKED:
					return true;
				}

				const auto& LD = Slot->Data;
				if ( LD.Embedded.Child && LD.Embedded.Expanded )
				{
					if ( LD.Embedded.Child->WndProc( CursorPosition, Message, wParam, lParam ) ) return true;
				}
			}

			return false;
		}

		// Returns true if any line changed, false otherwise
		bool ChangeSelectedLines( const DataStructureType Type )
		{
			// Find matching IDX
			auto Recurse = [] ( this auto& Self, View* Current ) -> View*
			{
				if ( !Current ) return nullptr;

				if ( Current->IDX == SelectCursorViewIDX ) return Current;

				for ( const auto& Line : Current->DefinedLines | std::views::values )
				{
					if ( auto* Found = Self( Line.Data.Embedded.Child ) )
					{
						return Found;
					}
				}

				return nullptr;
			};

			View* SelectedView = Recurse( this );

			if ( !SelectedView || SelectedView->SelectedOffsets.empty() )
			{
				std::println( "ChangeSelectedLines::SelectedView nullptr or SelectedOffsets empty (0x{:X}", reinterpret_cast<uintptr_t>( SelectedView ) );
				return false;
			}

			bool RequiresImmediate = false;
			std::println( "SelectedOffsets.size() = {}", SelectedView->SelectedOffsets.size() );

			// TODO: This has the same issue as ReClass for now, just get Min/Max and actually set each and every field, not just the "selected" ones
			std::ranges::for_each( SelectedView->SelectedOffsets, [&] ( const size_t Offset ) { SelectedView->ChangeFieldType( Offset, Type, RequiresImmediate ); } );
			std::println( "RequiresImmediate = {}", RequiresImmediate );

			if ( RequiresImmediate ) Recalculate();

			return RequiresImmediate;
		}

		void ChangeOffset( const size_t Offset, const DataStructureType Type, const std::optional<size_t> Size = std::nullopt )
		{
			bool RequiresImmediateChange = false;

			auto HandleRecalculation = [&]()
			{
				if ( RequiresImmediateChange )
				{
					this->Recalculate();
				}
				else
				{
					this->RecalculationScheduled = true;
				}
			};

			if ( ChangeFieldType( Offset, Type, RequiresImmediateChange, Size ) )
			{
				HandleRecalculation();
				return;
			}

			if ( Offset >= Node->GetSize() ) return;

			// TODO: Frankly this is left over code from my 1st iteration of this class, I'm unsure if we need this part?
			RequiresImmediateChange = true;
			Memory::Field NewField( Type, Offset );
			BackingData->Fields.push_back( std::move( NewField ) );
			HandleRecalculation();
		}

		void EnsureNoOverlap( const size_t Offset, const size_t Size ) const
		{
			std::erase_if( BackingData->Fields, [&] ( const Memory::Field& F )
			{
				return F.Offset < Offset + Size && F.Offset + F.Size > Offset;
			} );
		}

		void RequestRecalculation() { this->RecalculationScheduled = true; }

		// Getter/Setters
		// const Memory::Structs::Info& GetStructureTemplate() const { return BackingData; }
		std::shared_ptr<Memory::Info> GetBackingData() const { return BackingData; }

		void SetBackingData( const std::shared_ptr<Memory::Info>& NewBackingData )
		{
			BackingData            = NewBackingData;
			RecalculationScheduled = true;
		}

		uintptr_t GetAddress() const { return this->Node->GetAddress(); }
		void      SetBaseAddress( const uintptr_t NewAddress ) const { Node->SetAddress( NewAddress ); }
		size_t    GetSize() const { return this->Node->GetSize(); }

		void SetSize( const size_t NewSize )
		{
			this->Owner->Group->SetSize( this->Node, NewSize );
			this->Snapshot.Buffer.resize( NewSize );
		}

		// Memory Related Data
		ClassInstance*         Owner    = nullptr;
		Memory::InstanceData*  Node     = nullptr;
		Memory::MemorySnapshot Snapshot = {};

		// Exposed GUI Elements
		struct
		{
			Objects::Button Header   = {};
			uint32_t        TabIDX   = 0; // IDX of the non-owned TabSelect Button
			uint32_t        CloseIDX = 0; // IDX of the non-owned CloseTab Button
		} Buttons;

		// Misc. Data
		size_t           Depth                  = 0;
		uint32_t         IDX                    = 0;
		bool             Expanded               = false;
		RenderParameters Data                   = {};
		Vector2f         LastOrigin             = Vector2f::Zero;
		bool             RecalculationScheduled = false;

	private:
		void ScheduleRead()
		{
			NextRead                  = ViewClock::now() + std::chrono::milliseconds( ReadInterval );
			Memory::Manager* Instance = Memory::GetManager();
			Instance->Schedule( Owner->Group.get() );
		}

		// This is an expensive call that should only be done on frame begin (when RecalculationScheduled is checked)
		// Or if necessary after an action (e.g. changing a field type) to make sure we don't have one frame with wrong data
		void Recalculate()
		{
			Sort();
			Populate();
		}

		std::optional<size_t> GetRenderIndexFromOffset( const size_t Offset ) const
		{
			for ( const auto&& [ Index, Slot ] : RenderOrder | std::views::enumerate )
			{
				const auto&  ScopedData   = Slot->Data;
				const size_t ScopedOffset = GetOffset( ScopedData );

				if ( ScopedOffset == Offset ) return Index;
			}

			return std::nullopt;
		}

		// Returns true if the field (or size) changed, returns false if the type and size were unchanged
		bool ChangeFieldType( const size_t Offset, const DataStructureType NewType, bool& RequiresImmediate, const std::optional<size_t> Size = std::nullopt )
		{
			RequiresImmediate       = false;
			const size_t ActualSize = Size.value_or( Globals::GetDefaultDataStructureSize( NewType ) );

			EnsureNoOverlap( Offset, ActualSize );

			const auto Iterator = std::ranges::find_if( BackingData->Fields, [Offset] ( const Memory::Field& F )
			{
				return F.Offset == Offset;
			} );

			const bool     RequiresPromotion = Iterator == BackingData->Fields.end();
			Memory::Field* Target;

			if ( RequiresPromotion )
			{
				std::string NewName  = std::format( "Off_{:X}", Offset );
				auto&       Inserted = BackingData->Fields.emplace_back( NewType, Offset, NewName );
				Target               = &Inserted;
			}
			else
			{
				Target = std::addressof( *Iterator );
			}

			if ( !RequiresPromotion && Target->Type == NewType && Target->Size == ActualSize ) return false;

			// TODO: Maybe calling this late fucks something up?

			Target->Type     = NewType;
			Target->Size     = ActualSize;
			Target->Embedded = _( "RESERVED_NONE" );

			if ( Target->IsPOD() )
			{
				// If we're currently a POD, we make sure that if we were previously not, we clear the data properly
				const auto ScopedIterator = DefinedLines.find( Offset );

				if ( ScopedIterator != DefinedLines.end() )
				{
					auto& Slot = ScopedIterator->second;
					Slot.Data.ClearEmbeddedData();
					Slot.Line.ClearExpandButton();
				}
			}

			RequiresImmediate = true; // TODO: Actual logic here
			return true;
		}

		// Returns true if the size changed, returns false if the size was unchanged or the field didn't exist in template
		bool ChangeFieldSize( const size_t Offset, const size_t Size )
		{
			const auto Iterator = std::ranges::find_if( BackingData->Fields, [Offset] ( const Memory::Field& F )
			{
				return F.Offset == Offset;
			} );

			if ( Iterator == BackingData->Fields.end() || Iterator->Size == Size ) return false;
			Iterator->Size = Size;
			return true;
		}

		bool HandleLineSelection( const Vector2f& CursorPosition, const size_t ClickedIndex, const size_t ClickedOffset )
		{
			SelectCursorViewIDX = this->IDX;
			if ( CursorPosition.x < LastOrigin.x ) return false;

			const bool Shift = GetKeyState( VK_SHIFT ) & 0x8000;
			const bool Ctrl  = GetKeyState( VK_CONTROL ) & 0x8000;

			if ( Shift && AnchorOffset != SIZE_MAX )
			{
				const std::optional<size_t> Anchor = GetRenderIndexFromOffset( AnchorOffset );

				if ( Anchor.has_value() )
				{
					const size_t AnchorIndex = Anchor.value();
					const size_t Low         = min( AnchorIndex, ClickedIndex );
					const size_t High        = max( AnchorIndex, ClickedIndex );

					if ( !Ctrl ) SelectedOffsets.clear();

					for ( size_t LocalIDX = Low; LocalIDX <= High; LocalIDX++ )
					{
						const auto&  LD          = RenderOrder[ LocalIDX ]->Data;
						const size_t EntryOffset = GetOffset( LD );

						SelectedOffsets.insert( EntryOffset );
					}
				}
			}
			else if ( Ctrl )
			{
				if ( !SelectedOffsets.insert( ClickedOffset ).second ) SelectedOffsets.erase( ClickedOffset );
				AnchorOffset = ClickedOffset;
			}
			else
			{
				SelectedOffsets.clear();
				SelectedOffsets.insert( ClickedOffset );
				AnchorOffset = ClickedOffset;
			}

			return true;
		}

		float ComputeContentHeight() const
		{
			// Header is always visible if this is called, so min of this is always LINE_HEIGHT
			if ( !Expanded ) return LINE_HEIGHT;

			float Height = LINE_HEIGHT;
			for ( const auto* Line : RenderOrder )
			{
				Height += LINE_HEIGHT;
				const auto& LD = Line->Data;
				if ( LD.Embedded.Child && LD.Embedded.Expanded ) Height += LD.Embedded.Child->ComputeContentHeight();
			}

			return Height;
		}

		// This routine creates Children View where applicable
		void Populate()
		{
			if ( std::cmp_greater_equal( Depth, MaxDepth ) ) return;

			for ( const auto& Field : BackingData->Fields )
			{
				if ( Field.IsPOD() ) continue;

				auto Iterator = DefinedLines.find( Field.Offset );

				if ( Iterator == DefinedLines.end() ) continue;

				LineSlot& Slot = Iterator->second;
				LineData& LD   = Slot.Data;

				const bool HasEmbeddedData = std::holds_alternative<std::shared_ptr<Memory::Info>>( Field.Embedded );

				if ( !LD.Embedded.Child )
				{
					const size_t PointerSize    = HasEmbeddedData ? std::get<std::shared_ptr<Memory::Info>>( Field.Embedded )->Size : 256;
					View*        ChildView      = Owner->ExpandNode( Node, Field.Offset, PointerSize, Depth + 1 );
					ChildView->Node->IsEmbedded = Field.Type == T_EmbeddedClass;
					LD.Embedded.Child           = ChildView;
				}

				Slot.Line.EnsureExpandButton();

				if ( HasEmbeddedData )
				{
					LD.Embedded.Child->SetBackingData( std::get<std::shared_ptr<Memory::Info>>( Field.Embedded ) );
				}
				else
				{
					// TODO: There is probably a better way than a dummy Info
					LD.Embedded.Child->SetBackingData( ClassManager::GetEmptyView() );
					LD.Embedded.Child->SetSize( DefaultTabSize );
				}
			}
		}

		void Sort()
		{
			std::unordered_set<size_t> LiveOffsets;
			LiveOffsets.reserve( BackingData->Fields.size() );

			for ( const auto&& [ Index, Field ] : BackingData->Fields | std::views::enumerate )
			{
				const size_t Offset = Field.Offset;
				LiveOffsets.insert( Offset );

				if ( auto Iterator = DefinedLines.find( Offset ); Iterator != DefinedLines.end() )
				{
					Iterator->second.Data.SourceIndex = static_cast<ptrdiff_t>( Index );
				}
				else
				{
					LineData NewLD;
					NewLD.SourceIndex = static_cast<ptrdiff_t>( Index );
					auto& Slot        = DefinedLines.emplace( Offset, LineSlot{} ).first->second;
					Slot.Data         = std::move( NewLD );
				}
			}

			// Remove defined lines that are no longer live (aka they're now padding, so they don't belong in DefinedLines)
			std::erase_if( DefinedLines, [&] ( const auto& Slot ) { return !LiveOffsets.contains( Slot.first ); } );
			PaddingLines.clear();

			std::vector<const Memory::Field*> Sorted;
			Sorted.reserve( BackingData->Fields.size() );
			std::ranges::for_each( BackingData->Fields, [&] ( const Memory::Field& F ) { Sorted.push_back( &F ); } );
			std::ranges::sort( Sorted, {}, &Memory::Field::Offset );

			uintptr_t CurrentOffset = 0;

			auto EmitPadding = [&] ( const uintptr_t TargetOffset )
			{
				while ( CurrentOffset < TargetOffset )
				{
					const size_t Difference = TargetOffset - CurrentOffset;
					size_t       ChunkSize  = 1;

					for ( size_t S = 8; S >= 2; S >>= 1 )
					{
						if ( CurrentOffset % S == 0 && Difference >= S )
						{
							ChunkSize = S;
							break;
						}
					}

					LineData Pad;
					Pad.PaddingField = Memory::Field( ChunkSize, CurrentOffset );
					Pad.SourceIndex  = -1;

					auto& Slot = PaddingLines.emplace( CurrentOffset, LineSlot{} ).first->second;
					Slot.Data  = std::move( Pad );
					CurrentOffset += ChunkSize;
				}
			};

			auto HandleLine = [&] ( const Memory::Field* Field )
			{
				EmitPadding( Field->Offset );
				CurrentOffset = max( CurrentOffset, Field->Offset + Field->Size );
			};

			// If you can't tell, I really like std::ranges::for_each
			std::ranges::for_each( Sorted, HandleLine );

			// Make sure we pad to the end
			EmitPadding( Node->GetSize() );

			RenderOrder.clear();
			RenderOrder.reserve( DefinedLines.size() + PaddingLines.size() );

			for ( auto& Slot : DefinedLines | std::views::values ) RenderOrder.push_back( &Slot );
			for ( auto& Slot : PaddingLines | std::views::values ) RenderOrder.push_back( &Slot );

			std::ranges::sort( RenderOrder, [this] ( const LineSlot* A, const LineSlot* B )
			{
				const size_t OffA = GetOffset( A->Data );
				const size_t OffB = GetOffset( B->Data );
				return OffA < OffB;
			} );
		}

		// Selection Data
		std::unordered_set<size_t> SelectedOffsets = {};
		size_t                     AnchorOffset    = SIZE_MAX;

		// Timing Data
		ViewClock::time_point LastFrameTime = std::chrono::high_resolution_clock::now();
		ViewClock::time_point NextRead      = std::chrono::high_resolution_clock::now();

		// GUI Objects
		std::unordered_map<size_t, LineSlot> DefinedLines; // Defined by user (or template)
		std::unordered_map<size_t, LineSlot> PaddingLines; // Synthetic, generated by recalculation
		std::vector<LineSlot*>               RenderOrder;

		// Backing Memory Structure
		std::shared_ptr<Memory::Info> BackingData = nullptr;

		// Scroll Values
		float ScrollOffset   = 0.f;
		float ScrollTarget   = 0.f;
		float ContentHeight  = 0.f;
		float ViewableHeight = 0.f;

		// Modifiable Strings
		std::string AddressString = {};
		std::string SizeString    = {};
	};

	// Definitions
	namespace
	{
		void Disassemble( LineData& LD )
		{
			const auto&      ChildView = LD.Embedded.Child;
			const auto&      Node      = ChildView->Node;
			std::unique_lock Lock( ChildView->Owner->Group->Mutex, std::try_to_lock );

			if ( !Lock ) return;

			const auto& Buffer = Node->FrontBuffer.Buffer;

			static ZydisDecoder Decoder = []
			{
				ZydisDecoder D;
				ZydisDecoderInit( &D, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64 );
				return D;
			}();

			static ZydisFormatter Formatter = []
			{
				ZydisFormatter F;
				ZydisFormatterInit( &F, ZYDIS_FORMATTER_STYLE_INTEL );
				return F;
			}();

			DisassemblyCache Cache;
			Cache.Timestamp = ChildView->Snapshot.Timestamp;

			ZydisDecodedInstruction Instruction;
			ZydisDecodedOperand     Operands[ ZYDIS_MAX_OPERAND_COUNT ];

			size_t         Offset = 0;
			const uint8_t* Data   = Buffer.data();
			const size_t   Size   = Buffer.size();

			while ( ZYAN_SUCCESS( ZydisDecoderDecodeFull(&Decoder, Data + Offset, Size - Offset, &Instruction, Operands) ) )
			{
				char StringBuffer[ 256 ];
				ZydisFormatterFormatInstruction( &Formatter, &Instruction, Operands, Instruction.operand_count_visible, StringBuffer, sizeof( StringBuffer ), ChildView->GetAddress() + Offset, nullptr );

				std::string Bytes;
				std::ranges::for_each_n( Data + Offset, Instruction.length, [&Bytes] ( uint8_t Byte ) { Bytes += std::format( "{:02X} ", Byte ); } );

				Cache.Lines.emplace_back( StringBuffer );
				Offset += Instruction.length;
			}

			LD.Embedded.Disassembly = std::move( Cache );
		}

		View* ClassInstance::ExpandNode( Memory::InstanceData* ParentNode, size_t OffsetInParent, const size_t ReadSize, uint32_t Depth )
		{
			const auto ExistingIterator = ParentNode->Children.find( OffsetInParent );

			if ( ExistingIterator != ParentNode->Children.end() )
			{
				const auto Iterator = std::ranges::find( Children, ExistingIterator->second, &View::Node );
				if ( Iterator != Children.end() ) return Iterator->get();
			}

			const std::string ParentExpression = [&]()
			{
				if ( ParentNode == this->RootData.Node ) return this->RootData.View->AddressString;

				for ( const auto& V : Children ) if ( V->Node == ParentNode ) return V->AddressString;

				return std::format( "0x{:X}", ParentNode->GetAddress() );
			}();

			const std::string ChildExpression = OffsetInParent == 0 ? _( "this" ) : std::format( "[this+0x{:X}]", OffsetInParent );

			auto*       ChildNode = Group->AddDependent( ParentNode, OffsetInParent, ReadSize );
			const auto& ChildView = Children.emplace_back( std::make_unique<View>( this, ChildNode, Depth, ChildExpression ) );
			return ChildView.get();
		}

		ClassInstance* GetActiveTab()
		{
			const auto Iterator = std::ranges::find_if( Tabs, [] ( const std::unique_ptr<ClassInstance>& C ) { return C->RootData.View->IDX == ActiveTabIDX; } );
			return Iterator != Tabs.end() ? Iterator->get() : nullptr;
		}
	}

	// Tab Buttons
	// TODO: If there are too many buttons, this breaks, we need a way to move the cursor.
	namespace
	{
		// Returns the width of the button so that we can render TabClose accordingly
		float RenderSelectionButton( Objects::Button& Button, const Vector2f& Cursor, const float Tallest, const bool IsSelected )
		{
			Button.Position = Cursor;
			Button.Position.y -= 1.f;
			Button.Resize();
			Button.Size.x *= 2.f;
			Button.Size.y = Tallest;

			Button.Override.Border = IsSelected ? Themes::Manager::GetColor( "Global_Border"_H ) : Graphics::RGBA( 255, 255, 255, 0 );
			Button.Override.Idle   = IsSelected ? Themes::Manager::GetColor( "Global_PopupBackground"_H ) : Graphics::RGBA( 255, 0, 0, 0 );
			Button.IsRounded       = IsSelected;
			Button.Rounding        = 5.f;
			Button.Flags           = IsSelected ? ImDrawFlags_RoundCornersTop : 0;
			Button.Render();

			if ( IsSelected )
			{
				const float DelimitorHeight = Cursor.y + Tallest - 1.f;

				const Vector2f Start{ Cursor.x + 1.f, DelimitorHeight - 1.f };
				const Vector2f End{ Start.x + Button.Size.x - 3.f, DelimitorHeight - 1.f };
				Graphics::AddLine( Start, End, Button.GetActiveColor(), 4.f );
			}

			return Button.Size.x;
		}

		void RenderCloseButton( Objects::Button& Button, const Vector2f& Cursor, const float Tallest, const float TabWidth )
		{
			const float ButtonRadius  = Tallest * 0.4f;
			const float ButtonPadding = Tallest * 0.1f;

			Button.Position        = Vector2f{ Cursor.x + TabWidth - ButtonRadius - ButtonPadding, Cursor.y + Tallest / 2.f };
			Button.Radius          = ButtonRadius;
			Button.Override.Idle   = Graphics::RGBA( 255, 255, 255, 0 );
			Button.Override.Border = Graphics::RGBA( 255, 255, 255, 0 );
			Button.Render();
		}

		struct TabButton
		{
			Objects::Button TabSelection;
			Objects::Button TabClose;

			void Render( Vector2f& Cursor, const float Tallest, const uint32_t SelectedIDX )
			{
				const bool  IsSelected = this->TabSelection.IDX == SelectedIDX;
				const float TabWidth   = RenderSelectionButton( this->TabSelection, Cursor, Tallest, IsSelected );
				RenderCloseButton( this->TabClose, Cursor, Tallest, TabWidth );
				Cursor.x += TabWidth;
			}

			bool WndProc( const Vector2f Cursor, const uint32_t Message, const WPARAM wParam, const LPARAM lParam )
			{
				if ( this->TabClose.WndProc( Cursor, Message, wParam, lParam ) ) return true;

				return this->TabSelection.WndProc( Cursor, Message, wParam, lParam );
			}
		};

		std::vector<TabButton> Buttons = {};
	}

	// Internal Functions
	namespace Internals
	{
		namespace
		{
			void RenderPadding( Vector2f& Cursor, const Vector2f Original, Memory::Info* Template, LineData& Data, RenderParameters* RenderData )
			{
				std::string String = {};

				// So funnily enough, value_or returns by value, not reference
				// Therefore, to avoid const_cast for AddModifiableText, we have to have a ternary operator here
				auto Field = Data.IsSynthetic() ? Data.PaddingField.value() : Template->Fields[ Data.SourceIndex ];

				auto OnChange = [&, SourceIndex = Data.SourceIndex, Offset = Field.Offset, Size = Field.Size] ( const std::string& NewLabel, const bool WasCompleted )
				{
					if ( !WasCompleted ) return;

					if ( SourceIndex >= 0 )
					{
						Template->Fields[ SourceIndex ].Name = NewLabel;
					}
					else
					{
						DataStructureType Type = T_x64;
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
						default:
							break;
						}

						Memory::Field NewField = { Type, Offset, std::string( NewLabel ) };
						NewField.IsNamed       = true;
						Template->Fields.push_back( std::move( NewField ) );
						RenderData->ScheduleRecalculate = true;
					}
				};

				Cursor.x = Original.x + RenderData->Columns.NamedTypeOrLabel;
				{
					const uint32_t Color = Themes::Manager::GetColor( "MemoryGrid_PlaceholderLabel"_H );
					FontManager::AddModifiableText( Field.Name, 16.f, Color, Cursor, false, DT_VCENTER, 0, OnChange );
				}

				const uintptr_t BufferBaseAddress = reinterpret_cast<uintptr_t>( RenderData->Buffer ) + Field.Offset;

				Cursor.x = Original.x + RenderData->Columns.HexOrLabel;
				for ( auto y = 0u; y < min( 8u, static_cast<unsigned>(Field.Size) ); y++ )
				{
					String = std::format( "{:02X}", *reinterpret_cast<uint8_t*>( BufferBaseAddress + y ) );
					FontManager::AddSelectableText( String, 16.f, Themes::Manager::GetColor( "MemoryGrid_Hex"_H ), Cursor, true, DT_VCENTER );
					Cursor.x += RenderData->HexWidth + ELEMENT_PADDING;
				}

				Cursor.x = Original.x + RenderData->Columns.Chars;
				for ( auto y = 0u; y < min( 8u, static_cast<unsigned>(Field.Size) ); y++ )
				{
					String = std::format( "{}", Helpers::SanitizeByte( *reinterpret_cast<char*>( BufferBaseAddress + y ) ) );
					FontManager::AddSelectableText( String, 16.f, Themes::Manager::GetColor( "MemoryGrid_Char"_H ), Cursor, false, DT_VCENTER );
					Cursor.x += RenderData->CharWidth;
				}

				Cursor.x = Original.x + RenderData->Columns.Formatted;
				{
					String = Helpers::GetFormattedData( Field.Type, reinterpret_cast<void*>( BufferBaseAddress ) );
					FontManager::AddSelectableText( String, 16.f, Themes::Manager::GetColor( "MemoryGrid_Formatted"_H ), Cursor, false, DT_VCENTER );
				}

				Cursor.x = Original.x;
				Cursor.y += LINE_HEIGHT / 2.f;
			}

			void RenderRegular( Vector2f& Cursor, const Vector2f Original, Memory::Info* Template, Memory::Field* Field, const LineData& Data, RenderParameters* RenderData )
			{
				std::string& PointerName = Field->Name;
				std::string  ClassName   = _( "<UNNAMED CLASS>" );
				if ( Data.Embedded.Child && Data.Embedded.Child->GetBackingData() )
				{
					ClassName = _( "<" ) + Data.Embedded.Child->GetBackingData()->Label + _( ">" );
				}

				auto OnTextChange = [&, SourceIndex = Data.SourceIndex] ( const std::string& NewLabel, const bool WasCompleted )
				{
					if ( SourceIndex >= 0 && WasCompleted ) Template->Fields[ SourceIndex ].Name = NewLabel;
				};

				auto RenderToken = [&] ( std::string& Text, const uint32_t Color, const bool Editable = false )
				{
					if ( Editable ) FontManager::AddModifiableText( Text, 16.f, Color, Cursor, false, DT_VCENTER, 0, OnTextChange );
					else FontManager::AddSelectableText( Text, 16.f, Color, Cursor, false, DT_VCENTER );
				};

				Cursor.x             = Original.x + RenderData->Columns.NamedTypeOrLabel;
				std::string TypeName = Field->Type != T_rawPointer && Data.Embedded.Child ? ClassName : Globals::GetDataStructureName( Field->Type );
				RenderToken( TypeName, Themes::Manager::GetColor( "MemoryGrid_Char"_H ) );

				Cursor.x = Original.x + RenderData->Columns.HexOrLabel;
				RenderToken( PointerName, Themes::Manager::GetColor( "MemoryGrid_Hex"_H ), true );

				Cursor.x = Original.x + RenderData->Columns.Formatted;

				if ( Data.Embedded.Child )
				{
					if ( Field->Type == T_pClass || Field->Type == T_rawPointer )
					{
						const uintptr_t BufferBaseAddress = reinterpret_cast<uintptr_t>( RenderData->Buffer ) + Field->Offset;
						const uintptr_t VA                = *reinterpret_cast<uintptr_t*>( BufferBaseAddress );
						const bool      IsValidPointer    = Process::IsValidPointer( RenderData->PID, VA );

						if ( !IsValidPointer )
						{
							const std::string String = VA == 0
								                           ? std::format( "\xF3\xB0\x91\x83\xEF\x91\xA7 nullptr (INVALID POINTER)" )
								                           : std::format( "\xF3\xB0\x91\x83\xEF\x91\xA7 0x{:X} (INACCESSIBLE POINTER)", VA );
							FontManager::AddSelectableText( String, 16.f, Themes::Manager::GetColor( "MemoryGrid_InvalidPointer"_H ), Cursor, false, DT_VCENTER );
						}
						else
						{
							Process::PointerData PointerData = {};
							Process::ResolvePointer( RenderData->PID, VA, &PointerData );
							const std::string String = Helpers::FormatPointer( &PointerData, VA );
							FontManager::AddSelectableText( String, 16.f, Themes::Manager::GetColor( "MemoryGrid_Pointer"_H ), Cursor, false, DT_VCENTER );
						}
					}
				}
				else
				{
					const uintptr_t BufferBaseAddress = reinterpret_cast<uintptr_t>( RenderData->Buffer ) + Field->Offset;
					std::string     String            = Helpers::GetFormattedData( Field->Type, reinterpret_cast<void*>( BufferBaseAddress ) );

					if ( Field->Type == T_char_array || Field->Type == T_wide_array )
					{
						if ( Field->Size != String.length() )
						{
							Field->Size = String.length();
							// TODO: Use ChangeSize instead of ChangeOffset once T_wide_array is handled.
							RenderData->Owner->ChangeOffset( Field->Offset, T_char_array, Field->Size );
						}
					}

					RenderToken( String, Themes::Manager::GetColor( "MemoryGrid_Formatted"_H ), false );
				}

				Cursor.x = Original.x;
				Cursor.y += LINE_HEIGHT / 2.f;
			}
		}

		void RenderFunction( Vector2f& Cursor, const RenderParameters* RenderData, LineData& Data, size_t& i )
		{
			if ( !Data.Embedded.Disassembly.has_value() ) return; // TODO: Probably render an error?

			const Vector2f Origin               = Cursor;
			const auto&    [ Timestamp, Lines ] = Data.Embedded.Disassembly.value();

			for ( const auto& Instruction : Lines )
			{
				Cursor.x = Origin.x;
				i++;

				auto                   Space = std::ranges::find( Instruction, ' ' );
				const std::string_view Mnemonic( Instruction.begin(), Space );
				const std::string_view Operands( Space == Instruction.end() ? Instruction.end() : Space + 1, Instruction.end() );

				if ( i % 2 == 0 )
				{
					const uint32_t Color = Themes::Manager::GetColor( "MemoryGrid_ColorA"_H );
					Graphics::AddFilledRectangle( Vector2f( 1.f, Cursor.y ), RenderData->WindowSize.x - 2.f, LINE_HEIGHT, Color );
				}

				Cursor.y += LINE_HEIGHT / 2.f;
				FontManager::AddSelectableText( Mnemonic, 16.f, Themes::Manager::GetColor( "MemoryGrid_Offset"_H ), Cursor, false, DT_VCENTER );
				Cursor.x = Origin.x + RenderData->Columns.Address;
				FontManager::AddSelectableText( Operands, 16.f, Themes::Manager::GetColor( "MemoryGrid_Hex"_H ), Cursor, false, DT_VCENTER );
				Cursor.y += LINE_HEIGHT / 2.f;
			}

			Cursor.x = Origin.x;
		}

		void RenderLine( Vector2f& Cursor, RenderParameters* RenderData, LineData& Data, Memory::Info* Template, Memory::Field* Field, size_t& i, const bool IsSelected )
		{
			const Vector2f Original = Cursor;
			i++;

			if ( i % 2 == 0 )
			{
				const uint32_t Color = Themes::Manager::GetColor( "MemoryGrid_ColorA"_H );
				Graphics::AddFilledRectangle( Vector2f( 1.f, Cursor.y ), RenderData->WindowSize.x - 2.f, LINE_HEIGHT, Color );
			}

			if ( IsSelected )
			{
				const uint32_t Color = Themes::Manager::GetColor( "MemoryGrid_Selected"_H );
				Graphics::AddFilledRectangle( Vector2f( 1.f, Cursor.y ), RenderData->WindowSize.x - 2.f, LINE_HEIGHT, Color );
			}

			Cursor.y += LINE_HEIGHT / 2.f;

			{
				const std::string String = std::format( "{:05X}", Field->Offset );
				FontManager::AddSelectableText( String, 16.f, Themes::Manager::GetColor( "MemoryGrid_Offset"_H ), Cursor, true, DT_VCENTER );
			}

			{
				Cursor.x                 = Original.x + RenderData->Columns.Address;
				const std::string String = std::format( "0x{:X}", RenderData->BaseAddress + Field->Offset );
				FontManager::AddSelectableText( String, 16.f, Themes::Manager::GetColor( "MemoryGrid_VirtualAddress"_H ), Cursor, true, DT_VCENTER );
			}

			if ( Field->IsPadding() )
			{
				RenderPadding( Cursor, Original, Template, Data, RenderData );
				return;
			}

			RenderRegular( Cursor, Original, Template, Field, Data, RenderData );
		}
	}

	void Grid::OnAttachProcess( const uint32_t PID, ProcessStatus Status )
	{
		// Tabs.clear();

		// TODO: Notifications, also consider adding a unique identifier to PEB/ProcessBase to delete them on re-attach?
		// This doesn't really align with our ProcessManager though, the idea at the start was to be able to be connected to multiple processes at once...
		const uintptr_t PEB = Process::GetProcessPEB( PID );
		NewTab( _( "ProcessBase" ), PID, Process::GetProcessBase( PID ), 1024 );
		NewStructTab( _( "PEB" ), PID, PEB, _( "PEB" ) );
	}

	void Grid::NewStructTab( const std::string& Label, uint32_t PID, uintptr_t Address, const std::string_view TemplateName, std::optional<size_t> Size )
	{
		const bool                    IsNonBacked = TemplateName == _( "RESERVED_EMPTY" );
		std::shared_ptr<Memory::Info> BackingData = nullptr;
		size_t                        ActualSize;

		if ( IsNonBacked )
		{
			BackingData = ClassManager::InstantiateNonBacked( Size.value_or( 64 ) );
		}
		else
		{
			BackingData = ClassManager::GetInstanceFromName( TemplateName );
		}

		ActualSize = BackingData->Size;

		auto Class           = std::make_unique<ClassInstance>( Address, ActualSize, PID );
		Class->RootData.View = std::make_unique<View>( Class.get(), Class->RootData.Node, 0 );

		View* Tab = Class->RootData.View.get();

		Objects::Button Button      = {};
		Objects::Button CloseButton = {};

		Tab->Buttons.TabIDX   = Button.IDX;
		Tab->Buttons.CloseIDX = CloseButton.IDX;
		Tab->SetBackingData( BackingData );

		Button.Callback = &Grid::OnButtonCallback;
		Button.Type     = Objects::ButtonType::Rectangle;
		Button.Label    = Label.empty() ? std::format( "Instance_{:X}", Tab->IDX ) : Label;

		CloseButton.Callback = &Grid::OnButtonCallback;
		CloseButton.Type     = Objects::ButtonType::Circle;
		CloseButton.Label    = _( "\xEE\xA9\xB6" );
		CloseButton.FontSize = 6.f;

		ActiveTabIDX = Tab->IDX;

		Buttons.emplace_back( Button, CloseButton );
		Tabs.push_back( std::move( Class ) );
	}

	void Grid::NewTab( const std::string& Label, const uint32_t PID, const uintptr_t Address, size_t Size )
	{
		NewStructTab( Label, PID, Address, _( "RESERVED_EMPTY" ), Size );
	}

	void Grid::Present( Vector2f& Cursor, const Vector2f AvailableSize, float Opacity )
	{
		Helpers::ReadConfig(); // TODO: We're calling this too much, consider making an Event fired by Config Popup closed?

		const Vector2f TitleBarSize = Titlebar::GetSize();
		const float    Padding      = TitleBarSize.y * 0.5f;

		GridData.Origin = Cursor;
		GridData.Size   = AvailableSize;

		const auto* ActiveTab = GetActiveTab();

		if ( !ActiveTab && !Tabs.empty() )
		{
			ActiveTabIDX = Tabs.back()->RootData.View->IDX;
			ActiveTab    = GetActiveTab();
		}

		if ( !ActiveTab )
		{
			// TODO: Render a splash screen here
			return;
		}

#pragma push_macro("max")
#undef max
		const float Tallest = std::ranges::max( Buttons, {},
		                                        [] ( TabButton& B )
		                                        {
			                                        B.TabSelection.Resize();
			                                        return B.TabSelection.Size.y;
		                                        }
		                                      ).TabSelection.Size.y;
#pragma pop_macro("max")

		Graphics::AddFilledRectangle( Cursor, AvailableSize.x, Tallest, Themes::Manager::GetColor( "MemoryGrid_TabBackground"_H ) );

		const auto  BorderColor     = Themes::Manager::GetColor( "Global_Border"_H );
		const float DelimitorHeight = Cursor.y + Tallest - 1.f;

		{
			const Vector2f Start{ Cursor.x, DelimitorHeight };
			const Vector2f End{ Start.x + AvailableSize.x, DelimitorHeight };
			Graphics::AddLine( Start, End, BorderColor, 1.f );
		}

		auto TargetButtonIDX = ActiveTab->RootData.View->Buttons.TabIDX;
		std::ranges::for_each( Buttons, [&] ( auto& ButtonPair ) { ButtonPair.Render( Cursor, Tallest, TargetButtonIDX ); } );

		Cursor.x = GridData.Origin.x + Padding * 2.f;
		Cursor.y += Tallest;

		{
			size_t y = 0ull;
			ActiveTab->RootData.View->Render( Cursor, y );
		}

		Cursor = GridData.Origin;
	}

	bool Grid::WndProc( Vector2f Position, uint32_t Message, WPARAM wParam, LPARAM lParam )
	{
		const auto* ActiveTab = GetActiveTab();

		if ( ActiveTab )
		{
			const Vector2f LastTabOrigin = ActiveTab->RootData.View->LastOrigin;
			const Vector2f LastGridSize  = Dissector::GetGridSize();

			const bool InBounds = Position.x > LastTabOrigin.x && Position.y > LastTabOrigin.y && Position.x < LastTabOrigin.x + LastGridSize.x && Position.y < LastTabOrigin.y + LastGridSize.y;

			if ( InBounds )
			{
				switch ( Message )
				{
				case WM_MOUSEWHEEL:
					ActiveTab->RootData.View->OnMouseWheel( wParam );
					return true;
				case WM_KEYDOWN:
					ActiveTab->RootData.View->OnKeyDown( wParam );
					return true;
				default:
					break;
				}
			}

			if ( ActiveTab->RootData.View->WndProc( Position, Message, wParam, lParam ) ) return true;
		}

		bool ReturnValue = false;

		std::ranges::for_each( Buttons, [&] ( auto& ButtonPair ) { ReturnValue |= ButtonPair.WndProc( Position, Message, wParam, lParam ); } );

		return ReturnValue;
	}

	void Grid::ChangeSelected( const DataStructureType NewType )
	{
		const auto* ActiveTab = GetActiveTab();

		if ( ActiveTab && ActiveTab->RootData.View )
		{
			ActiveTab->RootData.View->ChangeSelectedLines( NewType );
		}
	}

	void Grid::OnButtonCallback( const Objects::Button* Instance )
	{
		const uint32_t BID = Instance->IDX;

		const auto Iterator = std::ranges::find_if( Tabs, [BID] ( const std::unique_ptr<ClassInstance>& CI )
		{
			const View* Root = CI->RootData.View.get();
			return Root->Buttons.TabIDX == BID || Root->Buttons.CloseIDX == BID;
		} );

		if ( Iterator == Tabs.end() ) return;

		View* Root = Iterator->get()->RootData.View.get();

		if ( Root->Buttons.CloseIDX == BID )
		{
			const auto ButtonIterator = std::ranges::find_if( Buttons, [Root] ( const auto& P )
			{
				return P.TabClose.IDX == Root->Buttons.CloseIDX || P.TabSelection.IDX == Root->Buttons.TabIDX;
			} );

			if ( ButtonIterator != Buttons.end() ) Buttons.erase( ButtonIterator );

			// TODO: We might need to unschedule before destroying, investigate :p
			Tabs.erase( Iterator );
			return;
		}

		ActiveTabIDX = Root->IDX;
	}

	std::shared_ptr<Memory::Info> Grid::GetInfoForView( const View* View )
	{
		return View->GetBackingData();
	}

	void Grid::RenderView( View* View, Vector2f& Cursor, size_t& i )
	{
		if ( !View ) return;
		View->Render( Cursor, i );
	}
}
