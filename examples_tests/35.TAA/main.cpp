#define _IRR_STATIC_LIB_
#include <irrlicht.h>
#include "../ext/FullScreenTriangle/FullScreenTriangle.h"

#define USE_JITTER 1

using namespace irr;
using namespace core;

bool quit = false;
//!Same As Last Example
class MyEventReceiver : public IEventReceiver
{
public:

	MyEventReceiver()
	{
	}

	bool OnEvent(const SEvent& event)
	{
        if (event.EventType == irr::EET_KEY_INPUT_EVENT && !event.KeyInput.PressedDown)
        {
            switch (event.KeyInput.Key)
            {
            case irr::KEY_KEY_Q: // switch wire frame mode
                quit = true;
                return true;
            default:
                break;
            }
        }

		return false;
	}

private:
};

// GLOBAL CONSTANTS
static const dimension2d<uint32_t> WINDOW_SIZE(1280, 720);
static const uint32_t WINDOWS_SIZE_array[]{ 1280, 720 };
static const core::vector2df WINDOW_SIZE_vec2df(1280.f, 720.f);
constexpr uint32_t JITTER_OFFSET_CNT = 8u;
static const core::vector2df JITTER_OFFSETS[JITTER_OFFSET_CNT]{
        core::vector2df(-7.0f, 1.0f) / (8.f * WINDOW_SIZE_vec2df),
        core::vector2df(-5.0f, -5.0f) / (8.f * WINDOW_SIZE_vec2df),
        core::vector2df(-1.0f, -3.0f) / (8.f * WINDOW_SIZE_vec2df),
        core::vector2df(3.0f, -7.0f) / (8.f * WINDOW_SIZE_vec2df),
        core::vector2df(5.0f, -1.0f) / (8.f * WINDOW_SIZE_vec2df),
        core::vector2df(7.0f, 7.0f) / (8.f * WINDOW_SIZE_vec2df),
        core::vector2df(1.0f, 3.0f) / (8.f * WINDOW_SIZE_vec2df),
        core::vector2df(-3.0f, 5.0f) / (8.f * WINDOW_SIZE_vec2df)
};
// GLOBAL VARS
static uint32_t g_FrameNum = 0u;

class SimpleCallBack : public video::IShaderConstantSetCallBack
{
    int32_t mvpUniformLocation;
    int32_t cameraDirUniformLocation;
    int32_t texUniformLocation[4];
    video::E_SHADER_CONSTANT_TYPE mvpUniformType;
    video::E_SHADER_CONSTANT_TYPE cameraDirUniformType;
    video::E_SHADER_CONSTANT_TYPE texUniformType[4];
public:
    SimpleCallBack() : cameraDirUniformLocation(-1), cameraDirUniformType(video::ESCT_FLOAT_VEC3) {}

    virtual void PostLink(video::IMaterialRendererServices* services, const video::E_MATERIAL_TYPE& materialType, const core::vector<video::SConstantLocationNamePair>& constants)
    {
        for (size_t i=0; i<constants.size(); i++)
        {
            if (constants[i].name=="MVP")
            {
                mvpUniformLocation = constants[i].location;
                mvpUniformType = constants[i].type;
            }
            else if (constants[i].name=="cameraPos")
            {
                cameraDirUniformLocation = constants[i].location;
                cameraDirUniformType = constants[i].type;
            }
        }
    }

    virtual void OnSetConstants(video::IMaterialRendererServices* services, int32_t userData)
    {
        core::vectorSIMDf modelSpaceCamPos;
        modelSpaceCamPos.set(services->getVideoDriver()->getTransform(video::E4X3TS_WORLD_VIEW_INVERSE).getTranslation());
        services->setShaderConstant(&modelSpaceCamPos,cameraDirUniformLocation,cameraDirUniformType,1);
        services->setShaderConstant(services->getVideoDriver()->getTransform(video::EPTS_PROJ_VIEW_WORLD).pointer(),mvpUniformLocation,mvpUniformType,1);
    }

    virtual void OnUnsetMaterial() {}
};

class VelocityBufCallBack : public video::IShaderConstantSetCallBack
{
    int32_t mvpUniformLocation = -1;
    int32_t prevMvpUniformLocation = -1;
    video::E_SHADER_CONSTANT_TYPE mvpUniformType = video::ESCT_FLOAT;
    video::E_SHADER_CONSTANT_TYPE prevMvpUniformType = video::ESCT_FLOAT;

