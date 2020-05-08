#include "IrrlichtBaWTexture.hpp"
#include "irr/asset/IImageAssetHandlerBase.h"

using namespace irr;
using namespace ext;
using namespace cegui;

IrrlichtBaWTexture::IrrlichtBaWTexture(irr::core::smart_refctd_ptr<irr::IrrlichtDevice> _device)
    : device(_device), bufferRowLength(0)
{

}

IrrlichtBaWTexture::~IrrlichtBaWTexture()
{

}

core::smart_refctd_ptr<video::IGPUImageView> createAndFillAttachement(irr::video::IVideoDriver* driver, const void* data, asset::E_FORMAT colorAttachmentFormat, size_t width, size_t height)
{
    asset::ICPUImage::SCreationParams imgInfo;
    imgInfo.format = colorAttachmentFormat;
    imgInfo.type = asset::ICPUImage::ET_2D;
    imgInfo.extent.width = width;
    imgInfo.extent.height = height;
    imgInfo.extent.depth = 1u;
    imgInfo.mipLevels = 1u;
    imgInfo.arrayLayers = 1u;
    imgInfo.samples = asset::ICPUImage::ESCF_1_BIT;
    imgInfo.flags = static_cast<asset::IImage::E_CREATE_FLAGS>(0u);

    auto image = asset::ICPUImage::create(std::move(imgInfo));
    const auto texelFormatBytesize = getTexelOrBlockBytesize(image->getCreationParameters().format);

    core::smart_refctd_ptr<asset::ICPUBuffer> texelBuffer = core::make_smart_refctd_ptr<asset::CCustomAllocatorCPUBuffer<core::null_allocator<uint8_t>>>(image->getImageDataSizeInBytes(), data, core::adopt_memory);
    auto regions = core::make_refctd_dynamic_array<core::smart_refctd_dynamic_array<asset::ICPUImage::SBufferCopy>>(1u);
    asset::ICPUImage::SBufferCopy& region = regions->front();

    region.imageSubresource.mipLevel = 0u;
    region.imageSubresource.baseArrayLayer = 0u;
    region.imageSubresource.layerCount = 1u;
    region.bufferOffset = 0u;
    region.bufferRowLength = image->getCreationParameters().extent.width;
    region.bufferImageHeight = 0u;
    region.imageOffset = { 0u, 0u, 0u };
    region.imageExtent = image->getCreationParameters().extent;

    image->setBufferAndRegions(std::move(texelBuffer), regions);

    asset::ICPUImageView::SCreationParams imgViewInfo;
    imgViewInfo.image = std::move(image);
    imgViewInfo.format = colorAttachmentFormat;
    imgViewInfo.viewType = asset::IImageView<asset::ICPUImage>::ET_2D;
    imgViewInfo.flags = static_cast<asset::ICPUImageView::E_CREATE_FLAGS>(0u);
    imgViewInfo.subresourceRange.baseArrayLayer = 0u;
    imgViewInfo.subresourceRange.baseMipLevel = 0u;
    imgViewInfo.subresourceRange.layerCount = imgInfo.arrayLayers;
    imgViewInfo.subresourceRange.levelCount = imgInfo.mipLevels;

    auto imageView = asset::ICPUImageView::create(std::move(imgViewInfo));
    auto gpuImageView = driver->getGPUObjectsFromAssets(&imageView.get(), &imageView.get() + 1)->front();

    return std::move(gpuImageView);
}

irr::video::IFrameBuffer* IrrlichtBaWTexture::createFrameBuffer(const void* data, asset::E_FORMAT colorAttachmentFormat, size_t width, size_t height)
{
    auto gpuImageViewColorBuffer = createAndFillAttachement(driver, data, colorAttachmentFormat, width, height);

    auto frameBuffer = driver->addFrameBuffer();
    frameBuffer->attach(textureColorAttachment, std::move(gpuImageViewColorBuffer));

    return frameBuffer;
}

irr::video::IFrameBuffer* IrrlichtBaWTexture::createFrameBuffer(core::smart_refctd_ptr<video::IGPUImageView> gpuImageView)
{
    auto frameBuffer = driver->addFrameBuffer();
    frameBuffer->attach(textureColorAttachment, std::move(gpuImageView));

    return frameBuffer;
}

const CEGUI::String& IrrlichtBaWTexture::getName() const
{
    return cachingName;
}

const CEGUI::Sizef& IrrlichtBaWTexture::getSize() const
{
    bool status = frameBuffer;

    if (status)
    {
        auto& params = frameBuffer->getAttachment(textureColorAttachment)->getCreationParameters().image->getCreationParameters();
        return { params.extent.width, params.extent.height };
    }
    else
    {
        assert(status);
        return { 0, 0 };
    }  
}

