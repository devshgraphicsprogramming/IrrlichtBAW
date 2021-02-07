#ifndef _EXTRA_CRAP_INCLUDED_
#define _EXTRA_CRAP_INCLUDED_

#include "irrlicht.h"

#include "../../ext/RadeonRays/RadeonRays.h"
// pesky leaking defines
#undef PI

#ifdef _IRR_BUILD_OPTIX_
#include "../../ext/OptiX/Manager.h"
#endif


class Renderer : public irr::core::IReferenceCounted, public irr::core::InterfaceUnmovable
{
    public:
		/**
		Plan for lighting:

		Option A) Path Guiding with Light Surface Sampling
			Do path guiding with spatio-directional (directions are implicit from light IDs) acceleration structure.
			Obviously the budgets for directions are low, so we might need to only track important lights and group them.

			Group lights should be made out of primitives like triangles, and we need a heuristic for picking a primitive from
			the light group surface that is less likely to be occluded or backfacing/at an angle as well as overall brighter.

			Then NEE uses path guiding, picks a nice light-group, then picks primitive and finally does spherical sampling on the primitive.
			4D samples needed

			BRDF sampling just samples the BRDF analytically, uses Closest Hit and proceeds classically.

			PROS: We know the point on the light we're supposed to hit, so we can do an AnyHit-only ray that is a lot cheaper.
			We only need a bit/boolean to know if we've hit the light, we know the light color up-front (or we don't need to fetch it in a dependent way).
			Env-Light becomes just a triangularized grid which is warped according to env-map CDF.
			CONS: Unused or bad quality samples.

		Option B) Path Guiding with Rejection Sampling
			Do path guiding with spatio-directional (directions are implicit from light IDs) acceleration structure, could be with complete disregard for light NEE.
			Then it would be better suited to integration of long light paths or VCM / BDPT with short eye-paths and long light paths. 

			We group lights into groups again, except that each group gets a Bounding Volume, either a Cube or a Sphere (whichever fits best =
			= gives better hit ratio with rejection sampling), only uniform scaling is allowed. BV could be a triangular convex hull also.
			
			Then NEE does perfect spherical sampling of the bounding volume. If hit ratio > 50% (or >73% for comparison against advanced surface heuristics) then it will
			perform better than Surface Sampling.
			3D-2D samples needed

			OPTIMIZATION: Could possibly shoot an AnyHit to the front of the convex hull volume, and then ClosestHit between the front and back.

			BRDF sampling just samples the BSDF analytically (or gives up and samples only the path-guiding AS), uses Closest Hit and proceeds classically.

			There's essentially 3 ways to generate samples: NEE with PGAS (discrete directions), NEE with PGAS (for all incoming lights), BSDF Analytical.

			PROS: Probably a much better sample generation strategy, might clean up a lot of noise.
			CONS: We don't know the point on the surface we are going to hit (could be any of multiple points for a concave light), so we cannot cast a fixed length ray.
			We need to cast a ray to the furthest back side of the Bounding Volume, and it cannot be an just an AnyHit ray, it needs to have a ClosestHit shader that will compare
			if the hit instanceID==lightGroupID. It can probably be optimized so that it uses a different shadow-only + light-compare SBT. So it may take a lot longer to compute a sample.

		CONCLUSION:
			We'll either be generating samples:
				A) From PGAS CDF
					No special light structure, just PGAS + GAS.
				B) Spherical sampling of Ellipsoids
					Ellipsoid List with a CDF for the whole list in PGAS, then analytical form ellipsoid scale, orientation and position
				C) Spherical sampling of Rectangles
					Paired Rectangle List (front + back so we don't generate samples on back-faces) with a CDF for the whole list in PGAS, then analytical
				D) Spherical sampling of Triangles
					Surface:
						Triangle group list in PGAS, then CDF for the triangles in the group, then analytical
					Rejection:
						Triangle List with a CDF for the whole list in PGAS, then analytical
		**/
		struct alignas(16) STriangleLightGroup
		{
			struct Triangle
			{
				irr::core::vectorSIMDf padding[3];
			} triangle;

			// we factor in the total surface area of the group into this
			// TODO: pack `strengthFactor` into RGB19E7 and turn this into Structure of Arrays layout (uint+uvec2)
			alignas(16) float strengthFactor[3];
			uint32_t triangleCDFOffset;
			uint32_t triangleCDFEnd;
		};
		struct alignas(16) SLight
		{
			struct CachedTransform
			{
				CachedTransform(const irr::core::matrix3x4SIMD& tform) : transform(tform)
				{
					auto tmp4 = irr::core::matrix4SIMD(transform.getSub3x3TransposeCofactors());
					transformCofactors = irr::core::transpose(tmp4).extractSub3x4();
				}

