#ifndef __IRR_C_MTL_PIPELINE_METADATA_H_INCLUDED__
#define __IRR_C_MTL_PIPELINE_METADATA_H_INCLUDED__

#include "irr/asset/IPipelineMetadata.h"

namespace irr {
namespace asset
{

class CMTLPipelineMetadata final : public IPipelineMetadata
{
public:
    struct SMtl
    {
        std::string name;

        //Ka
        core::vector3df_SIMD ambient = core::vector3df_SIMD(1.f);
        //Kd
        core::vector3df_SIMD diffuse = core::vector3df_SIMD(1.f);
        //Ks
        core::vector3df_SIMD specular = core::vector3df_SIMD(1.f);
        //Ke
        core::vector3df_SIMD emissive = core::vector3df_SIMD(1.f);
        //Tf
        core::vector3df_SIMD transmissionFilter = core::vector3df_SIMD(1.f);
        //Ns, specular exponent in phong model
        float shininess = 32.f;
        //d
        float opacity = 1.f;
        //illum
        uint32_t illumModel = 0u;
        //-bm
        float bumpFactor = 1.f;

        //PBR
        //Ni, index of refraction
        float IoR = 1.6f;
        //Pr
        float roughness = 0.f;
        //Pm
        float metallic = 0.f;
        //Ps
        float sheen;
        //Pc
        float clearcoatThickness;
        //Pcr
        float clearcoatRoughness;
        //aniso
        float anisotropy = 0.f;
        //anisor
        float anisoRotation = 0.f;

        enum E_MAP_TYPE : uint32_t
        {
            EMP_AMBIENT,
            EMP_DIFFUSE,
            EMP_SPECULAR,
            EMP_EMISSIVE,
            EMP_SHININESS,
            EMP_OPACITY,
            EMP_BUMP,
            EMP_NORMAL,
            EMP_DISPLACEMENT,
            EMP_ROUGHNESS,
            EMP_METALLIC,
            EMP_SHEEN,
            EMP_REFL_POSX,
            EMP_REFL_NEGX,
            EMP_REFL_POSY,
            EMP_REFL_NEGY,
            EMP_REFL_POSZ,
            EMP_REFL_NEGZ,

            EMP_COUNT
        };

        //paths to image files, note that they're relative to the mtl file
        std::string maps[EMP_COUNT];
        //-clamp
        uint32_t clamp;
        static_assert(sizeof(clamp)*8ull >= EMP_COUNT, "SMtl::clamp is too small!");
    };

    CMTLPipelineMetadata(const SMtl& _mtl) : m_material(_mtl) {}
    CMTLPipelineMetadata(SMtl&& _mtl) : m_material(std::move(_mtl)) {}

    const SMtl& getMaterial() const { return m_material; }

    core::SRange<ShaderInputSemantic> getCommonRequiredInputs() override { return {nullptr, nullptr}; }
    const char* getLoaderName() const override { return "CGraphicsPipelineLoaderMTL"; } //?? i dont really understand the docs specifying what this function should return

private:
    const SMtl m_material;
};

}}

#endif