const CEGUI::Sizef& IrrlichtBaWTexture::getOriginalDataSize() const
{
    bool status = frameBuffer;

    if (status)
    {
        auto& params = frameBuffer->getAttachment(textureColorAttachment)->getCreationParameters().image->getCreationParameters();
        return { bufferRowLength ? bufferRowLength : params.extent.width, params.extent.height };
    }
    else 
    {
        assert(status);
        return { 0, 0 };
    }
}

const CEGUI::Vector2f& IrrlichtBaWTexture::getTexelScaling() const
{
    bool status = frameBuffer;

    if (status)
    {
        auto& params = frameBuffer->getAttachment(textureColorAttachment)->getCreationParameters().image->getCreationParameters();
        auto width = bufferRowLength ? bufferRowLength : params.extent.width;

        return
        {
            static_cast<float>(width) == 0.f ? 0.f : 1.f / static_cast<float>(width),
            static_cast<float>(params.extent.height) == 0.f ? 0.f : 1.f / static_cast<float>(params.extent.height)
        };
    }
    else
    {
        assert(status);
        return { 0, 0 };
    }
}

void IrrlichtBaWTexture::loadFromFile(const CEGUI::String& filename, const CEGUI::String& resourceGroup)
{
    if (frameBuffer)
        frameBuffer->drop();

    auto path = resourceGroup + filename;
    auto assetManager = device->getAssetManager();

    auto params = asset::IAssetLoader::SAssetLoadParams();
    auto bundle = assetManager->getAsset(path, params);
    auto cpuImage = asset::IAsset::castDown<asset::ICPUImage>(bundle.getContents().first[0]);
    assert(cpuImage.get(), "something went wrong while casting, empty pointer detected!");

    bufferRowLength = cpuImage->getRegions().begin()->bufferRowLength;

    asset::ICPUImageView::SCreationParams imageViewInfo;
    imageViewInfo.image = std::move(cpuImage);
    imageViewInfo.format = imageViewInfo.image->getCreationParameters().format;
    imageViewInfo.viewType = asset::IImageView<asset::ICPUImage>::ET_2D;
    imageViewInfo.flags = static_cast<asset::ICPUImageView::E_CREATE_FLAGS>(0u);
    imageViewInfo.subresourceRange.baseArrayLayer = 0u;
    imageViewInfo.subresourceRange.baseMipLevel = 0u;
    imageViewInfo.subresourceRange.layerCount = imageViewInfo.image->getCreationParameters().arrayLayers;
    imageViewInfo.subresourceRange.levelCount = imageViewInfo.image->getCreationParameters().mipLevels;

    auto cpuImageView = asset::ICPUImageView::create(std::move(imageViewInfo));

    // cachingName = ""     TODO Okay, so we have to get likely GL id or something similar. Should I cast it to OpenGL image or maybe we should get CPU cache string?

    auto newTexture = device->getVideoDriver()->getGPUObjectsFromAssets(&cpuImageView.get(), &cpuImageView.get() + 1u)->front();
    frameBuffer = createFrameBuffer(newTexture);
}