				irr::core::matrix3x4SIMD transform;
				irr::core::matrix3x4SIMD transformCofactors;
			};


			SLight() {}
			SLight(const SLight& other)
			{
				std::copy(other.strengthFactor,other.strengthFactor+3u,strengthFactor);
				std::copy(other.triangle.vertices, other.triangle.vertices+3u, triangle.vertices);
			}
			~SLight() {}

			inline SLight& operator=(SLight&& other)
			{
				std::swap_ranges(strengthFactor,strengthFactor+3u,other.strengthFactor);
				std::swap_ranges(other.triangle.vertices,other.triangle.vertices+3u,triangle.vertices);
				return *this;
			}

			void setFactor(const irr::core::vectorSIMDf& _strengthFactor)
			{
				for (auto i=0u; i<3u; i++)
					strengthFactor[i] = _strengthFactor[i];
			}

			//! This is according to Rec.709 colorspace
			inline float getFactorLuminosity() const
			{
				//! TODO: More color spaces!
				const float* colorSpaceLumaCoeffs = &rec709LumaCoeffs[0];

				float luma = strengthFactor[0] * colorSpaceLumaCoeffs[0];
				luma += strengthFactor[1] * colorSpaceLumaCoeffs[1];
				luma += strengthFactor[2] * colorSpaceLumaCoeffs[2];
				return luma;
			}

			inline float computeAreaUnderTransform(const CachedTransform& precompTform,irr::core::vectorSIMDf differentialElementCrossProduct) const
			{
				precompTform.transformCofactors.mulSub3x3WithNx1(differentialElementCrossProduct);
				return irr::core::length(differentialElementCrossProduct).x;
			}

			inline float computeFlux(float triangulizationArea) const // also known as lumens
			{
				const auto unitHemisphereArea = 2.f*irr::core::PI<float>();
				const auto unitSphereArea = 2.f*unitHemisphereArea;

				float lightFlux = unitHemisphereArea*getFactorLuminosity()*triangulizationArea;
				return lightFlux;
			}

			static inline SLight createFromTriangle(const irr::core::vectorSIMDf& _strengthFactor, const CachedTransform& precompTform, const irr::core::vectorSIMDf* v, float* outArea=nullptr)
			{
				SLight triLight;
				triLight.setFactor(_strengthFactor);

				float triangleArea = 0.5f*triLight.computeAreaUnderTransform(precompTform,irr::core::cross(v[1]-v[0],v[2]-v[0]));
				if (outArea)
					*outArea = triangleArea;

				for (auto k=0u; k<3u; k++)
					precompTform.transform.transformVect(triLight.triangle.vertices[k], v[k]);
				triLight.triangle.vertices[1] -= triLight.triangle.vertices[0];
				triLight.triangle.vertices[2] -= triLight.triangle.vertices[0];
				// always flip the handedness so normal points inwards (need negative normal for differential area optimization)
				if (precompTform.transform.getPseudoDeterminant().x>0.f)
					std::swap(triLight.triangle.vertices[2], triLight.triangle.vertices[1]);

				return triLight;
			}

			//! different lights use different measures of their strength (this already has the reciprocal of the light PDF factored in)
			alignas(16) float strengthFactor[3];
			uint32_t padding;
			struct Triangle
			{
				irr::core::vectorSIMDf vertices[3];
			} triangle;
		};
		static_assert(sizeof(SLight)==64u,"Can't keep alignment straight!");

		// No 8k yet, too many rays to store
		_IRR_STATIC_INLINE_CONSTEXPR uint32_t MaxResolution[2] = {7680/2,4320/2};


		Renderer(irr::video::IVideoDriver* _driver, irr::asset::IAssetManager* _assetManager, irr::scene::ISceneManager* _smgr, bool useDenoiser = false);

		void init(	const irr::asset::SAssetBundle& meshes,
					bool isCameraRightHanded,
					irr::core::smart_refctd_ptr<irr::asset::ICPUBuffer>&& sampleSequence,
					uint32_t rayBufferSize=(sizeof(::RadeonRays::ray)*2u+sizeof(uint32_t)*2u)*MaxResolution[0]*MaxResolution[1]); // 2 samples for MIS

		void deinit();

