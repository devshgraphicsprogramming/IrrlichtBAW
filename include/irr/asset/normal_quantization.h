// Copyright (C) 2009-2012 Christian Stehno
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __IRR_NORMAL_QUANTIZATION_H_INCLUDED__
#define __IRR_NORMAL_QUANTIZATION_H_INCLUDED__

#include "irr/core/math/glslFunctions.tcc"
#include <vector>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <iostream>

namespace irr
{
namespace asset
{
	struct VectorUV
	{
		float u;
		float v;

		inline bool operator==(const VectorUV& other) const
		{
			return (u == other.u && v == other.v);
		}
	};

	struct Vector16u
	{
		uint16_t x;
		uint16_t y;
		uint16_t z;
	};

	inline VectorUV mapToBarycentric(const core::vectorSIMDf& vec)
	{
		//normal to A = [1,0,0], B = [0,1,0], C = [0,0,1] triangle
		static const core::vector3df_SIMD n(0.577f, 0.577f, 0.577f);

		//point of intersection of vec and triangle - ( n dot A ) / (n dot vec) ) * vec
		const float r = 0.577f / core::dot(n, vec).x;
		
		//[0, 1, 0] + u * [1, -1, 0] + v * [0, -1, 1] = P, so u = Px and v = Pz
		return { r * vec.x, r * vec.z };
	}

	template <typename T>
	inline T restoreSign(const core::vectorSIMDu32& vec, const core::vectorSIMDu32& xorflag, const core::vector4db_SIMD& negativeMask, const uint32_t quantizationBits)
	{
		static_assert(std::is_same<T, uint32_t>::value || std::is_same<T, uint64_t>::value, "Type of returned value should be uint32_t or uint64_t.");

		auto restoredAsVec = core::vectorSIMDu32(vec) ^ core::mix(core::vectorSIMDu32(0u), xorflag, negativeMask);
		restoredAsVec = (restoredAsVec + core::mix(core::vectorSIMDu32(0u), core::vectorSIMDu32(1u), negativeMask)) & xorflag;

		T restoredAsInt = restoredAsVec[0] | (restoredAsVec[1] << quantizationBits) | (restoredAsVec[2] << (quantizationBits * 2u));

		return restoredAsInt;
	}

	//will remove later
	struct QuantNormalHash
	{
		size_t operator()(const core::vectorSIMDf& vec) const noexcept
		{
			static constexpr size_t primeNumber1 = 73856093;
			static constexpr size_t primeNumber2 = 19349663;

			return  ((static_cast<size_t>(vec.x * std::numeric_limits<size_t>::max())* primeNumber1) ^
			         (static_cast<size_t>(vec.y * std::numeric_limits<size_t>::max())* primeNumber2));
		}
	};

	struct QuantNormalHashUV
	{
		size_t operator()(const VectorUV& vec) const noexcept
		{
			static constexpr size_t primeNumber1 = 73856093;
			static constexpr size_t primeNumber2 = 19349663;

			return  ((static_cast<size_t>(vec.u * std::numeric_limits<size_t>::max())* primeNumber1) ^
				(static_cast<size_t>(vec.v * std::numeric_limits<size_t>::max())* primeNumber2));
		}
	};

	struct QuantNormalEqualTo
	{
		bool operator()(const core::vectorSIMDf& lval, const core::vectorSIMDf& rval) const noexcept
		{
			return (lval == rval).all();
		}

		bool operator()(const VectorUV& lval, const VectorUV& rval) const noexcept
		{
			return (lval.u == rval.u && lval.v == rval.v);
		}
	};

	// defined in CMeshManipulator.cpp
	extern core::unordered_map<VectorUV, uint32_t, QuantNormalHashUV, QuantNormalEqualTo> normalCacheFor2_10_10_10QuantUm;
	extern core::unordered_map<VectorUV, Vector16u, QuantNormalHashUV, QuantNormalEqualTo> normalCacheFor8_8_8QuantUm;
	extern core::unordered_map<VectorUV, Vector16u, QuantNormalHashUV, QuantNormalEqualTo> normalCacheFor16_16_16QuantUm;
	extern core::unordered_map<VectorUV, uint64_t, QuantNormalHashUV, QuantNormalEqualTo> normalCacheForHalfFloatQuantUm;