void IrrlichtBaWTexture::loadFromMemory(const void* buffer, const CEGUI::Sizef& buffer_size, PixelFormat pixel_format)
{
    if (frameBuffer)
        frameBuffer->drop();

    asset::E_FORMAT newFormat = [&]()
    {
        switch (pixel_format)
        {
            case PixelFormat::PF_RGB: return asset::EF_R8G8B8_SRGB;
            case PixelFormat::PF_RGBA: return asset::EF_R8G8B8A8_SRGB;
            case PixelFormat::PF_RGBA_4444: return asset::EF_R4G4B4A4_UNORM_PACK16;
            case PixelFormat::PF_RGB_565: return asset::EF_R5G6B5_UNORM_PACK16;
            case PixelFormat::PF_PVRTC2: return asset::EF_PVRTC2_2BPP_SRGB_BLOCK_IMG;
            case PixelFormat::PF_PVRTC4: return asset::EF_PVRTC2_4BPP_SRGB_BLOCK_IMG;
            case PixelFormat::PF_RGB_DXT1: return asset::EF_BC1_RGB_SRGB_BLOCK;
            case PixelFormat::PF_RGBA_DXT1: return asset::EF_BC1_RGBA_SRGB_BLOCK;
            case PixelFormat::PF_RGBA_DXT3: return asset::EF_BC2_SRGB_BLOCK;
            case PixelFormat::PF_RGBA_DXT5: return asset::EF_BC3_SRGB_BLOCK;
                
            default:
                return asset::EF_UNKNOWN;
        }
    }();

    auto texelOrBlockByteSize = asset::getTexelOrBlockBytesize(newFormat);
    auto bufferByteSize = buffer_size.d_width * buffer_size.d_height * texelOrBlockByteSize;
    core::smart_refctd_ptr<asset::ICPUBuffer> cpuPixelBuffer = core::make_smart_refctd_ptr<asset::CCustomAllocatorCPUBuffer<core::null_allocator<uint8_t>>>(bufferByteSize, buffer, core::adopt_memory);
    core::smart_refctd_ptr<video::IGPUBuffer> gpuPixelBuffer = driver->getGPUObjectsFromAssets(&cpuPixelBuffer.get(), &cpuPixelBuffer.get() + 1u)->front();

    asset::IImage::SCreationParams imageParams;
    imageParams.flags = static_cast<asset::IImage::E_CREATE_FLAGS>(0);
    imageParams.type = asset::IImage::ET_2D;
    imageParams.format = newFormat;
    imageParams.extent = { buffer_size.d_width, buffer_size.d_height, 1 };
    imageParams.mipLevels = 1;
    imageParams.arrayLayers = 1;

    assert((asset::IImageAssetHandlerBase::calcPitchInBlocks(imageParams.extent.width, texelOrBlockByteSize) == imageParams.extent.width), "Width of memory data passed doesn't pass requirements - it isn't tightly packed!");

    auto regions = core::make_refctd_dynamic_array<core::smart_refctd_dynamic_array<asset::ICPUImage::SBufferCopy>>(1u);
    asset::ICPUImage::SBufferCopy& region = regions->front();
    region.imageSubresource.mipLevel = 0u;
    region.imageSubresource.baseArrayLayer = 0u;
    region.imageSubresource.layerCount = 1u;
    region.bufferOffset = 0u;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0u; 
    region.imageOffset = { 0u, 0u, 0u };
    region.imageExtent = imageParams.extent;

    auto gpuImage = device->getVideoDriver()->createGPUImage(std::move(imageParams));
    driver->copyBufferToImage(gpuPixelBuffer.get(), gpuImage.get(), regions->size(), regions->begin());

    asset::IImageView<video::IGPUImage>::SCreationParams viewParams;
    viewParams.flags = static_cast<decltype(viewParams.flags)>(0);
    viewParams.format = newFormat;
    viewParams.image = gpuImage;
    viewParams.subresourceRange.baseArrayLayer = 0;
    viewParams.subresourceRange.baseMipLevel = 0;
    viewParams.subresourceRange.layerCount = 1;
    viewParams.subresourceRange.levelCount = 1;
    viewParams.viewType = decltype(viewParams.viewType)::ET_2D;

    frameBuffer = createFrameBuffer(driver->createGPUImageView(std::move(viewParams)));
}

void IrrlichtBaWTexture::blitFromMemory(const void* sourceData, const CEGUI::Rectf& area)
{
    bool status = frameBuffer;

    if (status)
    {
        auto gpuTexture = frameBuffer->getAttachment(textureColorAttachment);
        auto& viewParams = gpuTexture->getCreationParameters();
        auto& extent = viewParams.image->getCreationParameters().extent;
        irr::core::recti coverArea(area.left(), area.top(), area.getWidth(), area.getHeight());

        auto temporaryInFrameBuffer = createFrameBuffer(sourceData, viewParams.format, extent.width, extent.height);
        driver->blitRenderTargets(temporaryInFrameBuffer, frameBuffer, false, false, coverArea, coverArea);
        temporaryInFrameBuffer->drop();
    }
    else
        assert(status);
}

void IrrlichtBaWTexture::blitToMemory(void* targetData)
{
    bool status = frameBuffer;

    if (status)
    {
        auto gpuTexture = frameBuffer->getAttachment(textureColorAttachment);
        auto& viewParams = gpuTexture->getCreationParameters();
        auto& extent = viewParams.image->getCreationParameters().extent;

        auto temporaryOutFrameBuffer = createFrameBuffer(targetData, viewParams.format, extent.width, extent.height);
        driver->blitRenderTargets(frameBuffer, temporaryOutFrameBuffer, false, false);
 
        auto bufferByteSize = viewParams.image->getImageDataSizeInBytes();
        auto destinationBoundMemory = const_cast<video::IDriverMemoryAllocation*>(viewParams.image->getBoundMemory()); // not sure
        destinationBoundMemory->mapMemoryRange(video::IDriverMemoryAllocation::EMCAF_READ, { 0u, bufferByteSize });
        core::smart_refctd_ptr<asset::ICPUBuffer> pixelBuffer = core::make_smart_refctd_ptr<asset::CCustomAllocatorCPUBuffer<core::null_allocator<uint8_t>>>(bufferByteSize, destinationBoundMemory->getMappedPointer(), core::adopt_memory);

        memcpy(targetData, pixelBuffer->getPointer(), pixelBuffer->getSize());

        temporaryOutFrameBuffer->drop();
    }
    else
        assert(status);
}

bool IrrlichtBaWTexture::isPixelFormatSupported(const PixelFormat fmt) const
{
    return true;
}