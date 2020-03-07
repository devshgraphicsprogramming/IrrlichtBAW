#ifndef __IRR_C_BRDF_EXPLORER_PIPELINE_METADATA_H_INCLUDED__
#define __IRR_C_BRDF_EXPLORER_PIPELINE_METADATA_H_INCLUDED__

#include "irr/asset/IPipelineMetadata.h"

namespace irr {
namespace asset
{

class CBRDFExplorerPipelineMetadata final : public IPipelineMetadata
{
public:
#include "irr/irrpack.h"
    //! This struct is compliant with GLSL's std140 and std430 layouts
    struct alignas(16) SBRDFExplorerMaterialParameters
    {
        core::matrix4SIMD uMVP = core::matrix4SIMD();
        core::vector3df uEmissive = core::vector3df();
        core::vector3df uAlbedo = core::vector3df();
        float uRoughness = 0;
        float uAnisotropy = 0;
        core::vector3df uRealIoR = core::vector3df();
        float uMetallic = 0;
        float uHeightScaleFactor = 0;
        core::vector3df uLightColor = core::vector3df();
        core::vector3df uLightPos = core::vector3df();
        core::vector3df uEyePos = core::vector3df();
        float uLightIntensity = 0;
        core::vector3df uImagIoR = core::vector3df();

        float padding[2];
    } PACK_STRUCT;
#include "irr/irrunpack.h" 
    //VS Intellisense shows error here because it think vectorSIMDf is 32 bytes, but it just Intellisense - it'll build anyway
    static_assert(sizeof(SBRDFExplorerMaterialParameters) == 176ull, "Something went wrong");

    enum E_MAP_TYPE : uint32_t
    {
        EMP_MVP = 15,
        EMP_EMISSIVE = 0,
        EMP_ALBEDO,
        EMP_ROUGHNESS,
        EMP_ANISOTROPY,
        EMP_REAL_IOR,
        EMP_METALLIC,
        EMP_HEIGHT_SCALE_FACTOR,
        EMP_LIGHT_COLOR,
        EMP_LIGHT_POSITION,
        EMP_EYE_POSITION,
        EMP_LIGHT_INTENSITY,
        EMP_IMAGE_IOR,
        
        EMP_COUNT
    };

    CBRDFExplorerPipelineMetadata(const SBRDFExplorerMaterialParameters& _params, core::smart_refctd_ptr<ICPUDescriptorSet>&& _ds3, core::smart_refctd_dynamic_array<ShaderInputSemantic>&& _inputs) :
        m_materialParams(_params), m_descriptorSet3(std::move(_ds3)), m_shaderInputs(std::move(_inputs)) {}

    const SBRDFExplorerMaterialParameters& getMaterialParams() const { return m_materialParams; }

    core::SRange<const ShaderInputSemantic> getCommonRequiredInputs() const override { return { m_shaderInputs->begin(), m_shaderInputs->end() }; }
    const char* getLoaderName() const override { return "???"; } // TODO
    ICPUDescriptorSet* getDescriptorSet() const { return m_descriptorSet3.get(); }

private:
    SBRDFExplorerMaterialParameters m_materialParams;
    core::smart_refctd_ptr<ICPUDescriptorSet> m_descriptorSet3;
    core::smart_refctd_dynamic_array<ShaderInputSemantic> m_shaderInputs;
};

}}

#endif // __IRR_C_BRDF_EXPLORER_PIPELINE_METADATA_H_INCLUDED__