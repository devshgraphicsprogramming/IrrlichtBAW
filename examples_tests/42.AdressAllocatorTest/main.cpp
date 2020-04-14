#define _IRR_STATIC_LIB_
#include <irrlicht.h>
#include <random>
#include <cmath>

using namespace irr;
using namespace core;

/*
	If traits say address allocator supports arbitrary order free,
	then ETAT_CHOOSE_RANDOMLY is valid. Otherwise ETAT_CHOOSE_MOST_RECENTLY_ALLOCATED
*/

enum E_TRAITS_ALLOCATION_TYPE
{
	ETAT_CHOOSE_RANDOMLY,
	ETAT_CHOOSE_MOST_RECENTLY_ALLOCATED,
	ETAT_COUNT
};

#include <irr/irrpack.h>
struct AllocationCreationParameters
{
	AllocationCreationParameters() {}

	struct RandomAmountOf
	{
		size_t multi_alloc;				//! Specifies amount of adress to be allocated with certain choosen allocator
		size_t adressesToDeallocate;    //! Specifies amount of adress to be deallocated with certain choosen allocator. Must be less than all allocated and must pick number less than `traits::max_multi_free`, but we don't have `max_multi_free`
	} randomAmountOf;
	
} PACK_STRUCT;

struct Generator
{
	Generator(size_t minAllocations, size_t maxAllocatios, size_t maxSizeSquaredPerFrame, size_t maxAlignmentPerFrame, size_t minByteSizeForReservingMemory, size_t maxByteSizeForReservingMemory, size_t maxOffset, size_t maxAlignOffset, size_t maxBlockSize)
		: allocsPerFrameRange(minAllocations, maxAllocatios), sizePerFrameRange(1, maxSizeSquaredPerFrame), alignmentPerFrameRange(1, maxAlignmentPerFrame), memoryReservePerFrameRange(minByteSizeForReservingMemory, maxByteSizeForReservingMemory), offsetPerFrameRange(1, maxOffset), alignOffsetPerFrameRange(1, maxAlignOffset), blockSizePerFrameRange(1, maxBlockSize) {}

	std::mt19937 mersenneTwister;
	std::uniform_int_distribution<uint32_t> allocsPerFrameRange;			// (PerFrame::minAllocs, minAllocs::maxAllocs)
	std::uniform_int_distribution<uint32_t> sizePerFrameRange;				// (1, PerFrame::maxSizeSquared)
	std::uniform_int_distribution<uint32_t> alignmentPerFrameRange;			// (1, PerFrame::maxAlignment)

	std::uniform_int_distribution<uint32_t> memoryReservePerFrameRange;		// (PerFrame::minByteSizeForReservingMemory, PerFrame::maxByteSizeForReservingMemory)
	std::uniform_int_distribution<uint32_t> offsetPerFrameRange;			// (1, PerFrame::maxOffset)
	std::uniform_int_distribution<uint32_t> alignOffsetPerFrameRange;		// (1, PerFrame::maxAlignOffset)
	std::uniform_int_distribution<uint32_t> blockSizePerFrameRange;			// (1, PerFrame::maxBlockSize)
} PACK_STRUCT;

/*
	Settings
*/

namespace PerFrame
{
	constexpr size_t minAllocs = 10000u;
	constexpr size_t maxAllocs = 20000u;
	constexpr size_t maxSizeForSquaring = 1024u;
	constexpr size_t maxAlignment = 128;
	constexpr size_t minByteSizeForReservingMemory = 2048;			// 2kB
	constexpr size_t maxByteSizeForReservingMemory = 2147483648;	// 2GB
	constexpr size_t maxOffset = 512;
	constexpr size_t maxAlignOffset = 64;
	constexpr size_t maxBlockSize = 600;
}

class AllocatorHandler
{
	public:

		using PoolAdressAllocator = core::PoolAddressAllocatorST<uint32_t>;

		AllocatorHandler(AllocationCreationParameters& allocationCreationParameters)
			: numberGenerator(PerFrame::minAllocs, PerFrame::maxAllocs, PerFrame::maxSizeForSquaring, PerFrame::maxAlignment, PerFrame::minByteSizeForReservingMemory, PerFrame::maxByteSizeForReservingMemory, PerFrame::maxOffset, PerFrame::maxAlignOffset, PerFrame::maxBlockSize), creationParameters(allocationCreationParameters) {}

