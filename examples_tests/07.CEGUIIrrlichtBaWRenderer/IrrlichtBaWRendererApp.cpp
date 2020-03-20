/*

MIT License

Copyright (c) 2019 InnerPiece Technology Co., Ltd.
https://innerpiece.io

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include "IrrlichtBaWRendererApp.h"
#include "../../ext/CEGUI/ExtCEGUI.h"
#include <CEGUI/RendererModules/OpenGL/Texture.h>
#include "CIrrlichtBaWRendererPipelineMetadata.h"

#include "workaroundFunctions.h"
#include "irr/video/CDerivativeMapCreator.h"

using namespace irr;
using namespace asset;
using namespace core;

namespace
{
    class CShaderConstantSetCallback
    {
        const irr::IrrlichtBaWRendererApp::SGUIState& GUIState;
        const irr::IrrlichtBaWRendererApp::SLightAnimData& LightAnimData;
        scene::ICameraSceneNode* Camera;


    public:
        CShaderConstantSetCallback(scene::ICameraSceneNode* _camera, const irr::IrrlichtBaWRendererApp::SGUIState& _guiState, const irr::IrrlichtBaWRendererApp::SLightAnimData& _lightAnim) : Camera{ _camera }, GUIState{_guiState}, LightAnimData{_lightAnim} 
        {

        }

        // there must be a pipeline metadata and driver which will be setting buffers
        virtual void OnSetConstants(video::IVideoDriver* driver)
        {

            /*
            // vertex shader
            auto mvp = Camera->getConcatenatedMatrix();
            services->setShaderConstant(mvp.pointer(), uVP.location, uVP.type, 1u);

            // fragment shader
            services->setShaderConstant(&GUIState.Emissive.Color.X, uEmissive.location, uEmissive.type, 1u);
            if (GUIState.Albedo.SourceDropdown == irr::IrrlichtBaWRendererApp::EDS_CONSTANT)
                services->setShaderConstant(&GUIState.Albedo.ConstantColor.X, uAlbedo.location, uAlbedo.type, 1u);
            if (GUIState.Roughness.SourceDropdown == irr::IrrlichtBaWRendererApp::EDS_CONSTANT)
                services->setShaderConstant(&GUIState.Roughness.ConstValue1, uRoughness.location, uRoughness.type, 1u);
            if (GUIState.Roughness.SourceDropdown==irr::IrrlichtBaWRendererApp::EDS_CONSTANT && !GUIState.Roughness.IsIsotropic)
                services->setShaderConstant(&GUIState.Roughness.ConstValue2, uAnisotropy.location, uAnisotropy.type, 1u);
            else {
                const float aniso = 0.f;
                services->setShaderConstant(&aniso, uAnisotropy.location, uAnisotropy.type, 1u);
            }
            if (GUIState.RefractionIndex.SourceDropdown == irr::IrrlichtBaWRendererApp::EDS_CONSTANT)
                services->setShaderConstant(&GUIState.RefractionIndex.ConstantReal.X, uRealIoR.location, uRealIoR.type, 1u);
            services->setShaderConstant(&GUIState.RefractionIndex.ConstantImag.X, uImagIoR.location, uImagIoR.type, 1u);
            if (GUIState.Metallic.SourceDropdown == irr::IrrlichtBaWRendererApp::EDS_CONSTANT)
                services->setShaderConstant(&GUIState.Metallic.ConstValue, uMetallic.location, uMetallic.type, 1u);
            services->setShaderConstant(&GUIState.BumpMapping.Height, uHeightScaleFactor.location, uHeightScaleFactor.type, 1u);
            if (!GUIState.Light.Animated)
                services->setShaderConstant(&GUIState.Light.ConstantPosition.X, uLightPos.location, uLightPos.type, 1u);
            else
                services->setShaderConstant(&LightAnimData.Position.X, uLightPos.location, uLightPos.type, 1u);
            services->setShaderConstant(&GUIState.Light.Color, uLightColor.location, uLightColor.type, 1u);

            auto eyePos = Camera->getPosition();
            services->setShaderConstant(&eyePos.X, uEyePos.location, uEyePos.type, 1u);

            services->setShaderConstant(&GUIState.Light.Intensity, uLightIntensity.location, uLightIntensity.type, 1u);
            */
        }

        virtual void OnUnsetMaterial() {}
    };
}

class CShaderManager
{
public:
    struct SParams
    {
        bool constantAlbedo;
        bool isotropicRoughness;
        bool constantRoughness;
        bool roughnessIsZero;
        bool constantRI;
        bool constantMetallic;
        bool metallicIsZero;
        bool metallicIsOne;
        bool AOEnabled;
        bool derivMapIsPresent;
    };

private:
    using Key_t = uint16_t;

    core::unordered_map<Key_t, core::smart_refctd_dynamic_array<core::smart_refctd_ptr<asset::ICPUSpecializedShader>>> Shaders;
    asset::IIncludeHandler* IncludeHandler = nullptr;
    const irr::IrrlichtBaWRendererApp::SGUIState& GUIState;
    const irr::IrrlichtBaWRendererApp::SLightAnimData& LightAnimData;
    scene::ICameraSceneNode* Camera = nullptr;

    enum E_SHADER_FLAGS : Key_t
    {
        ESF_CONST_ALBEDO = 1<<0,
        ESF_ISOTROPIC_ROUGHNESS = 1<<1,
        ESF_CONST_ROUGHNESS = 1<<2,
        ESF_ZERO_ROUGHNESS = 1<<3,
        ESF_CONST_RI = 1<<4,
        ESF_CONST_METALLIC = 1<<5,
        ESF_ZERO_METALLIC = 1<<6,
        ESF_ONE_METALLIC = 1<<7,
        ESF_AO_ENABLED = 1<<8,
        ESF_DERIV_MAP_PRESENT = 1<<9
    };

    static constexpr const char* VERTEX_SHADER_SRC = 
R"(#version 430 core

layout (location = 0) in vec3 vPosition;
layout (location = 2) in vec2 vTexCoords;
layout (location = 3) in vec3 vNormal;

out vec3 WorldPos;
out vec2 TexCoords;
out vec3 Normal;

layout (location = 15) uniform mat4 uVPMat;

void main()
{
    vec3 world = vPosition.xyz;
    WorldPos = world;
    TexCoords = vTexCoords;
    Normal = normalize(vNormal);
    
    gl_Position = uVPMat * vec4(world, 1.0);
}
)";

    static constexpr uint32_t firstSetBit(Key_t _x)
    {
        uint32_t n{};
        while (!(_x & Key_t{1u})) {
            ++n;
            _x >>= 1;
        }
        return n;
    }
    Key_t flagsToKey(const SParams& _p) {
        assert(!(_p.metallicIsZero && _p.metallicIsOne)); //this would be weird

        Key_t key{};
        key |= Key_t{_p.constantAlbedo}<<firstSetBit(ESF_CONST_ALBEDO);
        key |= Key_t{_p.isotropicRoughness}<<firstSetBit(ESF_ISOTROPIC_ROUGHNESS);
        key |= Key_t{_p.constantRoughness}<<firstSetBit(ESF_CONST_ROUGHNESS);
        key |= Key_t{_p.roughnessIsZero}<<firstSetBit(ESF_ZERO_ROUGHNESS);
        key |= Key_t{_p.constantRI}<<firstSetBit(ESF_CONST_RI);
        key |= Key_t{_p.constantMetallic}<<firstSetBit(ESF_CONST_METALLIC);
        key |= Key_t{_p.metallicIsZero}<<firstSetBit(ESF_ZERO_METALLIC);
        key |= Key_t{_p.metallicIsOne}<<firstSetBit(ESF_ONE_METALLIC);
        key |= Key_t{_p.AOEnabled}<<firstSetBit(ESF_AO_ENABLED);
        key |= Key_t{_p.derivMapIsPresent}<<firstSetBit(ESF_DERIV_MAP_PRESENT);

        return key;
    }

    std::string genGetters(const SParams& _params)
    {
        std::string source = 
R"(
float IoRfromF0(float F0) {
    return 2.0/(1.0 - sqrt(F0)) - 1.0;
}
float ReIoRfromF0andImIoR(float F0, float ImIoR)
{
   float T0 = 1.0-F0;
   float kT0 = ImIoR*T0;
   return (1.0+F0+sqrt(4.0*F0-kT0*kT0))/T0;
}
)";

        source += "float getRoughness(in vec2 texCoords) {\n";
        if (_params.constantRoughness)
            source += "\treturn uRoughness;";
        else
            source += "\treturn texture(uRoughnessMap, texCoords).x;";
        source += "\n}\n";

        source += "float getMetallic(in vec2 texCoords) {\n";
        if (_params.constantMetallic)
        {
            if (_params.metallicIsZero)
                source += "\treturn 0.0;";
            else if (_params.metallicIsOne)
                source += "\treturn 1.0;";
            else
                source += "\treturn uMetallic;";
        }
        else
            source += "\treturn texture(uMetallicMap, texCoords).x;";
        source += "\n}\n";

        source += "vec3 getReflectance(in vec2 texCoords) {\n";
        if (_params.constantRI)
            source += "\treturn uRealIoR;";
        else
            source +=
            "\treturn texture(uIoRMap, texCoords).xxx;";
        source += "\n}\n";

        source += "float getAO(in vec2 texCoords) {\n";
        if (_params.AOEnabled)
            source += "\treturn texture(uAOMap, texCoords).x;";
        else
            source += "\treturn 1.0;";
        source += "\n}\n";

        source += "vec3 getAlbedo(in vec2 texCoords) {\n";
        if (_params.constantAlbedo)
            source += "\treturn uAlbedo;";
        else
            source += "\treturn texture(uAlbedoMap, texCoords).rgb;";
        source += "\n}\n";

        return source;
    }

    core::smart_refctd_dynamic_array<core::smart_refctd_ptr<asset::ICPUSpecializedShader>> addShaders(const SParams& _params)
    {
        std::string source =
        R"(#version 430 core

layout (location = 0) out vec4 OutColor;

in vec3 WorldPos;
in vec2 TexCoords;
in vec3 Normal;

layout (location = 0) uniform vec3 uEmissive;
layout (location = 1) uniform vec3 uAlbedo;
layout (location = 2) uniform float uRoughness;
layout (location = 3) uniform float uAnisotropy;
layout (location = 4) uniform vec3 uRealIoR;
layout (location = 5) uniform float uMetallic;
layout (location = 6) uniform float uHeightScaleFactor;
layout (location = 7) uniform vec3 uLightColor;
layout (location = 8) uniform vec3 uLightPos;
layout (location = 9) uniform vec3 uEyePos;
layout (location = 10) uniform float uLightIntensity;
layout (location = 11) uniform vec3 uImagIoR;
layout (binding = 0) uniform sampler2D uAlbedoMap;
layout (binding = 1) uniform sampler2D uRoughnessMap;
layout (binding = 2) uniform sampler2D uIoRMap;
layout (binding = 3) uniform sampler2D uMetallicMap;
layout (binding = 4) uniform sampler2D uDerivativeMap;
layout (binding = 5) uniform sampler2D uAOMap;

float getRoughness(in vec2 texCoords);
float getMetallic(in vec2 texCoords);
vec3 getReflectance(in vec2 texCoords);
float getAO(in vec2 texCoords);
vec3 getAlbedo(in vec2 texCoords);
float IoRfromF0(float F0);
float ReIoRfromF0andImIoR(float F0, float ImIoR);

#define FLT_MIN 1.175494351e-38
#define FLT_MAX 3.402823466e+38
#define FLT_INF (1.0/0.0)

#define REFLECTANCE_SCALE_FACTOR 0.16
)"
+
IncludeHandler->getIncludeStandard("irr/builtin/glsl/brdf/diffuse/oren_nayar.glsl")
+
IncludeHandler->getIncludeStandard("irr/builtin/glsl/brdf/specular/ndf/ggx_trowbridge_reitz.glsl")
+
IncludeHandler->getIncludeStandard("irr/builtin/glsl/brdf/specular/geom/ggx_smith.glsl")
+
IncludeHandler->getIncludeStandard("irr/builtin/glsl/brdf/specular/fresnel/fresnel.glsl")
+
R"(
float diffuse(in float a2, in vec3 N, in vec3 L, in vec3 V, in float NdotL, in float NdotV);
vec3 specular(in float a2, in float at, in float ab, in float NdotL, in float NdotV, in float NdotH, in float VdotH, in float TdotV, in float TdotL, in float BdotV, in float BdotL, in float TdotH, in float BdotH, in mat2x3 ior, in float metallic);
vec3 Fresnel_combined(in mat2x3 ior, in float cosTheta, in float metallic);

