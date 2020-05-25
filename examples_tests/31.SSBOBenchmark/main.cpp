#define _IRR_STATIC_LIB_
#include <iostream>
#include <irrlicht.h>
#include "../source/Irrlicht/COpenGLExtensionHandler.h"
#include "../source/Irrlicht/COpenGLBuffer.h"
#include "../source/Irrlicht/COpenGLDriver.h"
//#include "../source/Irrlicht/COpenGLTextureBufferObject.h"

#include <irrlicht.h>

#include "../common/QToQuitEventReceiver.h"
#include "../source/Irrlicht/COpenGLExtensionHandler.h"

#include <random>

using namespace irr;
using namespace core;
using namespace asset;
using namespace video;

size_t convertBuf1(const float* _src, float* _dst, const size_t _boneCnt, const size_t _instCnt)
{
    for (size_t i = 0u; i < _boneCnt; ++i)
        memcpy(_dst + i*4*6, _src + i*4*7, sizeof(float) * _boneCnt * 4 * 6);

    const size_t perInstance = sizeof(float)*_boneCnt*4*6;

    for (size_t i = 1u; i < _instCnt; ++i)
        memcpy(_dst + i*perInstance, _dst, sizeof(float)*perInstance);

    return sizeof(float) * perInstance * _instCnt;
}
size_t convertBuf2(const float* _src, float* _dst, const size_t _boneCnt, const size_t _instCnt)
{
    // by mat3x4 (3 rows, 4 columns, column-major layout) i mean glsl's mat4x3...
    float mat3x4[16]; // layout: m00|m10|m20|---|m01|m11|m21|---|m02|m12|m22|---|m03|m13|m23|--- ("---" is unused padding)
    float mat3[12];
    const size_t size_mat3x4_src = 12u;

    const size_t boneSize = (sizeof(mat3x4) + sizeof(mat3)) / sizeof(float);
    for (size_t i = 0u; i < _boneCnt; ++i)
    {
        for (size_t j = 0u; j < 4u; ++j)
            memcpy(mat3x4 + 4*j, _src + 4*7*i + 3*j, 3*sizeof(float));
        for (size_t j = 0u; j < 3u; ++j)
            memcpy(mat3 + 4*j, _src + 4*7*i + size_mat3x4_src + 3*j, 3*sizeof(float));

        memcpy(_dst + boneSize*i, mat3x4, sizeof(mat3x4));
        memcpy(_dst + boneSize*i + sizeof(mat3x4)/sizeof(float), mat3, sizeof(mat3));
    }

    const size_t perInstance = boneSize * _boneCnt;

    for (size_t i = 1u; i < _instCnt; ++i)
        memcpy(_dst + i*perInstance, _dst, sizeof(float)*perInstance);

    return sizeof(float) * perInstance * _instCnt;
}
size_t convertBuf3(const float* _src, float* _dst, const size_t _boneCnt, const size_t _instCnt)
{
    const size_t size_mat3_dst = 12u; // 12 floats
    const size_t size_mat4x3_src = 12u; // 12 floats
    const size_t size_mat4x3_dst = 16u; // 16 floats
    const size_t offset_mat3_dst = _boneCnt * size_mat4x3_dst;
    for (size_t i = 0u; i < _boneCnt; ++i)
    {
        //memcpy(_dst + i*size_mat4x3_dst, _src + i*4*7, size_mat4x3*sizeof(float));
        for (size_t j = 0u; j < 4u; ++j)
            memcpy(_dst + i*size_mat4x3_dst + 4*j, _src + i*4*7 + 3*j, 3*sizeof(float));
        // each mat3 column aligned to 4*sizeof(float)
        for (size_t j = 0u; j < 3u; ++j)
            memcpy(_dst + offset_mat3_dst + i*size_mat3_dst + j*4, _src + i*4*7 + size_mat4x3_src + j*3, 3*sizeof(float));
    }

    const size_t perInstance = (size_mat3_dst + size_mat4x3_dst) * _boneCnt;

    for (size_t i = 1u; i < _instCnt; ++i)
        memcpy(_dst + i*perInstance, _dst, sizeof(float)*perInstance);

    return sizeof(float) * perInstance * _instCnt;
}
size_t convertBuf4(const float* _src, float* _dst, const size_t _boneCnt, const size_t _instCnt)
{
    const size_t size_mat4x3_src = 12u; // 12 floats
    const size_t 
        offset_mat4x3_0 = 0u,
        offset_mat4x3_1 = _boneCnt*4,
        offset_mat4x3_2 = _boneCnt*8,
        offset_mat4x3_3 = _boneCnt*12,
        offset_mat3_0 = _boneCnt*16,
        offset_mat3_1 = _boneCnt*20,
        offset_mat3_2 = _boneCnt*24;
    for (size_t i = 0u; i < _boneCnt; ++i)
    {
        memcpy(_dst + offset_mat4x3_0 + i*4, _src + i*4*7 + 0*3, 3*sizeof(float));
        memcpy(_dst + offset_mat4x3_1 + i*4, _src + i*4*7 + 1*3, 3*sizeof(float));
        memcpy(_dst + offset_mat4x3_2 + i*4, _src + i*4*7 + 2*3, 3*sizeof(float));
        memcpy(_dst + offset_mat4x3_3 + i*4, _src + i*4*7 + 3*3, 3*sizeof(float));
        // each mat3 column aligned to 4*sizeof(float)
        memcpy(_dst + offset_mat3_0 + i*4, _src + i*4*7 + size_mat4x3_src + 0*3, 3*sizeof(float));
        memcpy(_dst + offset_mat3_1 + i*4, _src + i*4*7 + size_mat4x3_src + 1*3, 3*sizeof(float));
        memcpy(_dst + offset_mat3_2 + i*4, _src + i*4*7 + size_mat4x3_src + 2*3, 3*sizeof(float));
    }

    const size_t perInstance = 4 * 7 * _boneCnt;

    for (size_t i = 1u; i < _instCnt; ++i)
        memcpy(_dst + i*perInstance, _dst, sizeof(float)*perInstance);

    return sizeof(float) * perInstance * _instCnt;
}
size_t convertBuf5(const float* _src, float* _dst, const size_t _boneCnt, const size_t _instCnt)
{
    size_t offsets[12+9];
    for (size_t i = 0u; i < sizeof(offsets)/sizeof(*offsets); ++i)
        offsets[i] = i*_boneCnt;

    for (size_t i = 0u; i < _boneCnt; ++i)
    {
        for (size_t o = 0u; o < sizeof(offsets)/sizeof(*offsets); ++o)
        {
            memcpy(_dst + offsets[o] + i, _src + i*4*7 + o, sizeof(float));
        }
    }

    const size_t perInstance = (12 + 9) * _boneCnt;

    for (size_t i = 1u; i < _instCnt; ++i)
        memcpy(_dst + i*perInstance, _dst, sizeof(float)*perInstance);

    return sizeof(float) * perInstance * _instCnt;
}

