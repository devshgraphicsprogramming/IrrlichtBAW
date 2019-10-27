#define _IRR_STATIC_LIB_

#include <irrlicht.h>

#include "BSDFValidatorApp.h"

#include <iostream>
#include <memory>

using namespace irr;


int main()
{
    // create device with full flexibility over creation parameters
    // you can add more parameters if desired, check irr::SIrrlichtCreationParameters
    irr::SIrrlichtCreationParameters params;
    params.Bits = 24; //may have to set to 32bit for some platforms
    params.ZBufferBits = 24; //we'd like 32bit here
    params.DriverType = video::EDT_OPENGL; //! Only Well functioning driver, software renderer left for sake of 2D image drawing
    params.WindowSize = core::dimension2d<uint32_t>(512, 512);
    params.Fullscreen = false;
    params.Vsync = true; //! If supported by target platform
    params.Doublebuffer = true;
    params.Stencilbuffer = false; //! This will not even be a choice soon
    params.AuxGLContexts = 16;

    IrrlichtDevice* device = createDeviceEx(params);
    if (device == 0)
        return 1; // could not create selected driver.

    video::IVideoDriver* driver = device->getVideoDriver();
    scene::ISceneManager* smgr = device->getSceneManager();

    std::unique_ptr<BSDFValidatorApp> bsdfValidatorApp(new BSDFValidatorApp(device));

    uint64_t lastFPSTime = 0;

    while (device->run())
    {
        if (device->isWindowActive())
        {
            driver->beginScene(true, true, video::SColor(255, 0, 0, 0));
            bsdfValidatorApp->RenderGUI();
            bsdfValidatorApp->RenderMesh();
            driver->endScene();
        }
    }
    device->drop();

    return 0;
}