vec3 calculateSurfaceGradient(in vec3 normal, in vec3 dpdx, in vec3 dpdy, in float dhdx, in float dhdy)
{
    vec3 r1 = cross(dpdy, normal);
    vec3 r2 = cross(normal, dpdx);
 
    return (r1*dhdx + r2*dhdy) / dot(dpdx, r1);
}

vec3 perturbNormal(in vec3 normal, in vec3 dpdx, in vec3 dpdy, in float dhdx, in float dhdy)
{
    return normalize(normal - calculateSurfaceGradient(normal, dpdx, dpdy, dhdx, dhdy));
}

float applyChainRule(in vec2 h_gradient, in vec2 dUVd_)
{
    return dot(h_gradient, dUVd_);
}

// Calculate the surface normal using the uv-space gradient (dhdu, dhdv)
vec3 calculateSurfaceNormal(in vec3 normal, in vec2 h_gradient, in vec3 dpdx, in vec3 dpdy, in vec2 dUVdx, in vec2 dUVdy)
{
    float dhdx = applyChainRule(h_gradient, dUVdx);
    float dhdy = applyChainRule(h_gradient, dUVdy);
 
    return perturbNormal(normal, dpdx, dpdy, dhdx, dhdy);
}

vec3 calcDiffuseFresnelCorrectionFactor(in vec3 n, in vec3 n2)
{
    //assert(n*n==n2);
    bvec3 TIR = lessThan(n,vec3(1.0));
    vec3 invdenum = mix(vec3(1.0), vec3(1.0)/(n2*n2*(vec3(554.33) - 380.7*n)), TIR);
    vec3 num = n*mix(vec3(0.1921156102251088),n*298.25 - 261.38*n2 + 138.43,TIR);
    num += mix(vec3(0.8078843897748912),vec3(-1.67),TIR);
    return num*invdenum;
}

void main() {
    vec3 N = normalize(Normal);
    const vec2 texCoords = vec2(TexCoords.x, 1.0-TexCoords.y);
    vec3 dp1 = dFdx(WorldPos);
    vec3 dp2 = dFdy(WorldPos);
    vec2 duv1 = dFdx(TexCoords);
    vec2 duv2 = dFdy(TexCoords);
    vec3 dp2perp = cross(dp2, N);
    vec3 dp1perp = cross(N, dp1);
    vec3 T = normalize(dp2perp * duv1.x + dp1perp * duv2.x);
)"
    +
    (_params.derivMapIsPresent ?
R"(
    vec2 derivMapSz = vec2(textureSize(uDerivativeMap, 0));
    vec2 h_gradient = texture(uDerivativeMap, texCoords).xy*uHeightScaleFactor*max(derivMapSz.x, derivMapSz.y);
    N = calculateSurfaceNormal(N, h_gradient, dp1, dp2, duv1, duv2);
)" : ""
    )
    +
R"(
    vec3 B = cross(N, T);
    const vec3 relLightPos = uLightPos - WorldPos;
    float NdotL = dot(N, relLightPos);

    const float ao = getAO(texCoords);

	vec3 color = uEmissive*ao*0.01;
	if (NdotL>FLT_MIN)
	{
		const float relLightPosLen2 = dot(relLightPos, relLightPos);
        const float L_rcpLen = inversesqrt(relLightPosLen2);
		NdotL *= L_rcpLen;

		// there are better identities to get all of these
		const vec3 V = normalize(uEyePos - WorldPos);
		const vec3 L = relLightPos*L_rcpLen;
        const vec3 H = normalize(V+L);

		float NdotV = dot(N, V);
        float LdotV = dot(L, V);

        // dots with H identities taken from Earl Hammon's PBR Diffuse Lighting GDC17 lecture
        const float LplusV_lenSq = 2.0 + 2.0*LdotV;
        const float LplusV_rcpLen = inversesqrt(LplusV_lenSq);
        const float NdotH = max((NdotL + NdotV) * LplusV_rcpLen, 0.0);
        const float VdotH = LplusV_rcpLen + LplusV_rcpLen*LdotV;

        const float TdotV = dot(T, V);
        const float TdotL = dot(T, L);
        const float BdotV = dot(B, V);
        const float BdotL = dot(B, L);
        const float TdotH = (TdotL + TdotV)*LplusV_rcpLen;
        const float BdotH = (BdotL + BdotV)*LplusV_rcpLen;
		// identity comment end (but also do you need to clamp all of them?)

		const float a2 = getRoughness(texCoords);
        const float at = sqrt(a2);
        const float ab = at*(1.0 - uAnisotropy);
		const float metallic = getMetallic(texCoords);
		const vec3 baseColor = getAlbedo(texCoords);
)"
    +
    [&_params] {
    if (!_params.constantRI)
        return
R"(
        vec3 reflectance = getReflectance(texCoords);
        vec3 F0 = mix(reflectance*reflectance*REFLECTANCE_SCALE_FACTOR, baseColor, metallic);
        vec3 realIoR = vec3(
            ReIoRfromF0andImIoR(F0.x, uImagIoR.x),
            ReIoRfromF0andImIoR(F0.y, uImagIoR.y),
            ReIoRfromF0andImIoR(F0.z, uImagIoR.z)
        );
        if (any(isnan(realIoR)))
        {
            OutColor = vec4(1.0, 0.0, 0.0, 1.0);
            return;
        }
)";
    else // getReflectance() reads reflectance texture OR uRealIoR uniform
        return "\t\tvec3 realIoR = getReflectance(texCoords);";
    }()
    +