    inline core::vectorSIMDf findBestFit(const uint32_t& bits, const core::vectorSIMDf& normal)
    {
        core::vectorSIMDf fittingVector = normal;
        fittingVector.makeSafe3D();
        fittingVector = abs(fittingVector);

		// precise normalize
		auto vectorForDots = fittingVector.preciseDivision(length(fittingVector));

        float maxNormalComp;
        core::vectorSIMDf corners[4];
        core::vectorSIMDf floorOffset;
        if (fittingVector.X>fittingVector.Y)
        {
            maxNormalComp = fittingVector.X;
            corners[1].set(0,1,0);
            corners[2].set(0,0,1);
            corners[3].set(0,1,1);
            floorOffset.set(0.499f,0.f,0.f);
        }
        else
        {
            maxNormalComp = fittingVector.Y;
            corners[1].set(1,0,0);
            corners[2].set(0,0,1);
            corners[3].set(1,0,1);
            floorOffset.set(0.f,0.499f,0.f);
        }
        //second round
        if (fittingVector.Z>maxNormalComp)
        {
            maxNormalComp = fittingVector.Z;
            corners[1].set(1,0,0);
            corners[2].set(0,1,0);
            corners[3].set(1,1,0);
            floorOffset.set(0.f,0.f,0.499f);
        }
		
		//max component of 3d normal cannot be less than sqrt(1/3)
        if (maxNormalComp<=0.577f) //max component of 3d normal cannot be less than sqrt(1/3)
		{
			_IRR_DEBUG_BREAK_IF(true);
            return core::vectorSIMDf(0.f);
		}


        fittingVector = fittingVector.preciseDivision(core::vectorSIMDf(maxNormalComp));


        const uint32_t cubeHalfSize = (0x1u<<(bits-1u))-1u;
		const core::vectorSIMDf cubeHalfSize3D = core::vectorSIMDf(cubeHalfSize);
		core::vectorSIMDf bestFit;
		float closestTo1 = -1.f;
		auto evaluateFit = [&](const core::vectorSIMDf& newFit) -> void
		{
			auto newFitLen = core::length(newFit);
			auto dp = core::dot<core::vectorSIMDf>(newFit,vectorForDots).preciseDivision(newFitLen);
			if (dp[0] > closestTo1)
			{
				closestTo1 = dp[0];
				bestFit = newFit;
			}
		};
		for (uint32_t n=cubeHalfSize; n>0u; n--)
		{
            //we'd use float addition in the interest of speed, to increment the loop
            //but adding a small number to a large one loses precision, so multiplication preferrable
            core::vectorSIMDf bottomFit = core::floor(fittingVector*float(n)+floorOffset);
			for (uint32_t i=0u; i<4u; i++)
			{
				core::vectorSIMDf bottomFitTmp = bottomFit;
				if (i)
					bottomFitTmp += corners[i];
                if ((bottomFitTmp>cubeHalfSize3D).any())
					continue;
				evaluateFit(bottomFitTmp);
			}
		}

		return bestFit;
    }

	inline uint32_t quantizeNormal2_10_10_10(const core::vectorSIMDf &normal)
	{
		constexpr uint32_t quantizationBits = 10u;
		const auto xorflag = core::vectorSIMDu32((0x1u << quantizationBits) - 1u);
		const auto negativeMask = normal < core::vectorSIMDf(0.0f);

		auto found = normalCacheFor2_10_10_10QuantUm.find(mapToBarycentric(core::abs(normal)));

		if (found != normalCacheFor2_10_10_10QuantUm.end() && (found->first == mapToBarycentric(core::abs(normal))))
		{
			const uint32_t absVec = found->second;

			auto vec = core::vectorSIMDu32(absVec, absVec >> quantizationBits, absVec >> quantizationBits * 2) & xorflag;

			return restoreSign<uint32_t>(vec, xorflag, negativeMask, quantizationBits);
		}

        core::vectorSIMDf fit = findBestFit(quantizationBits, normal);
		
		auto absIntFit = core::vectorSIMDu32(core::abs(fit)) & xorflag;

        uint32_t absBestFit = absIntFit[0]|(absIntFit[1]<<quantizationBits)|(absIntFit[2]<<(quantizationBits*2u));

		normalCacheFor2_10_10_10QuantUm.insert(std::make_pair(mapToBarycentric(core::abs(normal)), absBestFit));

	    return restoreSign<uint32_t>(absIntFit, xorflag, negativeMask, quantizationBits);
	}

