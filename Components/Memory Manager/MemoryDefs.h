#pragma once
#include <chrono>
#include <mutex>
#include <print>
#include <queue>
#include <shared_mutex>
#include <unordered_map>
#include <vector>
#include <Windows.h>

#include "Components/Process Manager/Process.h"
#include "Definitions/Globals.h"

namespace Memory
{
	struct MemorySnapshot
	{
		std::vector<uint8_t> Buffer    = {};
		uint64_t             Timestamp = 0;
	};

	struct InstanceData
	{
		explicit InstanceData( const uintptr_t Address, const size_t Size, const uint32_t PID ) : Address( Address ), Size( Size ), PID( PID )
		{
			FrontBuffer.Buffer.resize( Size );
			BackBuffer.Buffer.resize( Size );
		}

		InstanceData*         Parent       = nullptr;
		std::optional<size_t> ParentOffset = std::nullopt;

		std::unordered_map<size_t, InstanceData*> Children;

		MemorySnapshot FrontBuffer;
		MemorySnapshot BackBuffer;

		bool IsEmbedded = false;

		uint32_t  GetPID() const { return PID.load( std::memory_order_acquire ); }
		uintptr_t GetAddress() const { return Address.load( std::memory_order_acquire ); }
		uintptr_t GetSize() const { return Size.load( std::memory_order_acquire ); }

		void SetAddress( const uintptr_t NewAddress ) { Address.store( NewAddress, std::memory_order_release ); }

		void SetSize( const uintptr_t NewSize )
		{
			FrontBuffer.Buffer.resize( NewSize );
			BackBuffer.Buffer.resize( NewSize );
			Size.store( NewSize, std::memory_order_release );
		}

	private:
		std::atomic<uintptr_t> Address;
		std::atomic<size_t>    Size;
		std::atomic<uint32_t>  PID;
	};

	class ReadGroup
	{
	public:
		InstanceData* AddRoot( uintptr_t Address, size_t Size, uint32_t PID )
		{
			std::unique_lock NodesLock( NodesMutex );

			return Nodes.emplace_back( std::make_unique<InstanceData>( Address, Size, PID ) ).get();
		}

		InstanceData* AddDependent( InstanceData* Parent, size_t ParentOffset, size_t Size )
		{
			std::unique_lock NodesLock( NodesMutex );

			auto* Node         = Nodes.emplace_back( std::make_unique<InstanceData>( 0, Size, Parent->GetPID() ) ).get();
			Node->Parent       = Parent;
			Node->ParentOffset = ParentOffset;
			Parent->Children.emplace( ParentOffset, Node );
			return Node;
		}

		[[nodiscard]] bool TryRead()
		{
			if ( Fresh.load( std::memory_order_relaxed ) ) return false;

			auto Recurse = [] ( this auto& Self, const HANDLE hProcess, InstanceData* Node )
			{
				const uintptr_t Address = Node->GetAddress();
				const size_t    Size    = Node->GetSize();

				if ( !Process::ReadMemory( hProcess, Address, Node->BackBuffer.Buffer.data(), Size ) )
				{
					memset( Node->BackBuffer.Buffer.data(), 0, Size );
					return;
				}

				Node->BackBuffer.Timestamp = CurrentTimestamp();

				for ( const auto& [ Offset, Child ] : Node->Children )
				{
					if ( !Node->BackBuffer.Buffer.data() ) continue;

					uintptr_t Resolved = 0;

					if ( Child->IsEmbedded )
					{
						Resolved = Address + Offset;
					}
					else
					{
						std::memcpy( &Resolved, Node->BackBuffer.Buffer.data() + Offset, sizeof( uintptr_t ) );
					}

					Child->SetAddress( Resolved );

					Self( hProcess, Child );
				}
			};

			std::shared_lock Lock( NodesMutex );

			for ( const auto& Node : Nodes )
			{
				const bool IsRoot = Node->Parent == nullptr;

				if ( !IsRoot ) continue;

				auto ReturnValue = Process::GetHandle( Node->GetPID() );

				if ( !ReturnValue.has_value() ) continue;

				const HANDLE hProcess = ReturnValue.value();

				Recurse( hProcess, Node.get() );
			}

			{
				const std::unique_lock Lock( Mutex, std::try_to_lock );
				if ( !Lock ) return false;

				for ( const auto& Node : Nodes ) std::swap( Node->FrontBuffer, Node->BackBuffer );
			}

			Fresh.store( true, std::memory_order_release );
			return true;
		}

		[[nodiscard]] bool TryConsume( const InstanceData* Node, MemorySnapshot& OutSnapshot )
		{
			if ( !Fresh.load( std::memory_order_relaxed ) ) return false;

			const std::unique_lock Lock( Mutex, std::try_to_lock );
			if ( !Lock ) return false;

			OutSnapshot.Buffer    = Node->FrontBuffer.Buffer;
			OutSnapshot.Timestamp = Node->FrontBuffer.Timestamp;
			return true;
		}

		void MarkConsumed()
		{
			Fresh.store( false, std::memory_order_release );
		}

		void SetSize( InstanceData* Node, const size_t NewSize )
		{
			std::unique_lock NodesLock( NodesMutex );

			Node->SetSize( NewSize );

			std::unique_lock Lock( Mutex );
			std::unique_lock BackLock( BackMutex );

			Node->FrontBuffer.Buffer.resize( NewSize );
			Node->BackBuffer.Buffer.resize( NewSize );
		}

		std::mutex Mutex;

	private:
		std::vector<std::unique_ptr<InstanceData>> Nodes;

		std::shared_mutex NodesMutex;
		std::mutex        BackMutex;
		std::atomic<bool> Fresh = false;

		static uint64_t CurrentTimestamp()
		{
			return std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::high_resolution_clock::now().time_since_epoch() ).count();
		}
	};

	class Manager
	{
	public:
		explicit Manager( const size_t ThreadCount = std::thread::hardware_concurrency() )
		{
			for ( auto i{ 0ul }; i < ThreadCount; i++ )
			{
				Workers.emplace_back( [this] { WorkerLoop(); } );
			}
		}

		~Manager()
		{
			StopSource.request_stop();
			CV.notify_all();
			for ( auto& W : Workers ) { W.join(); }
		}

		void Schedule( ReadGroup* Data )
		{
			{
				std::lock_guard Lock( QueueMutex );
				Queue.push( Data );
			}

			CV.notify_one();
		}

	private:
		void WorkerLoop()
		{
			while ( !StopSource.stop_requested() )
			{
				ReadGroup* Data;
				{
					std::unique_lock Lock( QueueMutex );
					CV.wait( Lock, [this] { return !Queue.empty() || StopSource.stop_requested(); } );
					if ( StopSource.stop_requested() ) break;
					Data = Queue.front();
					Queue.pop();
				}

				// TODO: Log failures
				if ( Data ) ( void )Data->TryRead();
			}
		}

		std::stop_source         StopSource;
		std::mutex               QueueMutex;
		std::condition_variable  CV;
		std::queue<ReadGroup*>   Queue;
		std::vector<std::thread> Workers;
	};
}