R"(
        vec3 realIoR2 = realIoR*realIoR;
        mat2x3 IoR = mat2x3(realIoR2, uImagIoR*uImagIoR);

        // (2*n^2 + 2*cos(theta)^2 - 2) is (2*n^2 + cos(2*theta) - 1) is (sqrt(n^2 - sin(theta)^2)^2) + n^2 - sin(theta)^2)
        // which comes from (a2plusb2 + t0) in Fresnel_conductor (with assumption of Etak=0) which cannot be <0.
        // n is obviously Eta, real part of IoR
        // checks needed for n<1
        if (any(lessThan(2.0*realIoR2 + 2.0*NdotV*NdotV - 2.0, vec3(0.0))) ||
            any(lessThan(2.0*realIoR2 + 2.0*NdotL*NdotL - 2.0, vec3(0.0))) ||
            any(lessThan(2.0*realIoR2 + 2.0*VdotH*VdotH - 2.0, vec3(0.0)))
        ) {
            OutColor = vec4(1.0, 0.0, 0.0, 1.0);
            return;
        }
            
        vec3 diffuseFactor = calcDiffuseFresnelCorrectionFactor(realIoR, realIoR2) * (vec3(1.0)-Fresnel_combined(IoR, NdotV, metallic))*(vec3(1.0)-Fresnel_combined(IoR, NdotL, metallic));
		float diffuse = diffuse(a2, N, L, V, NdotL, NdotV) * (1.0 - metallic);
		vec3 spec = specular(a2, at, ab, NdotL, NdotV, NdotH, VdotH, TdotV, TdotL, BdotV, BdotL, TdotH, BdotH, IoR, metallic);

		color += ((diffuse * baseColor * ao * diffuseFactor) + spec) * NdotL * uLightIntensity * uLightColor / relLightPosLen2;
	}
	OutColor = vec4(color, 1.0);
}
)";
        source += genGetters(_params);

        source += "float diffuse(in float a2, in vec3 N, in vec3 L, in vec3 V, in float NdotL, in float NdotV) {\n";
        if (_params.constantMetallic && _params.metallicIsOne)
        {
            source += "\treturn 0.0;";
        }
        else
        {
            if (_params.constantRoughness && _params.roughnessIsZero)
                source += "\treturn 1.0/3.14159265359;";
            else
                source += "\treturn oren_nayar(a2, N, L, V, NdotL, NdotV);";
        }
        source += "\n}\n";
        source += 
R"(vec3 specular(in float a2, in float at, in float ab, in float NdotL, in float NdotV, in float NdotH, in float VdotH, in float TdotV, in float TdotL, in float BdotV, in float BdotL, in float TdotH, in float BdotH, in mat2x3 ior, in float metallic) {
	//assert(NdotL>FLT_MIN);
    if (NdotV<FLT_MIN)
        return vec3(0.0);
    if (a2*(1.0-uAnisotropy)<FLT_MIN)
		return vec3(/*NdotH>=(1.0-FLT_MIN) ? FLT_INF:*/0.0);

    vec3 f = Fresnel_combined(ior, VdotH, metallic);
    float ndf = GGXBurleyAnisotropic(uAnisotropy, a2, TdotH, BdotH, NdotH);//GGXTrowbridgeReitz(a2, NdotH);
    float geom = GGXSmithHeightCorrelated_aniso_wo_numerator(at, ab, TdotL, TdotV, BdotL, BdotV, NdotL, NdotV);//GGXSmithHeightCorrelated_wo_numerator(a2, NdotL, NdotV);

    // Note: (4.0*NdotV*NdotL) denominator is cancelled by GGXSmith's numerator, thus the use of GGXSmithHeightCorrelated_wo_numerator()
    return ndf*geom*f;
}

vec3 Fresnel_combined(in mat2x3 ior, in float cosTheta, in float metallic) {
    bvec3 not_inf = lessThan(ior[0]*ior[0] + ior[1]*ior[1], vec3(FLT_MAX));
    return mix(
        vec3(1.0),
        Fresnel_conductor(ior[0], ior[1], cosTheta),
        not_inf
    );
}
)";
        auto f = fopen("fragsrc.txt", "w");
        fprintf(f, "%s", source.c_str());
        fclose(f);

        auto vertexShader = core::make_smart_refctd_ptr<asset::ICPUSpecializedShader>(core::make_smart_refctd_ptr<asset::ICPUShader>(VERTEX_SHADER_SRC), asset::ISpecializedShader::SInfo({}, nullptr, "main", asset::ISpecializedShader::ESS_VERTEX));
        auto fragmentShader = core::make_smart_refctd_ptr<asset::ICPUSpecializedShader>(core::make_smart_refctd_ptr<asset::ICPUShader>(source.c_str()), asset::ISpecializedShader::SInfo({}, nullptr, "main", asset::ISpecializedShader::ESS_FRAGMENT));
       
        auto shaders = core::make_refctd_dynamic_array<core::smart_refctd_dynamic_array<core::smart_refctd_ptr<asset::ICPUSpecializedShader>>>(2);
        shaders->front() = std::move(vertexShader);
        shaders->back() = std::move(fragmentShader);

        // separate CB for each shader because shader-constants' values are most likely cached, so a single CB cannot be used for multiple shaders
        //video::IShaderConstantSetCallBack* cb = new CShaderConstantSetCallback(Camera, GUIState, LightAnimData);
        //video::E_MATERIAL_TYPE shader = static_cast<video::E_MATERIAL_TYPE>(Services->addHighLevelShaderMaterial(VERTEX_SHADER_SRC, nullptr, nullptr, nullptr, source.c_str(), 3u, video::EMT_SOLID, cb));
        //cb->drop();

        return Shaders.insert({flagsToKey(_params), shaders}).first->second;
    }

public:
    CShaderManager(asset::IIncludeHandler* _inclHandler, const irr::IrrlichtBaWRendererApp::SGUIState& _guiState, const irr::IrrlichtBaWRendererApp::SLightAnimData& _lightAnimDat, scene::ICameraSceneNode* _camera) 
        : IncludeHandler{_inclHandler}, GUIState{_guiState}, LightAnimData{_lightAnimDat}, Camera{_camera}
    {

    }

    auto getShader(const SParams& _params)
    {
        decltype(Shaders)::const_iterator found;
        if ((found = Shaders.find(flagsToKey(_params))) != Shaders.cend())
            return found->second;

        return addShaders(_params);
    }
};

class CDerivativeMapManager
{
    using Key_t = std::pair<core::smart_refctd_ptr<irr::video::IGPUImageView>, float>;

    core::map<Key_t, core::smart_refctd_ptr<irr::video::IGPUImageView>> DerivMaps;
    const irr::video::CDerivativeMapCreator* DerivMapCreator;

public:
    CDerivativeMapManager(IrrlichtDevice* _device) : DerivMapCreator(_device->getVideoDriver()->getDerivativeMapCreator()) {}
    ~CDerivativeMapManager() {
        for (auto& t : DerivMaps)
            t.second->drop();
    }

    core::smart_refctd_ptr<irr::video::IGPUImageView> getDerivativeMap(core::smart_refctd_ptr<irr::video::IGPUImageView> _bumpMap, float _heightFactor)
    {
        auto found = DerivMaps.find({_bumpMap, _heightFactor});
        if (found != DerivMaps.end())
            return found->second;
        return DerivMaps[{_bumpMap, _heightFactor}] = DerivMapCreator->createDerivMapFromBumpMap(_bumpMap.get(), _heightFactor);
    }
};