    core::matrix4SIMD m_VP;

public:
    virtual void PostLink(video::IMaterialRendererServices* services, const video::E_MATERIAL_TYPE& materialType, const core::vector<video::SConstantLocationNamePair>& constants)
    {
        for (size_t i=0; i<constants.size(); i++)
        {
            if (constants[i].name=="MVP")
            {
                mvpUniformLocation = constants[i].location;
                mvpUniformType = constants[i].type;
            }
            else if (constants[i].name=="prevMVP")
            {
                prevMvpUniformLocation = constants[i].location;
                prevMvpUniformType = constants[i].type;
            }
        }
    }

    virtual void OnSetConstants(video::IMaterialRendererServices* services, int32_t userData)
    {
        core::matrix4SIMD prevVP;
        if (g_FrameNum==0u)
        {
            prevVP = m_VP = services->getVideoDriver()->getTransform(video::EPTS_PROJ_VIEW);
        }
        else
        {
            prevVP = m_VP;
            m_VP = services->getVideoDriver()->getTransform(video::EPTS_PROJ_VIEW);
        }
#if USE_JITTER
        core::matrix4SIMD jitter;
        float jitterOffset[3]{0.f,0.f,0.f};
        memcpy(jitterOffset, &JITTER_OFFSETS[g_FrameNum%JITTER_OFFSET_CNT].X, 8);
        jitter.setTranslation(jitterOffset);
        m_VP = core::matrix4SIMD::concatenateBFollowedByA(jitter, m_VP);
#endif//USE_JITTER
        
        // will work only with assumption of static scenes (only camera movement) - i.e. world matrix is constant throughout frames
        // for it to work with moving objects, i would need to cache world matrices per scenenode somewhere
        auto world = services->getVideoDriver()->getTransform(video::E4X3TS_WORLD);
        auto mvp = core::concatenateBFollowedByA(m_VP, world);
        auto prevMVP = core::concatenateBFollowedByA(prevVP, world);
        services->setShaderConstant(mvp.pointer(), mvpUniformLocation, mvpUniformType, 1);
        services->setShaderConstant(prevMVP.pointer(), prevMvpUniformLocation, prevMvpUniformType, 1);
    }

    virtual void OnUnsetMaterial() {}
};

class TAACallback : public video::IShaderConstantSetCallBack
{
    int32_t jitterUniformLocation = 0;
    video::E_SHADER_CONSTANT_TYPE jitterUniformType = video::ESCT_FLOAT_VEC2;

public:
    virtual void PostLink(video::IMaterialRendererServices* services, const video::E_MATERIAL_TYPE& materialType, const core::vector<video::SConstantLocationNamePair>& constants)
    {
    }

    virtual void OnSetConstants(video::IMaterialRendererServices* services, int32_t userData)
    {
        const float* jitter{};
#if USE_JITTER
        jitter = &JITTER_OFFSETS[g_FrameNum%JITTER_OFFSET_CNT].X;
#else
        float zero[2]{};
        jitter = zero;
#endif
        services->setShaderConstant(jitter, jitterUniformLocation, jitterUniformType, 1);
    }

    virtual void OnUnsetMaterial() {}
};