		struct EntryForFrameData
		{
			uint32_t outAddr[PerFrame::maxAllocs];
			uint32_t sizes[PerFrame::maxAllocs];
			uint32_t alignments[PerFrame::maxAllocs];
		} perFrameData;

		void executeAllocatorTest()
		{
			os::Printer::log("Executing Pool Allocator Test!", ELL_INFORMATION);

			for (size_t i = 0; i < creationParameters.randomAmountOf.multi_alloc; ++i)
				executeForFrame();
		}

	private:

		void executeForFrame()
		{
			auto allocationAmountForSingeFrameTest = numberGenerator.allocsPerFrameRange(numberGenerator.mersenneTwister);

			for (size_t i = 0; i < allocationAmountForSingeFrameTest; ++i)
			{
				perFrameData.outAddr[i] = video::StreamingTransientDataBufferST<uint32_t, PoolAdressAllocator>::invalid_address; // I don't get it, why does acutally everything in engine use it? What is the purpose for that?
				perFrameData.sizes[i] = numberGenerator.sizePerFrameRange(numberGenerator.mersenneTwister);
				perFrameData.alignments[i] = numberGenerator.alignmentPerFrameRange(numberGenerator.mersenneTwister);
			}

			uint32_t randMaxAlign = 8 * numberGenerator.alignmentPerFrameRange(numberGenerator.mersenneTwister),
			randAddressSpaceSize = numberGenerator.memoryReservePerFrameRange(numberGenerator.mersenneTwister), 
			randOffset = numberGenerator.offsetPerFrameRange(numberGenerator.mersenneTwister) / 8, 
			randAlignOffset = numberGenerator.alignOffsetPerFrameRange(numberGenerator.mersenneTwister) / 8, 
			randBlockSz = numberGenerator.blockSizePerFrameRange(numberGenerator.mersenneTwister) / 24;

			const auto reservedSize = PoolAdressAllocator::reserved_size(randMaxAlign, randAddressSpaceSize, randBlockSz); // 3rd parameter onward is custom for each address alloc type
			void* reservedSpace = _IRR_ALIGNED_MALLOC(reservedSize, _IRR_SIMD_ALIGNMENT);
			auto poolAllocator = PoolAdressAllocator(reservedSpace, randOffset, randAlignOffset, randMaxAlign, randAddressSpaceSize, randBlockSz);

			auto maxSize = core::address_allocator_traits<PoolAdressAllocator>::max_size(poolAllocator); // max size of what?

			// TODO I think it isn't all

			core::address_allocator_traits<PoolAdressAllocator>::multi_alloc_addr(poolAllocator, PerFrame::maxAllocs, perFrameData.outAddr, perFrameData.sizes, perFrameData.alignments);

			// TODO capturing states
			//record all successful alloc addresses to the `core::vector`
			 
			_IRR_ALIGNED_FREE(reservedSpace);
		}

		Generator numberGenerator;
		AllocationCreationParameters creationParameters;

} PACK_STRUCT;

#include <irr/irrunpack.h>

int main()
{
	AllocationCreationParameters creationParams;
	creationParams.randomAmountOf.multi_alloc = 1000;				// TODO
	creationParams.randomAmountOf.adressesToDeallocate = 1000;		// TODO

	AllocatorHandler handler(creationParams);
	handler.executeAllocatorTest();
}

/*
	Instructions:
*/

// random dealloc function
		// randomly decide how many calls to `multi_alloc`
			// randomly how many addresses we should deallocate (but obvs less than all allocated) NOTE: must pick number less than `traits::max_multi_free`
				// if traits say address allocator supports arbitrary order free, then choose randomly, else choose most recently allocated



	// function to test allocator
		// pick random alignment, rand buffer size (between 2kb and 2GB), random offset (less than buffer size), random alignment offset (less than alignment) and other parameters randomly

		// allocate reserved space (for allocator state)

		// create address allocator

		// randomly decide the number of iterations of allocation and reset
			// declare `core::vector` to hold allocated addresses and sizes

			// randomly decide how many `multi_allocs` to do
				// randomly decide how many allocs in a `multi_alloc` NOTE: must pick number less than `traits::max_multi_alloc`
					// randomly decide sizes (but always less than `address_allocator_traits::max_size`)
				// record all successful alloc addresses to the `core::vector`

				// run random dealloc function
			//

			// run random dealloc function

			// randomly choose between reset and freeing all `core::vector` elements
				// reset
			// ELSE
				// free everything with a series of multi_free