		void render();

		bool isRightHanded() { return m_rightHanded; }

		auto* getColorBuffer() { return m_colorBuffer; }

		const auto& getSceneBound() const { return sceneBound; }

		uint64_t getTotalSamplesComputed() const { return static_cast<uint64_t>(m_samplesComputed)*static_cast<uint64_t>(m_rayCount)/m_samplesPerDispatch; }


		_IRR_STATIC_INLINE_CONSTEXPR uint32_t MaxDimensions = 4u;

		//! TODO: More color spaces!
		_IRR_STATIC_INLINE_CONSTEXPR float rec709LumaCoeffs[] = { 0.2126f, 0.7152, 0.0722 };
    protected:
        ~Renderer();


        irr::video::IVideoDriver* m_driver;

		irr::asset::IAssetManager* m_assetManager;
		irr::scene::ISceneManager* m_smgr;

		irr::core::vector<std::array<irr::core::vector3df_SIMD,3> > m_precomputedGeodesic;

		irr::core::smart_refctd_ptr<irr::ext::RadeonRays::Manager> m_rrManager;

		bool m_rightHanded;

		irr::core::smart_refctd_ptr<irr::video::ITextureBufferObject> m_sampleSequence;
		irr::core::smart_refctd_ptr<irr::video::ITexture> m_scrambleTexture;

		irr::video::E_MATERIAL_TYPE nonInstanced;
		uint32_t m_raygenProgram, m_compostProgram;
		irr::core::smart_refctd_ptr<irr::video::ITexture> m_depth,m_albedo,m_normals,m_lightIndex,m_accumulation,m_tonemapOutput;
		irr::video::IFrameBuffer* m_colorBuffer,* m_gbuffer,* tmpTonemapBuffer;

		uint32_t m_maxSamples;
		uint32_t m_raygenWorkGroups[2];
		uint32_t m_resolveWorkGroups[2];
		uint32_t m_samplesPerDispatch;
		uint32_t m_samplesComputed;
		uint32_t m_rayCount;
		uint32_t m_framesDone;
		irr::core::smart_refctd_ptr<irr::video::IGPUBuffer> m_rayBuffer;
		irr::core::smart_refctd_ptr<irr::video::IGPUBuffer> m_intersectionBuffer;
		irr::core::smart_refctd_ptr<irr::video::IGPUBuffer> m_rayCountBuffer;
		std::pair<::RadeonRays::Buffer*,cl_mem> m_rayBufferAsRR;
		std::pair<::RadeonRays::Buffer*,cl_mem> m_intersectionBufferAsRR;
		std::pair<::RadeonRays::Buffer*,cl_mem> m_rayCountBufferAsRR;

		irr::core::vector<irr::core::smart_refctd_ptr<irr::scene::IMeshSceneNode> > nodes;
		irr::core::aabbox3df sceneBound;
		irr::ext::RadeonRays::Manager::MeshBufferRRShapeCache rrShapeCache;
		irr::ext::RadeonRays::Manager::MeshNodeRRInstanceCache rrInstances;

		irr::core::vectorSIMDf constantClearColor;
		uint32_t m_lightCount;
		irr::core::smart_refctd_ptr<irr::video::IGPUBuffer> m_lightCDFBuffer;
		irr::core::smart_refctd_ptr<irr::video::IGPUBuffer> m_lightBuffer;
		irr::core::smart_refctd_ptr<irr::video::IGPUBuffer> m_lightRadianceBuffer;

	#ifdef _IRR_BUILD_OPTIX_
		irr::core::smart_refctd_ptr<irr::ext::OptiX::Manager> m_optixManager;
		CUstream m_cudaStream;
		irr::core::smart_refctd_ptr<irr::ext::OptiX::IContext> m_optixContext;
		irr::core::smart_refctd_ptr<irr::ext::OptiX::IDenoiser> m_denoiser;
		OptixDenoiserSizes m_denoiserMemReqs;
		irr::cuda::CCUDAHandler::GraphicsAPIObjLink<irr::video::IGPUBuffer> m_denoiserInputBuffer,m_denoiserStateBuffer,m_denoisedBuffer,m_denoiserScratchBuffer;

		enum E_DENOISER_INPUT
		{
			EDI_COLOR,
			EDI_ALBEDO,
			EDI_NORMAL,
			EDI_COUNT
		};
		OptixImage2D m_denoiserOutput;
		OptixImage2D m_denoiserInputs[EDI_COUNT];
	#endif
};

#endif
