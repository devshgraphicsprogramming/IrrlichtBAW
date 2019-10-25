#define _IRR_STATIC_LIB_

#include <irrlicht.h>

#include "BSDFValidatorApp.h"

#include <iostream>
#include <memory>

using namespace irr;

#include "irr/irrpack.h"
struct VertexStruct
{
    float position[3]; /// uses float hence need 4 byte alignment
} PACK_STRUCT;
#include "irr/irrunpack.h"


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

    IrrlichtDevice* device = createDeviceEx(params);
    if (device == 0)
        return 1; // could not create selected driver.

    video::IVideoDriver* driver = device->getVideoDriver();
    scene::ISceneManager* smgr = device->getSceneManager();

    video::SGPUMaterial material;
    material.BackfaceCulling = false; //! Triangles will be visible from both sides
    material.MaterialType = (video::E_MATERIAL_TYPE)driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles("../Shader.vert",
        "", "", "", //! No Geometry or Tessellation Shaders
        "../Shader.frag"); //! No custom user data

    driver->setMaterial(material);

    std::unique_ptr<BSDFValidatorApp> bsdfValidatorApp(new BSDFValidatorApp(*device));

    // This whole mesh business is just to check if the custom shaders
    // are woring as intended. It may or may not make it to the final PR.
    //
    // Create mesh.
    VertexStruct vertices[4] =
    {
        { -0.5f, -0.5f, 0.f },
        { 0.5f, -0.5f, 0.f },
        { 0.5f, 0.5f, 0.f },
        { -0.5f, 0.5f, 0.f }
    };

    uint16_t indices[6] =
    {
        0, 1, 2,
        2, 3, 0
    };

    // Upload mesh to the GPU
    auto upStreamBuff = driver->getDefaultUpStreamingBuffer();
    const void* dataToPlace[2] = { vertices, indices };
    uint32_t offsets[2] = { video::StreamingTransientDataBufferMT<>::invalid_address,video::StreamingTransientDataBufferMT<>::invalid_address };
    uint32_t alignments[2] = { sizeof(decltype(vertices[0u])),sizeof(decltype(indices[0u])) };
    uint32_t sizes[2] = { sizeof(vertices),sizeof(indices) };
    upStreamBuff->multi_place(2u, (const void* const*)dataToPlace, offsets, sizes, alignments);

    if (upStreamBuff->needsManualFlushOrInvalidate())
    {
        auto upStreamMem = upStreamBuff->getBuffer()->getBoundMemory();
        driver->flushMappedMemoryRanges({ video::IDriverMemoryAllocation::MappedMemoryRange(upStreamMem, offsets[0],sizes[0]),video::IDriverMemoryAllocation::MappedMemoryRange(upStreamMem,offsets[1],sizes[1]) });
    }

    auto meshBuffer = core::make_smart_refctd_ptr<video::IGPUMeshBuffer>();

    {
        // Define vertex attribute layout. 
        auto desc = driver->createGPUMeshDataFormatDesc();

        {
            auto buff = core::smart_refctd_ptr<video::IGPUBuffer>(upStreamBuff->getBuffer());

            // Position attribute.
            desc->setVertexAttrBuffer(core::smart_refctd_ptr(buff), asset::EVAI_ATTR0, asset::EF_R32G32B32_SFLOAT, sizeof(VertexStruct), 0);

            desc->setIndexBuffer(std::move(buff));
        }

        meshBuffer->setIndexBufferOffset(offsets[1]);
        meshBuffer->setIndexType(asset::EIT_16BIT);
        meshBuffer->setIndexCount(6);
        meshBuffer->setMeshDataAndFormat(std::move(desc));
    }

    driver->setTransform(video::E4X3TS_WORLD, core::matrix4x3());

    upStreamBuff->multi_free(2u, (uint32_t*)&offsets, (uint32_t*)&sizes, driver->placeFence()); 

    uint64_t lastFPSTime = 0;

    while (device->run())
    {
        if (device->isWindowActive())
        {
            driver->beginScene(true, true, video::SColor(255, 0, 0, 0));
            driver->drawMeshBuffer(meshBuffer.get());
            bsdfValidatorApp->RenderGUI();
            driver->endScene();
        }
    }
    device->drop();

    return 0;
}
