// Copyright (C) 2020- Mateusz 'DevSH' Kielan
// This file is part of the "IrrlichtBAW" engine.
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __IRR_COMMON_IMAGE_FILTER_KERNELS_H_INCLUDED__
#define __IRR_COMMON_IMAGE_FILTER_KERNELS_H_INCLUDED__


#include "irr/asset/filters/kernels/IImageFilterKernel.h"

#include <ratio>

namespace irr
{
namespace asset
{

class CFloatingPointOnlyImageFilterKernelBase
{
	public:
		using value_type = double;

		static inline bool validate(ICPUImage* inImage, ICPUImage* outImage)
		{
			const auto& inParams = inImage->getCreationParameters();
			return !isIntegerFormat(inParams.format);
		}
};

template<class Ratio=std::ratio<1,1> >
class CIsotropicSeparableImageFilterKernelBase
{
	public:
		_IRR_STATIC_INLINE_CONSTEXPR bool is_separable = true;
		const float isotropic_support = float(Ratio::num)/float(Ratio::den);
		const float symmetric_support[3] = { isotropic_support,isotropic_support,isotropic_support };
		const float positive_support[3] = { symmetric_support[0], symmetric_support[1], symmetric_support[2] };
		const float negative_support[3] = {-symmetric_support[0],-symmetric_support[1],-symmetric_support[2] };

	protected:
		inline bool inDomain(const core::vectorSIMDf& inPos)
		{
			return (abs(inPos)<core::vectorSIMDf(isotropic_support,isotropic_support,isotropic_support,FLT_MAX)).all();
		}
};


class CBoxImageFilterKernel : public CImageFilterKernel<CBoxImageFilterKernel>, public CIsotropicSeparableImageFilterKernelBase<std::ratio<1,2> >, public CFloatingPointOnlyImageFilterKernelBase
{
	public:
		inline float evaluate(const core::vectorSIMDf& inPos)
		{
			return inDomain(inPos) ? 1.0:0.0;
		}
};

class CTriangleImageFilterKernel : public CImageFilterKernel<CTriangleImageFilterKernel>, public CIsotropicSeparableImageFilterKernelBase<std::ratio<1,1> >, public CFloatingPointOnlyImageFilterKernelBase
{
	public:
		inline float evaluate(const core::vectorSIMDf& inPos)
		{
			if (inDomain(inPos))
			{
				auto hats = core::vectorSIMDf(1.f,1.f,1.f)-abs(inPos);
				return hats.x * hats.y * hats.z;
			}
			return 0.f;
		}
};

template<uint32_t support=3u>
class CKaiserImageFilterKernel : public CImageFilterKernel<CKaiserImageFilterKernel<support>>, public CIsotropicSeparableImageFilterKernelBase<std::ratio<support,1> >, public CFloatingPointOnlyImageFilterKernelBase
{
	public:
		const float alpha = 3.f;

		inline float evaluate(const core::vectorSIMDf& inPos)
		{
			if (inDomain(inPos))
			{
				const auto PI = core::PI<core::vectorSIMDf>();
				return core::sinc(inPos*PI)*core::KaiserWindow(inPos,core::vectorSIMDf(alpha),core::vectorSIMDf(isotropic_support))/PI;
			}
			return 0.f;
		}
};

template<class B=std::ratio<1,3>, class C=std::ratio<1,3> >
class CMitchellImageFilterKernel : public CImageFilterKernel<CMitchellImageFilterKernel<B,C> >, public CIsotropicSeparableImageFilterKernelBase<std::ratio<2,1> >, public CFloatingPointOnlyImageFilterKernelBase
{
	public:
		inline float evaluate(const core::vectorSIMDf& inPos)
		{
			if (inDomain(inPos))
				return core::mix(
									core::vectorSIMDf(p0)+radial*radial*(core::vectorSIMDf(p2)+radial*p3),
									core::vectorSIMDf(q0)+radial*(core::vectorSIMDf(q1)+radial*(core::vectorSIMDf(q2)+radial*q3)),
									radial>core::vectorSIMDf(1.f)
								);

			return 0.f;
		}

	protected:
		_IRR_STATIC_INLINE_CONSTEXPR float b = float(B::num)/float(B::den);
		_IRR_STATIC_INLINE_CONSTEXPR float c = float(C::num)/float(C::den);
		_IRR_STATIC_INLINE_CONSTEXPR float p0 = (6.0f - 2.0f * b) / 6.0f;
		_IRR_STATIC_INLINE_CONSTEXPR float p2 = (-18.0f + 12.0f * b + 6.0f * c) / 6.0f;
		_IRR_STATIC_INLINE_CONSTEXPR float p3 = (12.0f - 9.0f * b - 6.0f * c) / 6.0f;
		_IRR_STATIC_INLINE_CONSTEXPR float q0 = (8.0f * b + 24.0f * c) / 6.0f;
		_IRR_STATIC_INLINE_CONSTEXPR float q1 = (-12.0f * b - 48.0f * c) / 6.0f;
		_IRR_STATIC_INLINE_CONSTEXPR float q2 = (6.0f * b + 30.0f * c) / 6.0f;
		_IRR_STATIC_INLINE_CONSTEXPR float q3 = (-b - 6.0f * c) / 6.0f;
};
#undef IRR_ASSET_DECLARE_ISOTROPIC_FILTER

} // end namespace asset
} // end namespace irr

#endif