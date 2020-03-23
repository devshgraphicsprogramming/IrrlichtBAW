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

uint32_t CQuantNormalCache::quantizeNormal2_10_10(const core::vectorSIMDf& normal)
{
	constexpr uint32_t quantizationBits = 10u;
	const auto xorflag = core::vectorSIMDu32((0x1u << quantizationBits) - 1u);
	const auto negativeMask = normal < core::vectorSIMDf(0.0f);

	auto found = normalCacheFor2_10_10_10Quant.find(mapToBarycentric(core::abs(normal)));

	if (found != normalCacheFor2_10_10_10Quant.end() && (found->first == mapToBarycentric(core::abs(normal))))
	{
		const uint32_t absVec = found->second;
		auto vec = core::vectorSIMDu32(absVec, absVec >> quantizationBits, absVec >> quantizationBits * 2)& xorflag;

		return restoreSign<uint32_t>(vec, xorflag, negativeMask, quantizationBits);
	}

	core::vectorSIMDf fit = findBestFit(quantizationBits, normal);

	auto absIntFit = core::vectorSIMDu32(core::abs(fit)) & xorflag;

	uint32_t absBestFit = absIntFit[0] | (absIntFit[1] << quantizationBits) | (absIntFit[2] << (quantizationBits * 2u));

	normalCacheFor2_10_10_10Quant.insert(std::make_pair(mapToBarycentric(core::abs(normal)), absBestFit));

	return restoreSign<uint32_t>(absIntFit, xorflag, negativeMask, quantizationBits);
}

uint32_t CQuantNormalCache::quantizeNormal8_8_8(const core::vectorSIMDf& normal)
{
	constexpr uint32_t quantizationBits = 8u;
	const auto xorflag = core::vectorSIMDu32((0x1u << quantizationBits) - 1u);
	auto negativeMask = normal < core::vectorSIMDf(0.f);

	auto found = normalCacheFor8_8_8Quant.find(mapToBarycentric(normal));
	if (found != normalCacheFor8_8_8Quant.end() && (found->first == mapToBarycentric(normal)))
	{
		const auto absVec = core::vectorSIMDu32(found->second.x, found->second.y, found->second.z);

		return restoreSign<uint32_t>(absVec, xorflag, negativeMask, quantizationBits);
	}

	core::vectorSIMDf fit = findBestFit(quantizationBits, normal);

	auto absIntFit = core::vectorSIMDu32(core::abs(fit)) & xorflag;

	Vector8u bestFit = { absIntFit[0], absIntFit[1], absIntFit[2] };

	normalCacheFor8_8_8Quant.insert(std::make_pair(mapToBarycentric(normal), bestFit));

	return restoreSign<uint32_t>(absIntFit, xorflag, negativeMask, quantizationBits);
}

uint64_t CQuantNormalCache::quantizeNormal16_16_16(const core::vectorSIMDf& normal)
{
	constexpr uint32_t quantizationBits = 10u;
	const auto xorflag = core::vectorSIMDu32((0x1u << quantizationBits) - 1u);
	auto negativeMask = normal < core::vectorSIMDf(0.f);

	auto found = normalCacheFor16_16_16Quant.find(mapToBarycentric(normal));
	if (found != normalCacheFor16_16_16Quant.end() && (found->first == mapToBarycentric(normal)))
	{
		const auto absVec = core::vectorSIMDu32(found->second.x, found->second.y, found->second.z);

		return restoreSign<uint64_t>(absVec, xorflag, negativeMask, quantizationBits);
	}

	core::vectorSIMDf fit = findBestFit(quantizationBits, normal);
	auto absIntFit = core::vectorSIMDu32(core::abs(fit)) & xorflag;

	Vector16u bestFit = { absIntFit[0], absIntFit[1], absIntFit[2] };

	normalCacheFor16_16_16Quant.insert(std::make_pair(mapToBarycentric(normal), bestFit));

	return restoreSign<uint64_t>(absIntFit, xorflag, negativeMask, quantizationBits);
}

void CQuantNormalCache::insertIntoCache2_10_10_10(const core::vectorSIMDf& key, const uint32_t quantizedNormal)
{

}
void CQuantNormalCache::insertIntoCache8_8_8(const core::vectorSIMDf& key, const uint32_t quantizedNormal)
{

}
void CQuantNormalCache::insertIntoCache16_16_16(const core::vectorSIMDf& key, const uint32_t quantizedNormal)
{

}


}
}