int main()
{
    srand(time(0));
	// create device with full flexibility over creation parameters
	// you can add more parameters if desired, check irr::SIrrlichtCreationParameters
	irr::SIrrlichtCreationParameters params;
	params.Bits = 24; //may have to set to 32bit for some platforms
	params.ZBufferBits = 24; //we'd like 32bit here
	params.DriverType = video::EDT_OPENGL; //! Only Well functioning driver, software renderer left for sake of 2D image drawing
	params.WindowSize = WINDOW_SIZE;
	params.Fullscreen = false;
	params.Vsync = true; //! If supported by target platform
	params.Doublebuffer = true;
	params.Stencilbuffer = false; //! This will not even be a choice soon
	IrrlichtDevice* device = createDeviceEx(params);

	if (device == 0)
		return 1; // could not create selected driver.


	video::IVideoDriver* driver = device->getVideoDriver();

    SimpleCallBack* cb = new SimpleCallBack();
    video::E_MATERIAL_TYPE renderMaterialType = (video::E_MATERIAL_TYPE)driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
                                                        "../mesh.vert",
                                                        "","","",
                                                        "../mesh.frag",
                                                        3,video::EMT_SOLID,
                                                        cb,
                                                        0);
    cb->drop();
    VelocityBufCallBack* velBufCb = new VelocityBufCallBack();
    video::E_MATERIAL_TYPE velBufMaterialType = (video::E_MATERIAL_TYPE)driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
                                                        "../velbuf.vert",
                                                        "","","",
                                                        "../velbuf.frag",
                                                        3,video::EMT_SOLID,
                                                        velBufCb,
                                                        0);
    velBufCb->drop();
    TAACallback* TAAcb = new TAACallback();
    video::E_MATERIAL_TYPE TAAMaterialType = (video::E_MATERIAL_TYPE)driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
                                                        "../quad.vert",
                                                        "","","",
                                                        "../taa.frag",
                                                        3,video::EMT_SOLID,
                                                        TAAcb,
                                                        0);
    TAAcb->drop();

	scene::ISceneManager* smgr = device->getSceneManager();
	driver->setTextureCreationFlag(video::ETCF_ALWAYS_32_BIT, true);
	scene::ICameraSceneNode* camera =
		smgr->addCameraSceneNodeFPS(0,100.0f,0.01f);
	camera->setPosition(core::vector3df(-4,0,0));
	camera->setTarget(core::vector3df(0,0,0));
	camera->setNearValue(0.01f);
	camera->setFarValue(100.0f);
    smgr->setActiveCamera(camera);
	device->getCursorControl()->setVisible(false);
	MyEventReceiver receiver;
	device->setEventReceiver(&receiver);

    asset::IAssetManager& am = device->getAssetManager();

    scene::ISceneNode* meshSceneNodes[2]{};

    asset::IAssetLoader::SAssetLoadParams lparams;
    asset::ICPUMesh* cpumesh = static_cast<asset::ICPUMesh*>(am.getAsset("../../media/extrusionLogo_TEST_fixed.stl", lparams));

    if (cpumesh)
    {
        video::IGPUMesh* gpumesh = driver->getGPUObjectsFromAssets(&cpumesh, (&cpumesh)+1)[0];
        meshSceneNodes[0] = smgr->addMeshSceneNode(gpumesh);
        meshSceneNodes[0]->setMaterialType(renderMaterialType);
        gpumesh->drop();
    }

    cpumesh = static_cast<asset::ICPUMesh*>(am.getAsset("../../media/cow.obj", lparams));

    if (cpumesh)
    {
        video::IGPUMesh* gpumesh = driver->getGPUObjectsFromAssets(&cpumesh, (&cpumesh)+1)[0];
        meshSceneNodes[1] = smgr->addMeshSceneNode(gpumesh,0,-1,core::vector3df(3.f,1.f,0.f));
        meshSceneNodes[1]->setMaterialType(renderMaterialType);
        gpumesh->drop();
    }

    video::IGPUMeshBuffer* fullscreenTriMeshBuf = ext::FullScreenTriangle::createFullScreenTriangle(driver);

    auto velocityBuf = driver->createGPUTexture(video::ITexture::ETT_2D, WINDOWS_SIZE_array, 1u, asset::EF_R16G16_SNORM); // try 8bit unorm later
    auto colorBuf = driver->createGPUTexture(video::ITexture::ETT_2D, WINDOWS_SIZE_array, 1u, asset::EF_R8G8B8_UNORM);
    video::ITexture* historyBuffers[2]{
        driver->createGPUTexture(video::ITexture::ETT_2D, WINDOWS_SIZE_array, 1u, asset::EF_R8G8B8_UNORM),
        driver->createGPUTexture(video::ITexture::ETT_2D, WINDOWS_SIZE_array, 1u, asset::EF_R8G8B8_UNORM)
    };
    auto tmpColorBuf = driver->createGPUTexture(video::ITexture::ETT_2D, WINDOWS_SIZE_array, 1u, asset::EF_R8G8B8_UNORM);
    auto depthBuf = driver->createGPUTexture(video::ITexture::ETT_2D, WINDOWS_SIZE_array, 1u, asset::EF_D16_UNORM);

    //TODO maybe better to use separate fbo for each pass? (wouldn't need to reattach every pass every frame)
    video::IFrameBuffer* fbo = driver->addFrameBuffer();

    video::SGPUMaterial TAAMaterial;
    video::STextureSamplingParams sparams;
    sparams.UseMipmaps = 0u;
    sparams.MinFilter = sparams.MaxFilter = video::ETFT_LINEAR_NO_MIP;
    sparams.TextureWrapU = sparams.TextureWrapV = video::ETC_MIRROR_CLAMP_TO_EDGE;
    TAAMaterial.setTexture(0u, depthBuf);
    TAAMaterial.TextureLayer[0].SamplingParams = sparams;
    TAAMaterial.setTexture(1u, colorBuf);
    TAAMaterial.TextureLayer[1].SamplingParams = sparams;
    TAAMaterial.setTexture(2u, historyBuffers[1]);
    TAAMaterial.TextureLayer[2].SamplingParams = sparams;
    TAAMaterial.setTexture(3u, velocityBuf);
    TAAMaterial.TextureLayer[3].SamplingParams = sparams;
    TAAMaterial.MaterialType = TAAMaterialType;

	uint64_t lastFPSTime = 0;

	while(!quit && device->run())
	{
		driver->beginScene(false, false, video::SColor(255,255,255,255) );

        const float clearVel[4]{ 0.f, 0.f, 0.f, 0.f };
        const float clearColor[4]{ 0.f, 0.f, 1.f, 1.f };
        const float clearDepth = 0.f;

        // VELOCITY BUF PASS
        if (g_FrameNum > 0u) //first frame is without AA
        {
            for (auto node : meshSceneNodes)
                node->setMaterialType(velBufMaterialType);
            fbo->attach(video::EFAP_COLOR_ATTACHMENT0, velocityBuf);
            fbo->attach(video::EFAP_DEPTH_ATTACHMENT, depthBuf);
            driver->setRenderTarget(fbo);
            driver->clearZBuffer(clearDepth);
            driver->clearColorBuffer(video::EFAP_COLOR_ATTACHMENT0, clearVel);
            smgr->drawAll();
        }

        // COLOR PASS
        for (auto node : meshSceneNodes)
            node->setMaterialType(renderMaterialType);
        //first frame is without AA
        //and not AA'd result goes to 2nd frame as history buffer
        fbo->attach(video::EFAP_COLOR_ATTACHMENT0, g_FrameNum>0u ? colorBuf : historyBuffers[0]);
        fbo->attach(video::EFAP_DEPTH_ATTACHMENT, depthBuf);
        driver->setRenderTarget(fbo);
        driver->clearZBuffer(clearDepth);
        driver->clearColorBuffer(video::EFAP_COLOR_ATTACHMENT0, clearColor);
        smgr->drawAll();

        // AA PASS
        if (g_FrameNum > 0u) //first frame is without AA
        {
            fbo->attach(video::EFAP_COLOR_ATTACHMENT0, tmpColorBuf);
            fbo->attach(video::EFAP_COLOR_ATTACHMENT1, historyBuffers[g_FrameNum&1u]);
            fbo->attach(video::EFAP_DEPTH_ATTACHMENT, static_cast<video::ITexture*>(nullptr));
            driver->setRenderTarget(fbo);
            driver->clearZBuffer(clearDepth);
            TAAMaterial.setTexture(2u, historyBuffers[(g_FrameNum+1u)&1u]);
            driver->setMaterial(TAAMaterial);
            driver->drawMeshBuffer(fullscreenTriMeshBuf);
        }

        // copy result to screen
        // needed because TAA pass needs to write not only to screen, but also to history buffer
        driver->blitRenderTargets(fbo, nullptr, false, false);

		driver->endScene();

		// display frames per second in window title
		uint64_t time = device->getTimer()->getRealTime();
		if (time-lastFPSTime > 1000ull)
		{
			std::wostringstream sstr;
			sstr << L"Builtin Nodes Demo - Irrlicht Engine FPS:" << driver->getFPS() << " PrimitvesDrawn:" << driver->getPrimitiveCountDrawn();

			device->setWindowCaption(sstr.str().c_str());
			lastFPSTime = time;
		}
        ++g_FrameNum;
	}
    
	device->drop();

	return 0;
}