	inline uint32_t quantizeNormal888(const core::vectorSIMDf &normal)
	{
		constexpr uint32_t quantizationBits = 8u;
		const auto xorflag = core::vectorSIMDu32((0x1u << quantizationBits) - 1u);
		auto negativeMask = normal < core::vectorSIMDf(0.f);

		auto found = normalCacheFor8_8_8QuantUm.find(mapToBarycentric(normal));
		if (found != normalCacheFor8_8_8QuantUm.end() && (found->first == mapToBarycentric(normal)))
		{
			const auto absVec = core::vectorSIMDu32(found->second.x, found->second.y, found->second.z);
			return restoreSign<uint32_t>(absVec, xorflag, negativeMask, quantizationBits);
		}

		core::vectorSIMDf fit = findBestFit(quantizationBits, normal);

		auto absIntFit = core::vectorSIMDu32(core::abs(fit)) & xorflag;
        
		Vector16u bestFit = { absIntFit[0], absIntFit[1], absIntFit[2] };

		normalCacheFor8_8_8QuantUm.insert(std::make_pair(mapToBarycentric(normal), bestFit));

	    return restoreSign<uint32_t>(absIntFit, xorflag, negativeMask, quantizationBits);
	}

	inline uint64_t quantizeNormal16_16_16(const core::vectorSIMDf& normal)
	{
		constexpr uint32_t quantizationBits = 10u;
		const auto xorflag = core::vectorSIMDu32((0x1u << quantizationBits) - 1u);
		auto negativeMask = normal < core::vectorSIMDf(0.f);

		auto found = normalCacheFor16_16_16QuantUm.find(mapToBarycentric(normal));
		if (found != normalCacheFor16_16_16QuantUm.end() && (found->first == mapToBarycentric(normal)))
		{
			const auto absVec = core::vectorSIMDu32(found->second.x, found->second.y, found->second.z);
			return restoreSign<uint64_t>(absVec, xorflag, negativeMask, quantizationBits);
		}
		
        core::vectorSIMDf fit = findBestFit(quantizationBits, normal);
		auto absIntFit = core::vectorSIMDu32(core::abs(fit)) & xorflag;

		Vector16u bestFit = { absIntFit[0], absIntFit[1], absIntFit[2] };

		normalCacheFor16_16_16QuantUm.insert(std::make_pair(mapToBarycentric(normal), bestFit));

		return restoreSign<uint64_t>(absIntFit, xorflag, negativeMask, quantizationBits);
	}

	inline uint64_t quantizeNormalHalfFloat(const core::vectorSIMDf& normal)
	{
		//won't work

		auto found = normalCacheForHalfFloatQuantUm.find(mapToBarycentric(normal));
		if (found != normalCacheForHalfFloatQuantUm.end() && (found->first == mapToBarycentric(normal)))
		{
			return found->second;
		}

		uint16_t bestFit[4] {
			core::Float16Compressor::compress(normal.x),
			core::Float16Compressor::compress(normal.y),
			core::Float16Compressor::compress(normal.z),
			0u
		};

		
		normalCacheForHalfFloatQuantUm.insert(std::make_pair(mapToBarycentric(normal), reinterpret_cast<uint64_t>(bestFit)));

		return *reinterpret_cast<uint64_t*>(bestFit);
	}

} // end namespace scene
} // end namespace irr


#endif