namespace irr
{

IrrlichtBaWRendererApp::IrrlichtBaWRendererApp(IrrlichtDevice* device, irr::scene::ICameraSceneNode* _camera)
    :   Camera(_camera),
        Driver(device->getVideoDriver()),
        AssetManager(device->getAssetManager()),
        GUI(ext::cegui::createGUIManager(device)),
        ShaderManager(new CShaderManager(AssetManager->getGLSLCompiler()->getIncludeHandler(), GUIState, LightAnimData, Camera)),
        DerivativeMapManager(new CDerivativeMapManager(device))
{
    Camera->setPosition(core::vector3df(2.26f, -4.05f, 27.6f));
    Camera->setTarget(core::vector3df(0.f));

    TextureSlotMap = {
        { ETEXTURE_SLOT::TEXTURE_AO,
        std::make_tuple("AOTextureBuffer", // Texture buffer name (cegui image name)
            "AOTexture", // Texture name
            "MaterialParamsWindow/AOWindow/ImageButton") },

        { ETEXTURE_SLOT::TEXTURE_BUMP,
            std::make_tuple("BumpTextureBuffer", // Texture buffer name (cegui image name)
                "BumpTexture", // Texture name
                "MaterialParamsWindow/BumpWindow/ImageButton") },

        { ETEXTURE_SLOT::TEXTURE_SLOT_1,
            std::make_tuple("T1TextureBuffer", // Texture buffer name (cegui image name)
                "T1Texture", // Texture name
                "TextureViewWindow/Texture0Window/Texture") },

        { ETEXTURE_SLOT::TEXTURE_SLOT_2,
            std::make_tuple("T2TextureBuffer", // Texture buffer name (cegui image name)
                "T2Texture", // Texture name
                "TextureViewWindow/Texture1Window/Texture") },

        { ETEXTURE_SLOT::TEXTURE_SLOT_3,
            std::make_tuple("T3TextureBuffer", // Texture buffer name (cegui image name)
                "T3Texture", // Texture name
                "TextureViewWindow/Texture2Window/Texture") },

        { ETEXTURE_SLOT::TEXTURE_SLOT_4,
            std::make_tuple("T4TextureBuffer", // Texture buffer name (cegui image name)
                "T4Texture", // Texture name
                "TextureViewWindow/Texture3Window/Texture") },
    };

    GUI->init();
    GUI->createRootWindowFromLayout(
        ext::cegui::readWindowLayout("../../media/brdf_explorer/MainWindow.layout")
    );
    auto onColorPicked = [](const ::CEGUI::Colour& _ceguiColor, core::vector3df& _irrColor) {
        _irrColor.X = _ceguiColor.getRed();
        _irrColor.Y = _ceguiColor.getGreen();
        _irrColor.Z = _ceguiColor.getBlue();
    };
    GUI->createColourPicker(false, "LightParamsWindow/ColorWindow", "Color", "pickerLightColor", std::bind(onColorPicked, std::placeholders::_1, std::ref(GUIState.Light.Color)));
    GUI->createColourPicker(true, "MaterialParamsWindow/EmissiveWindow", "Emissive", "pickerEmissiveColor", std::bind(onColorPicked, std::placeholders::_1, std::ref(GUIState.Emissive.Color)));
    GUI->createColourPicker(true, "MaterialParamsWindow/AlbedoWindow", "Albedo", "pickerAlbedoColor", std::bind(onColorPicked, std::placeholders::_1, std::ref(GUIState.Albedo.ConstantColor)));

    // Fill all the available texture slots using the default (no texture) image
    //const auto image_default = ext::cegui::loadImage("../../media/brdf_explorer/DefaultEmpty.png");
    auto cputexture_default = loadCPUTexture("../../media/brdf_explorer/DefaultEmpty.png");

    auto imageInfo = cputexture_default->getCreationParameters();
    ICPUImageView::SCreationParams imageViewInfo;
    imageViewInfo.image = std::move(cputexture_default);
    imageViewInfo.format = imageInfo.format;
    imageViewInfo.viewType = IImageView<ICPUImage>::E_TYPE::ET_2D;
    imageViewInfo.flags = static_cast<ICPUImageView::E_CREATE_FLAGS>(0u);
    imageViewInfo.subresourceRange.baseArrayLayer = 0u;
    imageViewInfo.subresourceRange.baseMipLevel = 0u;
    imageViewInfo.subresourceRange.layerCount = imageInfo.arrayLayers;
    imageViewInfo.subresourceRange.levelCount = imageInfo.mipLevels;

    auto imageView = ICPUImageView::create(std::move(imageViewInfo));

    DefaultTexture = Driver->getGPUObjectsFromAssets(&imageView.get(), (&imageView.get())+1)->front();
    auto textureBundle = asset::SAssetBundle({ imageView });

    loadTextureSlot(ETEXTURE_SLOT::TEXTURE_AO, DefaultTexture, textureBundle.getCacheKey());
    loadTextureSlot(ETEXTURE_SLOT::TEXTURE_BUMP, DefaultTexture, textureBundle.getCacheKey());
    loadTextureSlot(ETEXTURE_SLOT::TEXTURE_SLOT_1, DefaultTexture, textureBundle.getCacheKey());
    loadTextureSlot(ETEXTURE_SLOT::TEXTURE_SLOT_2, DefaultTexture, textureBundle.getCacheKey());
    loadTextureSlot(ETEXTURE_SLOT::TEXTURE_SLOT_3, DefaultTexture, textureBundle.getCacheKey());
    loadTextureSlot(ETEXTURE_SLOT::TEXTURE_SLOT_4, DefaultTexture, textureBundle.getCacheKey());

    auto root = GUI->getRootWindow();
    // Material window: Subscribe to sliders' events and set its default value to
    // 0.0.
    for (uint32_t i = 0u; i < 6u; ++i)
    {
        const std::string RIWindowName = "MaterialParamsWindow/RefractionIndexWindow" + std::to_string(i+1u);
        GUI->registerSliderEvent(
            (RIWindowName+"/Slider").c_str(), i < 3u ? sliderRealRIRange : sliderImagRIRange, 0.01f,
            [root, this, RIWindowName, i](const ::CEGUI::EventArgs&) {
            auto val = static_cast<::CEGUI::Slider*>(
                root->getChild(
                    RIWindowName+"/Slider"))
                ->getCurrentValue();
            val = std::max(val, i<3u ? 0.04f : 0.f);
            root->getChild(
                RIWindowName+"/LabelPercent")
                ->setText(ext::cegui::toStringFloat(val, 2));

            if (i < 3u) // real
                (&GUIState.RefractionIndex.ConstantReal.X)[i%3] = val;
            else // imag
                (&GUIState.RefractionIndex.ConstantImag.X)[i%3] = val;
        });
    }

    GUI->registerSliderEvent(
        "MaterialParamsWindow/MetallicWindow/Slider", sliderMetallicRange, 0.01f,
        [root,this](const ::CEGUI::EventArgs&) {
            auto metallic = static_cast<::CEGUI::Slider*>(
                root->getChild("MaterialParamsWindow/MetallicWindow/Slider"))
                                 ->getCurrentValue();
            root->getChild("MaterialParamsWindow/MetallicWindow/LabelPercent")
                ->setText(ext::cegui::toStringFloat(metallic, 2));

            GUIState.Metallic.ConstValue = metallic;
        });

    GUI->registerSliderEvent(
        "MaterialParamsWindow/RoughnessWindow/Slider", sliderRoughness1Range,
        0.01f, [this](const ::CEGUI::EventArgs&) {
            auto root = GUI->getRootWindow();

            const auto v = static_cast<::CEGUI::Slider*>(
                root->getChild("MaterialParamsWindow/RoughnessWindow/Slider"))
                               ->getCurrentValue();
            const auto s = ext::cegui::toStringFloat(v, 2);

            root->getChild("MaterialParamsWindow/RoughnessWindow/LabelPercent1")
                ->setText(s);

            GUIState.Roughness.ConstValue1 = v;
        });

    GUI->registerSliderEvent(
        "MaterialParamsWindow/RoughnessWindow/Slider2", sliderRoughness2Range,
        0.01f, [root,this](const ::CEGUI::EventArgs&) {
            auto roughness = static_cast<::CEGUI::Slider*>(
                root->getChild("MaterialParamsWindow/RoughnessWindow/Slider2"))
                                 ->getCurrentValue();
            root->getChild("MaterialParamsWindow/RoughnessWindow/LabelPercent2")
                ->setText(ext::cegui::toStringFloat(roughness, 2));

            GUIState.Roughness.ConstValue2 = roughness;
        });

    // Set the sliders' text objects to their default value (whatever value the
    // slider was set to).
    {
        // Roughness slider, first one
        root->getChild("MaterialParamsWindow/RoughnessWindow/LabelPercent2")
            ->setText(ext::cegui::toStringFloat(
                static_cast<::CEGUI::Slider*>(
                    root->getChild("MaterialParamsWindow/RoughnessWindow/Slider2"))
                    ->getCurrentValue(),
                2));

        // Roughness slider, second one
        root->getChild("MaterialParamsWindow/RoughnessWindow/LabelPercent1")
            ->setText(ext::cegui::toStringFloat(
                static_cast<::CEGUI::Slider*>(
                    root->getChild("MaterialParamsWindow/RoughnessWindow/Slider"))
                    ->getCurrentValue(),
                2));

        // Refractive index slider
        for (uint32_t i = 0u; i < 6u; ++i)
        {
            const std::string RIWindowName = "RefractionIndexWindow" + std::to_string(i+1u);
            auto slider = static_cast<::CEGUI::Slider*>(
                root->getChild(
                    "MaterialParamsWindow/" + RIWindowName + "/Slider"));
            if (i < 3u)
                slider->setCurrentValue(1.33f);
            float val = slider->getCurrentValue();
            root->getChild("MaterialParamsWindow/"+RIWindowName+"/LabelPercent")
                ->setText(ext::cegui::toStringFloat(val,2));
        }
        // Metallic slider
        root->getChild("MaterialParamsWindow/MetallicWindow/LabelPercent")
            ->setText(ext::cegui::toStringFloat(
                static_cast<::CEGUI::Slider*>(
                    root->getChild("MaterialParamsWindow/MetallicWindow/Slider"))
                    ->getCurrentValue(),
                2));

        // Bump-mapping's height slider
        root->getChild("MaterialParamsWindow/BumpWindow/LabelPercent")
            ->setText(ext::cegui::toStringFloat(
                static_cast<::CEGUI::Slider*>(
                    root->getChild("MaterialParamsWindow/BumpWindow/Spinner"))
                    ->getCurrentValue(),
                2));
    }

    // light animation checkbox
    auto lightAnimated = static_cast<::CEGUI::ToggleButton*>(root->getChild("LightParamsWindow/AnimationWindow/Checkbox"));
    lightAnimated->subscribeEvent(
        ::CEGUI::ToggleButton::EventSelectStateChanged,
        [this](const ::CEGUI::EventArgs& e) {
            const ::CEGUI::WindowEventArgs& we = static_cast<const ::CEGUI::WindowEventArgs&>(e);
            GUIState.Light.Animated = static_cast<::CEGUI::ToggleButton*>(we.window)->isSelected();

            auto root = GUI->getRootWindow();
            root->getChild("LightParamsWindow/PositionWindow")->setDisabled(GUIState.Light.Animated);
        }
    );
    lightAnimated->setSelected(true);

    GUI->registerSliderEvent(
        "LightParamsWindow/IntensityWindow/IntensitySlider", sliderLightIntensityRange, 1.f,
        [root, this](const ::CEGUI::EventArgs&) {
        auto intensity = static_cast<::CEGUI::Slider*>(
            root->getChild("LightParamsWindow/IntensityWindow/IntensitySlider"))
            ->getCurrentValue();
        GUIState.Light.Intensity = intensity+1.f;
    });
    static_cast<::CEGUI::Slider*>(
        root->getChild("LightParamsWindow/IntensityWindow/IntensitySlider"))->setCurrentValue(800.f);

    auto lightZ = static_cast<::CEGUI::Spinner*>(root->getChild("LightParamsWindow/PositionWindow/LightZ"));
    lightZ->subscribeEvent(
        ::CEGUI::Spinner::EventValueChanged,
        [this](const ::CEGUI::EventArgs& e) {
            const ::CEGUI::WindowEventArgs& we = static_cast<const ::CEGUI::WindowEventArgs&>(e);
            GUIState.Light.ConstantPosition.Z = static_cast<::CEGUI::Spinner*>(we.window)->getCurrentValue();
        }
    );
    auto lightY = static_cast<::CEGUI::Spinner*>(root->getChild("LightParamsWindow/PositionWindow/LightY"));
    lightY->subscribeEvent(
        ::CEGUI::Spinner::EventValueChanged,
        [this](const ::CEGUI::EventArgs& e) {
            const ::CEGUI::WindowEventArgs& we = static_cast<const ::CEGUI::WindowEventArgs&>(e);
            GUIState.Light.ConstantPosition.Y = static_cast<::CEGUI::Spinner*>(we.window)->getCurrentValue();
        }
    );
    auto lightX = static_cast<::CEGUI::Spinner*>(root->getChild("LightParamsWindow/PositionWindow/LightX"));
    lightX->subscribeEvent(
        ::CEGUI::Spinner::EventValueChanged,
        [this](const ::CEGUI::EventArgs& e) {
            const ::CEGUI::WindowEventArgs& we = static_cast<const ::CEGUI::WindowEventArgs&>(e);
            GUIState.Light.ConstantPosition.X = static_cast<::CEGUI::Spinner*>(we.window)->getCurrentValue();
        }
    );
    
    // Isotropic checkbox
    auto isotropic = static_cast<::CEGUI::ToggleButton*>(root->getChild("MaterialParamsWindow/RoughnessWindow/Checkbox"));
    isotropic->subscribeEvent(
        ::CEGUI::ToggleButton::EventSelectStateChanged,
        [this](const ::CEGUI::EventArgs& e) {
            auto root = GUI->getRootWindow();

            const ::CEGUI::WindowEventArgs& we = static_cast<const ::CEGUI::WindowEventArgs&>(e);
            GUIState.Roughness.IsIsotropic = static_cast<::CEGUI::ToggleButton*>(we.window)->isSelected();
            static_cast<::CEGUI::Slider*>(
                root->getChild("MaterialParamsWindow/RoughnessWindow/Slider2"))
                ->setDisabled(GUIState.Roughness.IsIsotropic);

            if (GUIState.Roughness.IsIsotropic) {
                root->getChild("MaterialParamsWindow/RoughnessWindow/LabelPercent2")
                    ->setText(ext::cegui::toStringFloat(
                        static_cast<::CEGUI::Slider*>(
                            root->getChild(
                                "MaterialParamsWindow/RoughnessWindow/Slider"))
                            ->getCurrentValue(),
                        2));
            }
        });

    // Load Model button
    auto button_loadModel = static_cast<::CEGUI::PushButton*>(
        root->getChild("LoadModelButton"));

    button_loadModel->subscribeEvent(::CEGUI::PushButton::EventClicked,
        ::CEGUI::Event::Subscriber(&IrrlichtBaWRendererApp::eventMeshBrowse, this));

    // AO texturing & bump-mapping texturing window
    auto button_browse_AO = static_cast<::CEGUI::PushButton*>(
        root->getChild("MaterialParamsWindow/AOWindow/Button"));

    button_browse_AO->subscribeEvent(::CEGUI::PushButton::EventClicked,
        ::CEGUI::Event::Subscriber(&IrrlichtBaWRendererApp::eventAOTextureBrowse, this));

    static_cast<::CEGUI::DefaultWindow*>(
        root->getChild("MaterialParamsWindow/AOWindow/ImageButton"))
        ->subscribeEvent(::CEGUI::Window::EventMouseClick,
            ::CEGUI::Event::Subscriber(&IrrlichtBaWRendererApp::eventAOTextureBrowse, this));

    static_cast<::CEGUI::Editbox*>(root->getChild("MaterialParamsWindow/AOWindow/Editbox"))
        ->subscribeEvent(::CEGUI::Editbox::EventTextAccepted,
            ::CEGUI::Event::Subscriber(&IrrlichtBaWRendererApp::eventAOTextureBrowse_EditBox, this));

    auto ao_enabled = static_cast<::CEGUI::ToggleButton*>(root->getChild("MaterialParamsWindow/AOWindow/Checkbox"));
    ao_enabled->subscribeEvent(
        ::CEGUI::ToggleButton::EventSelectStateChanged,
        [this](const ::CEGUI::EventArgs& e) {
            auto root = GUI->getRootWindow();

            const ::CEGUI::WindowEventArgs& we = static_cast<const ::CEGUI::WindowEventArgs&>(e);
            GUIState.AmbientOcclusion.Enabled = static_cast<::CEGUI::ToggleButton*>(we.window)->isSelected();
        });

    auto button_browse_bump_map = static_cast<::CEGUI::PushButton*>(
        root->getChild("MaterialParamsWindow/BumpWindow/Button"));

    button_browse_bump_map->subscribeEvent(::CEGUI::PushButton::EventClicked,
        ::CEGUI::Event::Subscriber(&IrrlichtBaWRendererApp::eventBumpTextureBrowse, this));

    static_cast<::CEGUI::DefaultWindow*>(root->getChild("MaterialParamsWindow/BumpWindow/ImageButton"))
        ->subscribeEvent(::CEGUI::Window::EventMouseClick,
            ::CEGUI::Event::Subscriber(&IrrlichtBaWRendererApp::eventBumpTextureBrowse, this));

    static_cast<::CEGUI::Editbox*>(root->getChild("MaterialParamsWindow/BumpWindow/Editbox"))
        ->subscribeEvent(::CEGUI::Editbox::EventTextAccepted,
            ::CEGUI::Event::Subscriber(&IrrlichtBaWRendererApp::eventBumpTextureBrowse_EditBox, this));

    GUI->registerSliderEvent(
        "MaterialParamsWindow/BumpWindow/Spinner", sliderBumpHeightRange, 1.0f,
        [root,this](const ::CEGUI::EventArgs&) {
            auto height = static_cast<::CEGUI::Slider*>(
                root->getChild("MaterialParamsWindow/BumpWindow/Spinner"))
                                 ->getCurrentValue();
            root->getChild("MaterialParamsWindow/BumpWindow/LabelPercent")
                ->setText(ext::cegui::toStringFloat(height, 2));
            GUIState.BumpMapping.Height = height;
            DerivMapGeneration.HeightFactorChanged = true;
            DerivMapGeneration.TimePointLastHeightFactorChange = Clock::now();
        });

    auto bump_enabled = static_cast<::CEGUI::ToggleButton*>(root->getChild("MaterialParamsWindow/BumpWindow/Checkbox"));
    bump_enabled->subscribeEvent(
        ::CEGUI::ToggleButton::EventSelectStateChanged,
        [this](const ::CEGUI::EventArgs& e) {
        const ::CEGUI::WindowEventArgs& we = static_cast<const ::CEGUI::WindowEventArgs&>(e);
        GUIState.BumpMapping.Enabled = static_cast<::CEGUI::ToggleButton*>(we.window)->isSelected();
    });

    initDropdown();
    initTooltip();

    // Setting up the texture preview window
    std::array<::CEGUI::PushButton*, 4> texturePreviewIcon = {
        static_cast<::CEGUI::PushButton*>(
            root->getChild("TextureViewWindow/Texture0Window/Texture")),
        static_cast<::CEGUI::PushButton*>(
            root->getChild("TextureViewWindow/Texture1Window/Texture")),
        static_cast<::CEGUI::PushButton*>(
            root->getChild("TextureViewWindow/Texture2Window/Texture")),
        static_cast<::CEGUI::PushButton*>(
            root->getChild("TextureViewWindow/Texture3Window/Texture"))
    };

    for (const auto& v : texturePreviewIcon)
    {
        v->subscribeEvent(::CEGUI::PushButton::EventClicked,
            ::CEGUI::Event::Subscriber(&IrrlichtBaWRendererApp::eventTextureBrowse, this));
    }

    // Setting up the master windows & their default opacity
    auto* window_material = static_cast<::CEGUI::FrameWindow*>(root->getChild("MaterialParamsWindow"));
    window_material->subscribeEvent(
        ::CEGUI::FrameWindow::EventCloseClicked, [root](const ::CEGUI::EventArgs&) {
            static_cast<::CEGUI::FrameWindow*>(
                root->getChild("MaterialParamsWindow"))
                ->setVisible(false);
        });
    GUI->setOpacity("MaterialParamsWindow", defaultOpacity);

    auto* window_light = static_cast<::CEGUI::FrameWindow*>(root->getChild("LightParamsWindow"));
    window_light->subscribeEvent(::CEGUI::FrameWindow::EventCloseClicked,
        [root](const CEGUI::EventArgs&) {
            static_cast<CEGUI::FrameWindow*>(
                root->getChild("LightParamsWindow"))
                ->setVisible(false);
        });
    GUI->setOpacity("LightParamsWindow", defaultOpacity);

    auto* window_texture = static_cast<::CEGUI::FrameWindow*>(root->getChild("TextureViewWindow"));
    window_texture->subscribeEvent(
        ::CEGUI::FrameWindow::EventCloseClicked, [root](const ::CEGUI::EventArgs&) {
            static_cast<::CEGUI::FrameWindow*>(
                root->getChild("TextureViewWindow"))
                ->setVisible(false);
        });
    GUI->setOpacity("TextureViewWindow", defaultOpacity);

    {//default mesh

        auto geometryMesh = AssetManager->getGeometryCreator()->createSphereMesh(10.f, 64u, 64u);
        auto defaultMesh = core::make_smart_refctd_ptr<asset::CCPUMesh>();
        auto interfaceMesh = core::smart_refctd_ptr_static_cast<asset::ICPUMesh>(defaultMesh);

        // todo stuff pulled from geometryMesh 

        DefaultMesh = Mesh = Driver->getGPUObjectsFromAssets(&interfaceMesh.get(), (&interfaceMesh.get()) + 1)->front();
        //setUpLight(5.f);
    }

    setGUIForConstantIoR();
}

void IrrlichtBaWRendererApp::initDropdown()
{
    static const std::vector<const char*> drop_ID = {
        "Constant", "Texture 0", "Texture 1", "Texture 2", "Texture 3"
    };
    const auto default_halignment = ::CEGUI::HA_RIGHT;
    const auto default_width = ::CEGUI::UDim(0.5f, 0.0f);
    const auto default_position = ::CEGUI::UVector2(::CEGUI::UDim(0.0f, 0.0f), ::CEGUI::UDim(0.125f, 0.0f));

    auto root = GUI->getRootWindow();

    auto* albedo_drop = GUI->createDropDownList(
        "MaterialParamsWindow/AlbedoDropDownList", "DropDown_Albedo", drop_ID,
        [this](const ::CEGUI::EventArgs&) {
            auto root = GUI->getRootWindow();
            auto* list = static_cast<::CEGUI::Combobox*>(root->getChild(
                "MaterialParamsWindow/AlbedoDropDownList/DropDown_Albedo"));
            list->setProperty("NormalEditTextColour", GUI->WhiteProperty);

            root->getChild("MaterialParamsWindow/AlbedoWindow")
                ->setDisabled(list->getSelectedItem()->getText() != "Constant");

            GUIState.Albedo.SourceDropdown = getDropdownState(DROPDOWN_ALBEDO_NAME);
            updateMaterial();
        });

    albedo_drop->setHorizontalAlignment(default_halignment);
    albedo_drop->setWidth(default_width);
    albedo_drop->setPosition(default_position);

    auto* roughness_drop = GUI->createDropDownList(
        "MaterialParamsWindow/RoughnessDropDownList", "DropDown_Roughness",
        drop_ID, [this](const ::CEGUI::EventArgs&) {
            auto root = GUI->getRootWindow();
            auto* list = static_cast<CEGUI::Combobox*>(root->getChild(
                "MaterialParamsWindow/RoughnessDropDownList/DropDown_Roughness"));
            list->setProperty("NormalEditTextColour", GUI->WhiteProperty);

            root->getChild("MaterialParamsWindow/RoughnessWindow")
                ->setDisabled(list->getSelectedItem()->getText() != "Constant");

            GUIState.Roughness.SourceDropdown = getDropdownState(DROPDOWN_ROUGHNESS_NAME);
            updateMaterial();
        });

    roughness_drop->setHorizontalAlignment(default_halignment);
    roughness_drop->setWidth(default_width);
    roughness_drop->setPosition(default_position);

    auto* ri_drop = GUI->createDropDownList(
        "MaterialParamsWindow/RIDropDownList", "DropDown_RI", drop_ID,
        [this](const ::CEGUI::EventArgs&) {
            auto root = GUI->getRootWindow();

            auto* list = static_cast<::CEGUI::Combobox*>(
                root->getChild("MaterialParamsWindow/RIDropDownList/DropDown_RI"));
            list->setProperty("NormalEditTextColour", GUI->WhiteProperty);

            bool realRIComesFromTexture = list->getSelectedItem()->getText() != "Constant";
            root->getChild("MaterialParamsWindow/RefractionIndexWindow1")
                ->setDisabled(realRIComesFromTexture);
            root->getChild("MaterialParamsWindow/RefractionIndexWindow2")
                ->setDisabled(realRIComesFromTexture);
            root->getChild("MaterialParamsWindow/RefractionIndexWindow3")
                ->setDisabled(realRIComesFromTexture);

            if (realRIComesFromTexture)
                resetGUIAfterConstantIoR();
            else
                setGUIForConstantIoR();

            GUIState.RefractionIndex.SourceDropdown = getDropdownState(DROPDOWN_RI_NAME);
            updateMaterial();
        });

    ri_drop->setHorizontalAlignment(default_halignment);
    ri_drop->setWidth(default_width);
    ri_drop->setPosition(default_position);

    auto* metallic_drop = GUI->createDropDownList(
        "MaterialParamsWindow/MetallicDropDownList", "DropDown_Metallic", drop_ID,
        [this](const ::CEGUI::EventArgs&) {
            auto root = GUI->getRootWindow();

            auto* list = static_cast<::CEGUI::Combobox*>(root->getChild(
                "MaterialParamsWindow/MetallicDropDownList/DropDown_Metallic"));
            list->setProperty("NormalEditTextColour", GUI->WhiteProperty);

            root->getChild("MaterialParamsWindow/MetallicWindow")
                ->setDisabled(list->getSelectedItem()->getText() != "Constant");

            GUIState.Metallic.SourceDropdown = getDropdownState(DROPDOWN_METALLIC_NAME);
            updateMaterial();
        });

    metallic_drop->setHorizontalAlignment(default_halignment);
    metallic_drop->setWidth(default_width);
    metallic_drop->setPosition(default_position);
}

void IrrlichtBaWRendererApp::initTooltip()
{
    auto root = GUI->getRootWindow();

    static_cast<CEGUI::DefaultWindow*>(
        root->getChild("MaterialParamsWindow/BumpWindow/ImageButton"))
        ->setTooltipText("Left-click to select a bump-mapping texture.");
    static_cast<CEGUI::DefaultWindow*>(
        root->getChild("MaterialParamsWindow/AOWindow/ImageButton"))
        ->setTooltipText("Left-click to select an AO texture.");
    static_cast<CEGUI::DefaultWindow*>(
        root->getChild("TextureViewWindow/Texture0Window/Texture"))
        ->setTooltipText("Left-click to select a new texture.");
    static_cast<CEGUI::DefaultWindow*>(
        root->getChild("TextureViewWindow/Texture1Window/Texture"))
        ->setTooltipText("Left-click to select a new texture.");
    static_cast<CEGUI::DefaultWindow*>(
        root->getChild("TextureViewWindow/Texture2Window/Texture"))
        ->setTooltipText("Left-click to select a new texture.");
    static_cast<CEGUI::DefaultWindow*>(
        root->getChild("TextureViewWindow/Texture3Window/Texture"))
        ->setTooltipText("Left-click to select a new texture.");
}

void IrrlichtBaWRendererApp::setGUIForConstantIoR()
{
    auto root = GUI->getRootWindow();

    auto* list = static_cast<::CEGUI::Combobox*>(root->getChild(
        "MaterialParamsWindow/MetallicDropDownList/DropDown_Metallic"));

    auto selection_Constant = list->getListboxItemFromIndex(0u);
    auto selection_current = list->getSelectedItem();
    if (selection_current != selection_Constant) // set metallic-source dropdown state to Constant
    {
        list->setItemSelectState(selection_Constant, true);
        list->setItemSelectState(selection_current, false);
    }
    auto slider = static_cast<::CEGUI::Slider*>(root->getChild("MaterialParamsWindow/MetallicWindow/Slider"));
    slider->setCurrentValue(0.f); // set slider of constant metallic value to 0
    root->getChild("MaterialParamsWindow/MetallicWindow/LabelPercent")->setText("N/A"); // display N/A as metallic constant value

    // appropriately set GUIState "cache"
    GUIState.Metallic.SourceDropdown = EDS_CONSTANT;
    GUIState.Metallic.ConstValue = 0.f;

    // disable metallic options in GUI
    root->getChild("MaterialParamsWindow/MetallicWindow")->setDisabled(true);
    root->getChild("MaterialParamsWindow/MetallicDropDownList")->setDisabled(true);
}

void IrrlichtBaWRendererApp::resetGUIAfterConstantIoR()
{
    auto root = GUI->getRootWindow();

    root->getChild("MaterialParamsWindow/MetallicWindow/LabelPercent")->setText("0.00");

    // re-enable metallic options
    root->getChild("MaterialParamsWindow/MetallicWindow")->setDisabled(false);
    root->getChild("MaterialParamsWindow/MetallicDropDownList")->setDisabled(false);
}

void IrrlichtBaWRendererApp::setLightPosition(const irr::core::vector3df& _lightPos)
{
    using namespace std::literals::string_literals;

    auto root = GUI->getRootWindow();
    const char* xyz[]{"X", "Y", "Z"};
    uint32_t i = 0u;
    for (const char* c : xyz)
    {
        auto lightCoord = static_cast<::CEGUI::Spinner*>(root->getChild("LightParamsWindow/PositionWindow/Light"s+c));
        lightCoord->setCurrentValue((&_lightPos.X)[i++]);
    }
    GUIState.Light.ConstantPosition = _lightPos;
}

void IrrlichtBaWRendererApp::renderGUI()
{
    GUI->render();
}

void IrrlichtBaWRendererApp::renderMesh()
{
    if (!Mesh)
        return;

    LightAnimData.update();

    CShaderManager::SParams params;
    params.constantAlbedo = (GUIState.Albedo.SourceDropdown==EDS_CONSTANT);
    params.constantMetallic = (GUIState.Metallic.SourceDropdown==EDS_CONSTANT);
    params.constantRI = (GUIState.RefractionIndex.SourceDropdown==EDS_CONSTANT);
    params.constantRoughness = (GUIState.Roughness.SourceDropdown==EDS_CONSTANT);
    params.AOEnabled = GUIState.AmbientOcclusion.Enabled;
    params.isotropicRoughness = GUIState.Roughness.IsIsotropic;
    params.metallicIsOne = (GUIState.Metallic.ConstValue==1.f);
    params.metallicIsZero = (GUIState.Metallic.ConstValue==0.f);
    params.roughnessIsZero = (GUIState.Roughness.ConstValue1==0.f);
    params.derivMapIsPresent = GUIState.BumpMapping.Enabled;

    // TODO pull the pipeline and gpu mesh and set it all
    //Material.MaterialType = ShaderManager->getShader(params);

    auto meshbuffer = Mesh->getMeshBuffer(MESHBUFFER_NUM);
    //Driver->setMaterial(Material);
    Driver->drawMeshBuffer(meshbuffer);
}

void IrrlichtBaWRendererApp::update()
{
    updateMaterial();
}

void IrrlichtBaWRendererApp::loadTextureSlot(ETEXTURE_SLOT slot, core::smart_refctd_ptr<irr::video::IGPUImageView> _texture, const std::string& _texName)
{
    auto tupl = TextureSlotMap[slot];
    auto root = GUI->getRootWindow();
    auto& renderer = GUI->getRenderer();

    auto image = _texture->getCreationParameters().image;
    auto extent = image->getCreationParameters().extent;

    auto gputex = _texture;
    ::CEGUI::Sizef texSize;
    texSize.d_width = extent.width;
    texSize.d_height = extent.height;

    //Material.setTexture(slot-TEXTURE_SLOT_1, gputex);
    if (slot == TEXTURE_AO)
        Textures.AO = gputex;
    else if (slot == TEXTURE_BUMP)
        Textures.BumpMap = gputex;
    else if (slot >= TEXTURE_SLOT_1)
        *(Textures.TextureViewer->begin() + slot-TEXTURE_SLOT_1) = gputex;
    updateMaterial();

    // TODO 
    // GPU interface should provide a way to get this around from the image
    auto objectNativeID = 0; 

    ::CEGUI::Texture& ceguiTexture = !renderer.isTextureDefined(_texName)
        ? irrTex2ceguiTex(objectNativeID, texSize, _texName, renderer)
        : renderer.getTexture(_texName);

   
    /*
    ::CEGUI::BasicImage& image = !::CEGUI::ImageManager::getSingleton().isDefined(std::get<0>(tupl))
        ? static_cast<::CEGUI::BasicImage&>(::CEGUI::ImageManager::getSingleton().create(
              "BasicImage", std::get<0>(tupl)))
        : static_cast<::CEGUI::BasicImage&>(
              ::CEGUI::ImageManager::getSingleton().get(std::get<0>(tupl)));
    image.setTexture(&ceguiTexture);
    image.setArea(::CEGUI::Rectf(0, 0, texSize.d_width, texSize.d_height));
    image.setAutoScaled(::CEGUI::AutoScaledMode::ASM_Both);
    */

    static const std::vector<const char*> property = { "NormalImage", "HoverImage", "PushedImage" };

    for (const auto& v : property) {
        root->getChild(std::get<2>(tupl))->setProperty(v, std::get<0>(tupl));
    }
}

void IrrlichtBaWRendererApp::loadTextureSlot_CPUTex(ETEXTURE_SLOT slot, core::smart_refctd_ptr<irr::asset::ICPUImage> _cputexture)
{
    if (!_cputexture.get())
        return;

    auto imageInfo = _cputexture->getCreationParameters();
    ICPUImageView::SCreationParams imageViewInfo;
    imageViewInfo.image = std::move(_cputexture);
    imageViewInfo.format = imageInfo.format;
    imageViewInfo.viewType = IImageView<ICPUImage>::E_TYPE::ET_2D;
    imageViewInfo.flags = static_cast<ICPUImageView::E_CREATE_FLAGS>(0u);
    imageViewInfo.subresourceRange.baseArrayLayer = 0u;
    imageViewInfo.subresourceRange.baseMipLevel = 0u;
    imageViewInfo.subresourceRange.layerCount = imageInfo.arrayLayers;
    imageViewInfo.subresourceRange.levelCount = imageInfo.mipLevels;

    auto imageView = ICPUImageView::create(std::move(imageViewInfo));

    auto textureBundle = asset::SAssetBundle({ imageView });
    auto gputexture = Driver->getGPUObjectsFromAssets(&imageView.get(), (&imageView.get())+1)->front();
    loadTextureSlot(slot, gputexture, textureBundle.getCacheKey()); 
}

core::smart_refctd_ptr<irr::asset::ICPUImage> IrrlichtBaWRendererApp::loadCPUTexture(const std::string& _path)
{
    irr::asset::IAssetLoader::SAssetLoadParams lparams;
    auto image = AssetManager->getAsset(_path, lparams).getContents().first[0];
    return core::smart_refctd_ptr_static_cast<asset::ICPUImage>(std::move(image));
}

auto IrrlichtBaWRendererApp::loadMesh(const std::string& _path) -> SCPUGPUMesh
{
    irr::asset::IAssetLoader::SAssetLoadParams lparams;
    auto cpumesh = core::smart_refctd_ptr_static_cast<asset::ICPUMesh>(AssetManager->getAsset(_path, lparams).getContents().first[0]);
    if (!cpumesh.get())
        return {nullptr, nullptr};
    cpumesh->getMeshBuffer(MESHBUFFER_NUM)->recalculateBoundingBox();

    auto gpumesh = Driver->getGPUObjectsFromAssets(&cpumesh.get(), (&cpumesh.get())+1)->front();

    return {cpumesh, gpumesh};
}

void IrrlichtBaWRendererApp::loadMeshAndReplaceTextures(const std::string& _path)
{
    auto loadedMesh = loadMesh(_path);
    if (!loadedMesh.cpu)
        return;

    Mesh = loadedMesh.gpu;

    irr::video::IGPUMeshBuffer* mb = Mesh->getMeshBuffer(MESHBUFFER_NUM);

    setUpLight();

    // TODO fetch pipeline
    //const irr::video::SGPUMaterial& itsMaterial = mb->getMaterial();
    /*
    for (uint32_t t = 0u; t < 4u; ++t)
    {
        if (Textures.TextureViewer[t]==DefaultTexture && itsMaterial.getTexture(t))
        {
            irr::video::IVirtualTexture* newtex = itsMaterial.getTexture(t);
            std::string texname = loadedMesh.cpu->getMeshBuffer(MESHBUFFER_NUM)->getMaterial().getTexture(t)->getCacheKey();
            loadTextureSlot(static_cast<ETEXTURE_SLOT>(TEXTURE_SLOT_1 + t), newtex, texname);
        }
    }
    */
}

void IrrlichtBaWRendererApp::setUpLight(float radiusMlt)
{
    irr::video::IGPUMeshBuffer* mb = Mesh->getMeshBuffer(MESHBUFFER_NUM);
    const irr::core::aabbox3df& aabb = mb->getBoundingBox();
    const irr::core::vector3df aabb_sz = aabb.getExtent();
    LightAnimData.Radius = std::max(aabb_sz.X, aabb_sz.Z) / 2.f;
    LightAnimData.Radius *= radiusMlt;
    irr::core::vector3df aabb_verts[8];
    aabb.getEdges(aabb_verts);
    auto lowest_highest = std::minmax_element(aabb_verts, aabb_verts+8, [](const irr::core::vector3df& a, const irr::core::vector3df& b) { return a.Y < b.Y; });
    LightAnimData.Position.Y = (lowest_highest.first->Y + lowest_highest.second->Y) / 2.f;
    irr::core::vector2df center;
    for (uint32_t i = 0u; i < 8u; ++i)
    {
        center.X += aabb_verts[i].X;
        center.Y += aabb_verts[i].Y;
    }
    LightAnimData.Center = center / 8.f;

    setLightPosition(aabb_sz*irr::core::vector3df{0.5f, -1.f, 0.5f});
}

void IrrlichtBaWRendererApp::updateTooltip(const char* name, const char* text)
{
    std::string s(text);
    ext::cegui::Replace(s, "\\", "\\\\");

    static_cast<CEGUI::DefaultWindow*>(GUI->getRootWindow()->getChild(name))
        ->setTooltipText(s.c_str());
}

auto IrrlichtBaWRendererApp::getDropdownState(const char* _dropdownName) const -> E_DROPDOWN_STATE
{
    auto root = GUI->getRootWindow();
    auto* list = static_cast<::CEGUI::Combobox*>(root->getChild(_dropdownName));

    auto mapStrToEnum = [] (const std::string& _str) {
        const char* Texture = "Texture";
        if (_str.compare(0, strlen(Texture), Texture) == 0)
            return static_cast<E_DROPDOWN_STATE>(EDS_TEX0 + _str[strlen(Texture)+1]-'0');
        else return EDS_CONSTANT;
    };

    return mapStrToEnum(list->getSelectedItem()->getText());
}

void IrrlichtBaWRendererApp::showErrorMessage(const char* title, const char* message)
{
    auto root = GUI->getRootWindow();
    if (!root->isChild("MessageBoxRoot")) {
        CEGUI::Window* layout = CEGUI::WindowManager::getSingleton().loadLayoutFromFile(
            "MessageBox.layout");
        layout->setVisible(false);
        layout->setAlwaysOnTop(true);
        layout->setSize(
            CEGUI::USize(CEGUI::UDim(0.5, 0.0f), CEGUI::UDim(0.2f, 0.0f)));
        layout->setHorizontalAlignment(CEGUI::HA_CENTRE);
        layout->setVerticalAlignment(CEGUI::VA_CENTRE);

        static_cast<CEGUI::PushButton*>(
            layout->getChild("FrameWindow/ButtonWindow/Button"))
            ->subscribeEvent(
                CEGUI::PushButton::EventClicked,
                [root](const CEGUI::EventArgs&) {
                    root->getChild("MessageBoxRoot")->setVisible(false);
                });

        root->addChild(layout);
    }

    auto header = static_cast<CEGUI::DefaultWindow*>(root->getChild("MessageBoxRoot"));
    header->setVisible(true);
    header->activate();

    auto frame = static_cast<CEGUI::FrameWindow*>(header->getChild("FrameWindow"));
    frame->setText(title);
    static_cast<CEGUI::DefaultWindow*>(frame->getChild("Label"))
        ->setText(message);
}

void IrrlichtBaWRendererApp::updateMaterial()
{
    auto common = [this](E_DROPDOWN_STATE texnum, uint32_t texunit) {
        uint32_t ix = texnum - EDS_TEX0;
        //if (Textures.TextureViewer[ix] && Textures.TextureViewer[ix] != DefaultTexture)   TODO descriptor sets 
            //Material.setTexture(texunit, Textures.TextureViewer[ix]);
    };
    if (GUIState.Albedo.SourceDropdown != EDS_CONSTANT)
        common(GUIState.Albedo.SourceDropdown, ALBEDO_MAP_TEX_UNIT);
    if (GUIState.Roughness.SourceDropdown != EDS_CONSTANT)
        common(GUIState.Roughness.SourceDropdown, ROUGHNESS_MAP_TEX_UNIT);
    if (GUIState.RefractionIndex.SourceDropdown != EDS_CONSTANT)
        common(GUIState.RefractionIndex.SourceDropdown, IOR_MAP_TEX_UNIT);
    if (GUIState.Metallic.SourceDropdown != EDS_CONSTANT)
        common(GUIState.Metallic.SourceDropdown, METALLIC_MAP_TEX_UNIT);

    constexpr float WAIT_TIME = 0.3f; //seconds

    float timeSinceChange = std::chrono::duration_cast<Duration>(Clock::now() - DerivMapGeneration.TimePointLastHeightFactorChange).count();
    if (Textures.BumpMap &&
        Textures.BumpMap != DefaultTexture &&
        DerivMapGeneration.HeightFactorChanged &&
        timeSinceChange >= WAIT_TIME
        ) {
        DerivativeMapManager->getDerivativeMap(Textures.BumpMap, GUIState.BumpMapping.Height);
        DerivMapGeneration.HeightFactorChanged = false;
        //Material.setTexture(DERIV_MAP_TEX_UNIT, DerivativeMapManager->getDerivativeMap(Textures.BumpMap, GUIState.BumpMapping.Height));
    }

    //if (Textures.AO && Textures.AO != DefaultTexture)
        //Material.setTexture(AO_MAP_TEX_UNIT, Textures.AO);
}

void IrrlichtBaWRendererApp::eventAOTextureBrowse(const ::CEGUI::EventArgs&)
{
    const auto p = GUI->openFileDialog(ImageFileDialogTitle, ImageFileDialogFilters);

    if (p.first) {
        auto box = static_cast<CEGUI::Editbox*>(
            GUI->getRootWindow()->getChild("MaterialParamsWindow/AOWindow/Editbox"));

        loadTextureSlot_CPUTex(ETEXTURE_SLOT::TEXTURE_AO, loadCPUTexture(p.second));

        box->setText(p.second);
        /*updateTooltip(
            "MaterialParamsWindow/AOWindow/ImageButton",
            ext::cegui::ssprintf("%s (%ux%u)\nLeft-click to select a new texture.",
                p.second.c_str(), cputexture->getSize()[0], cputexture->getSize()[1])
                .c_str());*/
    }
}

void IrrlichtBaWRendererApp::eventAOTextureBrowse_EditBox(const ::CEGUI::EventArgs&)
{
    auto box = static_cast<CEGUI::Editbox*>(
        GUI->getRootWindow()->getChild("MaterialParamsWindow/AOWindow/Editbox"));

    if (ext::cegui::Exists(box->getText().c_str())) {
        loadTextureSlot_CPUTex(ETEXTURE_SLOT::TEXTURE_AO, loadCPUTexture(box->getText()));

        /*updateTooltip(
            "MaterialParamsWindow/AOWindow/ImageButton",
            irr::ext::cegui::ssprintf("%s (%ux%u)\nLeft-click to select a new texture.",
                box->getText().c_str(), cputexture->getSize()[0], cputexture->getSize()[1])
                .c_str());*/
    } else {
        std::string s;
        s += std::string(box->getText().c_str()) + ": The file couldn't be opened.";
        ext::cegui::Replace(s, "\\", "\\\\");
        showErrorMessage("Error", s.c_str());
    }
}

void IrrlichtBaWRendererApp::eventBumpTextureBrowse(const ::CEGUI::EventArgs&)
{
    const auto p = GUI->openFileDialog(ImageFileDialogTitle, ImageFileDialogFilters);

    if (p.first) {
        auto box = static_cast<CEGUI::Editbox*>(
            GUI->getRootWindow()->getChild("MaterialParamsWindow/BumpWindow/Editbox"));
        
        loadTextureSlot_CPUTex(ETEXTURE_SLOT::TEXTURE_BUMP, loadCPUTexture(p.second));

        box->setText(p.second);
        /*updateTooltip(
            "MaterialParamsWindow/BumpWindow/ImageButton",
            ext::cegui::ssprintf("%s (%ux%u)\nLeft-click to select a new texture.",
                p.second.c_str(), cputexture->getSize()[0], cputexture->getSize()[1])
                .c_str());*/
    }
}

void IrrlichtBaWRendererApp::eventBumpTextureBrowse_EditBox(const ::CEGUI::EventArgs&)
{
    auto box = static_cast<CEGUI::Editbox*>(
        GUI->getRootWindow()->getChild("MaterialParamsWindow/BumpWindow/Editbox"));

    if (ext::cegui::Exists(box->getText().c_str())) {
        loadTextureSlot_CPUTex(ETEXTURE_SLOT::TEXTURE_BUMP, loadCPUTexture(box->getText()));

        /*updateTooltip(
            "MaterialParamsWindow/BumpWindow/ImageButton",
            ext::cegui::ssprintf("%s (%ux%u)\nLeft-click to select a new texture.",
                box->getText().c_str(), cputexture->getSize()[0], cputexture->getSize()[1])
                .c_str());*/
    } else {
        std::string s;
        s += std::string(box->getText().c_str()) + ": The file couldn't be opened.";
        ext::cegui::Replace(s, "\\", "\\\\");
        showErrorMessage("Error", s.c_str());
    }
}

void IrrlichtBaWRendererApp::eventTextureBrowse(const CEGUI::EventArgs& e)
{
    const CEGUI::WindowEventArgs& we = static_cast<const CEGUI::WindowEventArgs&>(e);
    const auto parent = static_cast<CEGUI::PushButton*>(we.window)->getParent()->getName();
    const auto p = GUI->openFileDialog(ImageFileDialogTitle, ImageFileDialogFilters);


    const auto path_label = ext::cegui::ssprintf("TextureViewWindow/%s/LabelWindow/Label", parent.c_str());
    const auto path_texture = ext::cegui::ssprintf("TextureViewWindow/%s/Texture", parent.c_str());

    if (p.first) {
        auto box = static_cast<CEGUI::Editbox*>(GUI->getRootWindow()->getChild(path_label));
        const auto v = ext::cegui::Split(p.second, '\\');

        ETEXTURE_SLOT type;
        if (parent == "Texture0Window")
            type = ETEXTURE_SLOT::TEXTURE_SLOT_1;
        else if (parent == "Texture1Window")
            type = ETEXTURE_SLOT::TEXTURE_SLOT_2;
        else if (parent == "Texture2Window")
            type = ETEXTURE_SLOT::TEXTURE_SLOT_3;
        else if (parent == "Texture3Window")
            type = ETEXTURE_SLOT::TEXTURE_SLOT_4;

        loadTextureSlot_CPUTex(type, loadCPUTexture(p.second));

        box->setText(v[v.size() - 1]);
        /*updateTooltip(
            path_texture.c_str(),
            ext::cegui::ssprintf("%s (%ux%u)\nLeft-click to select a new texture.",
                p.second.c_str(), cputexture->getSize()[0], cputexture->getSize()[1])
                .c_str());*/
    }
}

void IrrlichtBaWRendererApp::eventMeshBrowse(const CEGUI::EventArgs& e)
{
    const auto p = GUI->openFileDialog(MeshFileDialogTitle, MeshFileDialogFilters);

    if (p.first)
    {
        loadMeshAndReplaceTextures(p.second);
    }
}


IrrlichtBaWRendererApp::~IrrlichtBaWRendererApp()
{
    delete DerivativeMapManager;
    delete ShaderManager;
}

} // namespace irr