/*struct UBOManager
{
    UBOManager(size_t _sz, video::IVideoDriver* _drv) :
        drv(_drv),
        updateNum{0u},
        fence{nullptr, nullptr, nullptr, nullptr}
    {
        video::IDriverMemoryBacked::SDriverMemoryRequirements reqs;
        reqs.vulkanReqs.alignment = 4;
        reqs.vulkanReqs.memoryTypeBits = 0xffffffffu;
        reqs.memoryHeapLocation = video::IDriverMemoryAllocation::ESMT_DEVICE_LOCAL;
        reqs.mappingCapability = video::IDriverMemoryAllocation::EMCF_CAN_MAP_FOR_WRITE | video::IDriverMemoryAllocation::EMCF_COHERENT;
        reqs.prefersDedicatedAllocation = true;
        reqs.requiresDedicatedAllocation = true;
        reqs.vulkanReqs.size = 4*_sz;
        mappedBuf = drv->createGPUBufferOnDedMem(reqs);

        reqs.mappingCapability = video::IDriverMemoryAllocation::EMCF_CANNOT_MAP;
        reqs.vulkanReqs.size = _sz;
        ubo = drv->createGPUBufferOnDedMem(reqs);

        mappedMem = (uint8_t*)mappedBuf->getBoundMemory()->mapMemoryRange(video::IDriverMemoryAllocation::EMCAF_WRITE, { updateNum*ubo->getSize(), ubo->getSize() });
    }
    ~UBOManager()
    {
        mappedBuf->getBoundMemory()->unmapMemory();
        mappedBuf->drop();
        ubo->drop();
    }

    void update(size_t _off, size_t _sz, const void* _data)
    {
        if (fence[updateNum])
        {
            auto waitf = [this] {
                auto res = fence[updateNum]->waitCPU(10000000000ull);
                return (res == video::EDFR_CONDITION_SATISFIED || res == video::EDFR_ALREADY_SIGNALED);
            };
			while (!waitf()) {}
			fence[updateNum] = nullptr;
        }

        memcpy(mappedMem + updateNum*ubo->getSize() + _off, _data, _sz);
        video::COpenGLExtensionHandler::extGlFlushMappedNamedBufferRange(dynamic_cast<video::COpenGLBuffer*>(mappedBuf)->getOpenGLName(), updateNum*ubo->getSize() + _off, _sz);

        drv->copyBuffer(mappedBuf, ubo, updateNum*ubo->getSize() + _off, _off, _sz);
		fence[updateNum] = drv->placeFence();

        updateNum = (updateNum + 1) % 4;
    }

    void bind(uint32_t _bnd, ptrdiff_t _off, ptrdiff_t _sz)
    {
        auto auxCtx = const_cast<video::COpenGLDriver::SAuxContext*>(static_cast<video::COpenGLDriver*>(drv)->getThreadContext());
        auto glbuf = static_cast<const video::COpenGLBuffer*>(ubo);
        auxCtx->setActiveUBO(_bnd, 1, &glbuf, &_off, &_sz);
    }

    video::IGPUBuffer* ubo;

private:
    video::IVideoDriver* drv;
    video::IGPUBuffer* mappedBuf;
    uint8_t* mappedMem;
    uint8_t updateNum;
    core::smart_refctd_ptr<video::IDriverFence> fence[4];
};*/

