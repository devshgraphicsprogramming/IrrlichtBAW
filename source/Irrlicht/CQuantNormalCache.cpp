#include "CQuantNormalCache.h"

namespace irr
{
namespace asset
{

core::vectorSIMDf CQuantNormalCache::findBestFit(const uint32_t& bits, const core::vectorSIMDf& normal) const
{
	core::vectorSIMDf fittingVector = normal;
	fittingVector.makeSafe3D();
	fittingVector = abs(fittingVector);

	// precise normalize
	auto vectorForDots = fittingVector.preciseDivision(length(fittingVector));

	float maxNormalComp;
	core::vectorSIMDf corners[4];
	core::vectorSIMDf floorOffset;
	if (fittingVector.X > fittingVector.Y)
	{
		maxNormalComp = fittingVector.X;
		corners[1].set(0, 1, 0);
		corners[2].set(0, 0, 1);
		corners[3].set(0, 1, 1);
		floorOffset.set(0.499f, 0.f, 0.f);
	}
	else
	{
		maxNormalComp = fittingVector.Y;
		corners[1].set(1, 0, 0);
		corners[2].set(0, 0, 1);
		corners[3].set(1, 0, 1);
		floorOffset.set(0.f, 0.499f, 0.f);
	}
	//second round
	if (fittingVector.Z > maxNormalComp)
	{
		maxNormalComp = fittingVector.Z;
		corners[1].set(1, 0, 0);
		corners[2].set(0, 1, 0);
		corners[3].set(1, 1, 0);
		floorOffset.set(0.f, 0.f, 0.499f);
	}

	//max component of 3d normal cannot be less than sqrt(1/3)
	if (maxNormalComp <= 0.577f) //max component of 3d normal cannot be less than sqrt(1/3)
	{
		_IRR_DEBUG_BREAK_IF(true);
		return core::vectorSIMDf(0.f);
	}


	fittingVector = fittingVector.preciseDivision(core::vectorSIMDf(maxNormalComp));


	const uint32_t cubeHalfSize = (0x1u << (bits - 1u)) - 1u;
	const core::vectorSIMDf cubeHalfSize3D = core::vectorSIMDf(cubeHalfSize);
	core::vectorSIMDf bestFit;
	float closestTo1 = -1.f;
	auto evaluateFit = [&](const core::vectorSIMDf& newFit) -> void
	{
		auto newFitLen = core::length(newFit);
		auto dp = core::dot<core::vectorSIMDf>(newFit, vectorForDots).preciseDivision(newFitLen);
		if (dp[0] > closestTo1)
		{
			closestTo1 = dp[0];
			bestFit = newFit;
		}
	};
	for (uint32_t n = cubeHalfSize; n > 0u; n--)
	{
		//we'd use float addition in the interest of speed, to increment the loop
		//but adding a small number to a large one loses precision, so multiplication preferrable
		core::vectorSIMDf bottomFit = core::floor(fittingVector * float(n) + floorOffset);
		for (uint32_t i = 0u; i < 4u; i++)
		{
			core::vectorSIMDf bottomFitTmp = bottomFit;
			if (i)
				bottomFitTmp += corners[i];
			if ((bottomFitTmp > cubeHalfSize3D).any())
				continue;
			evaluateFit(bottomFitTmp);
		}
	}

	return bestFit;
}

void CQuantNormalCache::insertIntoCache2_10_10_10(const VectorUV key, const uint32_t quantizedNormal)
{
	normalCacheFor2_10_10_10Quant.insert(std::make_pair(key, quantizedNormal));
}

void CQuantNormalCache::insertIntoCache8_8_8(const VectorUV key, const Vector8u quantizedNormal)
{
	normalCacheFor8_8_8Quant.insert(std::make_pair(key, quantizedNormal));
}
void CQuantNormalCache::insertIntoCache16_16_16(const VectorUV key, const Vector16u quantizedNormal)
{
	normalCacheFor16_16_16Quant.insert(std::make_pair(key, quantizedNormal));
}

bool CQuantNormalCache::loadNormalQuantCacheFromBuffer(E_QUANT_NORM_CACHE_TYPE type, SBufferRange<ICPUBuffer>& buffer, CQuantNormalCache& quantNormalCache)
{
	const uint64_t bufferSize = buffer.buffer.get()->getSize();
	const uint64_t offset = buffer.offset;
	const size_t cacheElementSize = quantNormalCache.getCacheElementSize(type);

	uint8_t* buffPointer = static_cast<uint8_t*>(buffer.buffer.get()->getPointer());
	const uint8_t* const bufferRangeEnd = buffPointer + offset + buffer.size;

	if (bufferRangeEnd > buffPointer + bufferSize)
	{
		os::Printer::log("cannot read from this buffer - invalid range", ELL_ERROR);
		return false;
	}

	size_t quantVecSize = 0u;
	switch (type)
	{
	case E_QUANT_NORM_CACHE_TYPE::Q_2_10_10_10:
		quantVecSize = sizeof(uint32_t);
		break;
	case E_QUANT_NORM_CACHE_TYPE::Q_8_8_8:
		quantVecSize = sizeof(CQuantNormalCache::Vector8u);
		break;
	case E_QUANT_NORM_CACHE_TYPE::Q_16_16_16:
		quantVecSize = sizeof(CQuantNormalCache::Vector16u);
		break;
	}

	buffPointer += offset;
	while (buffPointer < bufferRangeEnd)
	{
		CQuantNormalCache::VectorUV key{ *reinterpret_cast<float*>(buffPointer),* reinterpret_cast<float*>(buffPointer + sizeof(float)) };
		buffPointer += sizeof(CQuantNormalCache::VectorUV);

		switch (type)
		{
		case E_QUANT_NORM_CACHE_TYPE::Q_2_10_10_10:
		{
			uint32_t vec;
			memcpy(&vec, buffPointer, quantVecSize);
			buffPointer += quantVecSize;

			quantNormalCache.insertIntoCache2_10_10_10(key, vec);
		}
		break;
		case E_QUANT_NORM_CACHE_TYPE::Q_8_8_8:
		{
			CQuantNormalCache::Vector8u vec;
			memcpy(&vec, buffPointer, quantVecSize);
			buffPointer += quantVecSize;

			quantNormalCache.insertIntoCache8_8_8(key, vec);
		}
		break;
		case E_QUANT_NORM_CACHE_TYPE::Q_16_16_16:
		{
			CQuantNormalCache::Vector16u vec;
			memcpy(&vec, buffPointer, quantVecSize);
			buffPointer += quantVecSize;

			quantNormalCache.insertIntoCache16_16_16(key, vec);
		}
		break;
		}
	}

	return true;
}

bool CQuantNormalCache::saveCacheToBuffer(E_QUANT_NORM_CACHE_TYPE type, SBufferBinding<ICPUBuffer>& buffer, CQuantNormalCache& quantNormalCache)
{
	const uint64_t bufferSize = buffer.buffer.get()->getSize();
	const uint64_t offset = buffer.offset;

	if (bufferSize + offset > quantNormalCache.getCacheSizeInBytes(type))
	{
		os::Printer::log("Cannot save cache to buffer - not enough space", ELL_ERROR);
		return false;
	}

	uint8_t* buffPointer = static_cast<uint8_t*>(buffer.buffer.get()->getPointer()) + offset;

	switch (type)
	{
	case E_QUANT_NORM_CACHE_TYPE::Q_2_10_10_10:
	{
		auto cache = quantNormalCache.getCache2_10_10_10();

		for (auto it = cache.begin(); it != cache.end(); it++)
		{
			memcpy(buffPointer, &(it->first), sizeof(CQuantNormalCache::VectorUV));
			buffPointer += sizeof(CQuantNormalCache::VectorUV);

			memcpy(buffPointer, &(it->second), sizeof(uint32_t));
			buffPointer += sizeof(uint32_t);
		}

		return true;
	}
	case E_QUANT_NORM_CACHE_TYPE::Q_8_8_8:
	{
		auto cache = quantNormalCache.getCache8_8_8();

		for (auto it = cache.begin(); it != cache.end(); it++)
		{
			memcpy(buffPointer, &(it->first), sizeof(CQuantNormalCache::VectorUV));
			buffPointer += sizeof(CQuantNormalCache::VectorUV);

			memcpy(buffPointer, &(it->second), sizeof(CQuantNormalCache::Vector8u));
			buffPointer += sizeof(CQuantNormalCache::Vector8u);
		}

		return true;
	}
	case E_QUANT_NORM_CACHE_TYPE::Q_16_16_16:
	{
		auto cache = quantNormalCache.getCache16_16_16();

		for (auto it = cache.begin(); it != cache.end(); it++)
		{
			memcpy(buffPointer, &(it->first), sizeof(CQuantNormalCache::VectorUV));
			buffPointer += sizeof(CQuantNormalCache::VectorUV);

			memcpy(buffPointer, &(it->second), sizeof(CQuantNormalCache::Vector16u));
			buffPointer += sizeof(CQuantNormalCache::Vector16u);
		}

		return true;
	}
	}

	return false;
}

}
}