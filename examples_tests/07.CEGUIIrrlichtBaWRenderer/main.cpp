#define _IRR_STATIC_LIB_

#include <irrlicht.h>
#include "IrrlichtBaWRendererApp.h"
#include "CIrrlichtBaWRendererIncludeLoader.h"

using namespace irr;

int main()
{
    // create device with full flexibility over creation parameters
    // you can add more parameters if desired, check irr::SIrrlichtCreationParameters
    irr::SIrrlichtCreationParameters params;
    params.Bits = 24; //may have to set to 32bit for some platforms
    params.ZBufferBits = 24; //we'd like 32bit here
    params.DriverType = video::EDT_OPENGL; //! Only Well functioning driver, software renderer left for sake of 2D image drawing
    params.WindowSize = core::dimension2d<uint32_t>(1280, 720);
    params.Fullscreen = false;
    params.Vsync = true; //! If supported by target platform
    params.Doublebuffer = true;
    params.Stencilbuffer = false; //! This will not even be a choice soon
    params.AuxGLContexts = 16;
    auto device = createDeviceEx(params);
    device->setWindowCaption(L"CEGUI IrrlichtBaW Renderer");
    device->getCursorControl()->setVisible(false);

    if (!device.get())
        return 1; // could not create selected driver.

    auto driver = device->getVideoDriver();
    auto smgr = device->getSceneManager();
    auto am = device->getAssetManager();

    scene::ICameraSceneNode* camera = smgr->addCameraSceneNodeModifiedMaya(nullptr, -400.f, 20.f, 200.f, -1, 28.f, 1.f);
    camera->setNearValue(0.01f);
    camera->setFarValue(1000.0f);
    smgr->setActiveCamera(camera);

    {
        auto brdfBuiltinLoader = core::make_smart_refctd_ptr<CIrrlichtBaWRendererIncludeLoader>();
        am->getGLSLCompiler()->getIncludeHandler()->addBuiltinIncludeLoader(brdfBuiltinLoader);
    }

    auto irrlichtBawApp = new IrrlichtBaWRendererApp(device.get(), camera);

    uint64_t lastFPSTime = 0;

    while(device->run())
    if (device->isWindowActive())
    {
        driver->beginScene(true, true, video::SColor(255,0,0,0) );

        // needed for camera to move
        smgr->drawAll();

        irrlichtBawApp->update();
        //irrlichtBawApp->renderMesh();
        irrlichtBawApp->renderGUI();
        driver->endScene();
    }
    
    delete irrlichtBawApp;

    return 0;
}