//#define BENCH

#include "irr/irrpack.h"
struct Vertex
{
    uint32_t boneID;
    float pos[3];
    uint8_t color[4];
    uint8_t uv[2];
    float normal[3];
} PACK_STRUCT;
#include "irr/irrunpack.h"

#include "common.glsl"
#include "commonIndirect.glsl"

        //TODO: create shader, and then from check how introspector creates renderpass

int main()
{
    // create device with full flexibility over creation parameters
    // you can add more parameters if desired, check irr::SIrrlichtCreationParameters
    irr::SIrrlichtCreationParameters params;
    params.Bits = 24; //may have to set to 32bit for some platforms
    params.ZBufferBits = 24; //we'd like 32bit here
    params.DriverType = video::EDT_OPENGL; //! Only Well functioning driver, software renderer left for sake of 2D image drawing
    params.WindowSize = dimension2d<uint32_t>(1280, 720);
    params.Fullscreen = false;
    params.Vsync = false;
    params.Doublebuffer = true;
    params.Stencilbuffer = false; //! This will not even be a choice soon
    auto device = createDeviceEx(params);

    if (!device)
        return 1; // could not create selected driver.

    QToQuitEventReceiver receiver;
    device->setEventReceiver(&receiver);

    auto* am = device->getAssetManager();
    video::IVideoDriver* driver = device->getVideoDriver();

    IAssetLoader::SAssetLoadParams lp;

    auto vertexShaderBundle = am->getAsset("../ssboBenchmarkShaders/vertexShader.vert", lp);
    auto fragmentShaderBundle = am->getAsset("../ssboBenchmarkShaders/fragmentShader.frag", lp);

    CShaderIntrospector introspector(am->getGLSLCompiler());
    const auto extensions = driver->getSupportedGLSLExtensions();

    //create cpu shaders
    ICPUSpecializedShader* shaders[2] =
    {
        IAsset::castDown<ICPUSpecializedShader>(vertexShaderBundle.getContents().first->get()),
        IAsset::castDown<ICPUSpecializedShader>(fragmentShaderBundle.getContents().first->get())
    };
    auto cpuPipeline = introspector.createApproximateRenderpassIndependentPipelineFromIntrospection(shaders, shaders + 2, extensions->begin(), extensions->end());

    //temporary
    constexpr size_t diskCount = 3001;

    std::vector<uint16_t> tesselation(diskCount);

    //get random tesselation for disks
    {
        std::random_device rd;
        std::mt19937 mt(rd());
        std::uniform_int_distribution<uint32_t> dist(15, 14999);

        //TODO: test
        //`dist(mt) | 0x0001` so vertexCount is always odd (only when diskCount is odd as well)
        std::generate(tesselation.begin(), tesselation.end(), [&]() { return dist(mt) | 0x0001; });
    }

    //set mesh params
    const size_t vertexCount = std::accumulate<core::vector<uint16_t>::iterator, size_t>(tesselation.begin(), tesselation.end(), 0) + 2 * diskCount; //sum + 2 * diskCount because vertex count of each disk is equal to: (tesselation + 2)
    const size_t indexCount  = std::accumulate<core::vector<uint16_t>::iterator, size_t>(tesselation.begin(), tesselation.end(), 0) * 3;
    E_INDEX_TYPE indexType = E_INDEX_TYPE::EIT_32BIT;
    constexpr uint32_t diskVertexSize = 30;
    constexpr uint32_t vertexSize = sizeof(Vertex);
    SBufferBinding<ICPUBuffer> vertexBuffer{ 0, core::make_smart_refctd_ptr<ICPUBuffer>(vertexCount * vertexSize) };
    // SBufferBinding<ICPUBuffer> bindings[ICPUMeshBuffer::MAX_ATTR_BUF_BINDING_COUNT];
    SBufferBinding<ICPUBuffer> indexBuffer{ 0, core::make_smart_refctd_ptr<ICPUBuffer>(indexCount * sizeof(uint32_t)) };

    SPrimitiveAssemblyParams assemblyParams;
    assemblyParams.primitiveType = E_PRIMITIVE_TOPOLOGY::EPT_TRIANGLE_LIST;

    SVertexInputParams vertexInputParams = 
    { 
        0b1111u, 0b1u,
        {
            {0u,EF_R32_UINT,offsetof(Vertex,boneID)},
            {0u,EF_R32G32B32_SFLOAT,offsetof(Vertex,pos)},
            {0u,EF_R8G8B8A8_UNORM,offsetof(Vertex,color)},
            {0u,EF_R8G8_USCALED,offsetof(Vertex,uv)},
            {0u,EF_R32G32B32_SFLOAT,offsetof(Vertex,normal)}
        },
        {vertexSize,EVIR_PER_VERTEX} 
    };


    core::vector<core::matrix3x4SIMD> SSBOData; //bone matrices will also reside here
    SSBOData.reserve(diskCount);

    //create disks and join them into one mesh buffer
    {
        uint8_t* vtxBuffPtr = static_cast<uint8_t*>(vertexBuffer.buffer.get()->getPointer());
        uint32_t* idxBuffPtr = static_cast<uint32_t*>(indexBuffer.buffer.get()->getPointer());

        size_t objectID = 0;
        for (uint16_t tess : tesselation)
        {
            auto disk = am->getGeometryCreator()->createDiskMesh(1.5f, tess);
            uint8_t* oldVertexPtr = static_cast<uint8_t*>(disk.bindings[0].buffer.get()->getPointer());

            //TODO: test
            //fill vertex buffer
            for (uint16_t i = 0; i < tess; i++)
            {
                *reinterpret_cast<uint32_t*>(vtxBuffPtr) = objectID;
                vtxBuffPtr += sizeof(uint32_t);

                memcpy(vtxBuffPtr, oldVertexPtr, diskVertexSize);
                vtxBuffPtr += diskVertexSize;
            }

            //TODO: test
            //fill index buffer
            for (int i = 0, nextVertex = 1; i < 3 * tess; i += 3)
            {
                idxBuffPtr[0] = 0;
                idxBuffPtr[1] = nextVertex++;
                idxBuffPtr[2] = (nextVertex > tess) ? 1 : nextVertex;

                idxBuffPtr += 3;
            }

            objectID++;
        }

        //set model matrices
        for (int x = 0; x < (diskCount - 1) / 3; x++)
        for (int y = 0; y < (diskCount - 1) / 3; y++)
        for (int z = 0; z < (diskCount - 1) / 3; z++)
            SSBOData.emplace_back(core::matrix3x4SIMD().setTranslation(core::vectorSIMDf(x * 2, y * 2, z * 2)));

        SSBOData.emplace_back(core::matrix3x4SIMD().setTranslation(core::vectorSIMDf(diskCount * 2)));

        //
    }

        //create shader pipeline
    core::smart_refctd_ptr<video::IGPURenderpassIndependentPipeline> gpuPipeline;
    //gpuPipeline = driver->getGPUObjectsFromAssets(&cpuDiskDrawDirectPipeline.get(), &cpuDiskDrawDirectPipeline.get() + 1)->operator[](0);

    auto smgr = device->getSceneManager();

    scene::ICameraSceneNode* camera = smgr->addCameraSceneNodeFPS(0, 100.0f, 0.01f);
    camera->setPosition(core::vector3df(-4, 0, 0));
    camera->setTarget(core::vector3df(0, 0, 0));
    camera->setNearValue(0.01f);
    camera->setFarValue(250.0f);
    smgr->setActiveCamera(camera);

    device->getCursorControl()->setVisible(false);


    uint64_t lastFPSTime = 0;
    float lastFastestMeshFrameNr = -1.f;

        // ----------------------------- MAIN LOOP ----------------------------- 

    while (device->run() && receiver.keepOpen())
        //if (device->isWindowActive())
    {
        driver->beginScene(true, true, video::SColor(255, 255, 255, 255));

        //! This animates (moves) the camera and sets the transforms
        camera->OnAnimate(std::chrono::duration_cast<std::chrono::milliseconds>(device->getTimer()->getTime()).count());
        camera->render();

        core::matrix3x4SIMD normalMatrix;
        camera->getViewMatrix().getSub3x3InverseTranspose(normalMatrix);

        driver->endScene();

        // display frames per second in window title
        uint64_t time = device->getTimer()->getRealTime();
        if (time - lastFPSTime > 1000)
        {
            std::wostringstream str;
            str << L"SSBO Benchmark - Irrlicht Engine [" << driver->getName() << "] FPS:" << driver->getFPS() << " PrimitvesDrawn:" << driver->getPrimitiveCountDrawn();

            device->setWindowCaption(str.str());
            lastFPSTime = time;
        }
    }

    return 0;
}
