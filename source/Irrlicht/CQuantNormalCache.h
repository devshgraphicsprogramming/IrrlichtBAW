#ifndef C_QUANT_NORMAL_CACHE_H_INCLUDED
#define C_QUANT_NORMAL_CACHE_H_INCLUDED

#include "irrlicht.h"

namespace irr 
{
namespace asset 
{

class CQuantNormalCache
{ 
public:
	uint32_t quantizeNormal2_10_10_10(const core::vectorSIMDf& normal);
	uint32_t quantizeNormal8_8_8(const core::vectorSIMDf& normal);
	uint64_t quantizeNormal16_16_16(const core::vectorSIMDf& normal);

	void insertIntoCache2_10_10_10(const core::vectorSIMDf& key, const uint32_t quantizedNormal);
	void insertIntoCache8_8_8(const core::vectorSIMDf& key, const uint32_t quantizedNormal);
	void insertIntoCache16_16_16(const core::vectorSIMDf& key, const uint64_t quantizedNormal);

private:
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

	struct Vector8u
	{
		uint8_t x;
		uint8_t y;
		uint8_t z;
	};

	struct QuantNormalHash
	{
		inline size_t operator()(const VectorUV& vec) const noexcept
		{
			static constexpr size_t primeNumber1 = 18446744073709551557;
			static constexpr size_t primeNumber2 = 4611686018427388273;

			return  ((static_cast<size_t>(static_cast<double>(vec.u)* std::numeric_limits<size_t>::max())* primeNumber1) ^
				(static_cast<size_t>(static_cast<double>(vec.v)* std::numeric_limits<size_t>::max())* primeNumber2));
		}
	};

	struct QuantNormalEqualTo
	{
		inline bool operator()(const core::vectorSIMDf& lval, const core::vectorSIMDf& rval) const noexcept
		{
			return (lval == rval).all();
		}

		inline bool operator()(const VectorUV& lval, const VectorUV& rval) const noexcept
		{
			return (lval.u == rval.u && lval.v == rval.v);
		}
	};

	inline VectorUV mapToBarycentric(const core::vectorSIMDf& vec) const
	{
		//normal to A = [1,0,0], B = [0,1,0], C = [0,0,1] triangle
		static const core::vector3df_SIMD n(0.577f, 0.577f, 0.577f);

		//point of intersection of vec and triangle - ( n dot A ) / (n dot vec) ) * vec
		const float r = 0.577f / core::dot(n, vec).x;

		//[0, 1, 0] + u * [1, -1, 0] + v * [0, -1, 1] = P, so u = Px and v = Pz
		return { r * vec.x, r * vec.z };
	}

	template <typename T>
	inline T restoreSign(const core::vectorSIMDu32& vec, const core::vectorSIMDu32& xorflag, const core::vector4db_SIMD& negativeMask, const uint32_t quantizationBits) const 
	{
		static_assert(std::is_same<T, uint32_t>::value || std::is_same<T, uint64_t>::value, "Type of returned value should be uint32_t or uint64_t.");

		auto restoredAsVec = core::vectorSIMDu32(vec) ^ core::mix(core::vectorSIMDu32(0u), xorflag, negativeMask);
		restoredAsVec = (restoredAsVec + core::mix(core::vectorSIMDu32(0u), core::vectorSIMDu32(1u), negativeMask)) & xorflag;

		T restoredAsInt = restoredAsVec[0] | (restoredAsVec[1] << quantizationBits) | (restoredAsVec[2] << (quantizationBits * 2u));

		return restoredAsInt;
	}

	core::vectorSIMDf findBestFit(const uint32_t& bits, const core::vectorSIMDf& normal) const;

private:
	core::unordered_map<VectorUV, uint32_t, QuantNormalHash, QuantNormalEqualTo> normalCacheFor2_10_10_10Quant;
	core::unordered_map<VectorUV, Vector8u, QuantNormalHash, QuantNormalEqualTo> normalCacheFor8_8_8Quant;
	core::unordered_map<VectorUV, Vector16u, QuantNormalHash, QuantNormalEqualTo> normalCacheFor16_16_16Quant;
};

}
}
#endif