#include "IrrlichtBaWTexture.hpp"
#include "irr/asset/IImageAssetHandlerBase.h"

using namespace irr;
using namespace ext;
using namespace cegui;

static constexpr std::string_view _IRR_TEXTURE_MUST_EXIST_ = "To perform the function, gpu texture must exist at that point!";
static constexpr std::string_view _IRR_CEGUI_UNSUPPORTED_FORMAT_ = "Unsupported pixel format detected!";
static constexpr std::string_view _IRR_WRONG_CASTING_ = "Something went wrong while casting, empty pointer detected!";
static constexpr std::string_view _IRR_NOT_TIGHTLY_PACKED_ = "Width of memory data passed doesn't pass requirements - it isn't tightly packed!";

video::IDriverMemoryBacked::SDriverMemoryRequirements getMemoryRequirements(size_t vulkanReqsSize)
{
    video::IDriverMemoryBacked::SDriverMemoryRequirements requirements;
    requirements.vulkanReqs.size = vulkanReqsSize;
    requirements.vulkanReqs.alignment = 4;
    requirements.vulkanReqs.memoryTypeBits = 0xffffffffu;
    requirements.memoryHeapLocation = video::IDriverMemoryAllocation::ESMT_DEVICE_LOCAL;
    requirements.mappingCapability = video::IDriverMemoryAllocation::EMCAF_READ_AND_WRITE;
    requirements.prefersDedicatedAllocation = true;
    requirements.requiresDedicatedAllocation = true;

    return requirements;
}


IrrlichtBaWTexture::IrrlichtBaWTexture(irr::core::smart_refctd_ptr<irr::IrrlichtDevice> _device)
    : device(_device), bufferRowLength(0)
{

}

IrrlichtBaWTexture::~IrrlichtBaWTexture()
{

}

const CEGUI::String& IrrlichtBaWTexture::getName() const
{
    return cachingName;
}

const CEGUI::Sizef& IrrlichtBaWTexture::getSize() const
{
    bool status = gpuImageViewTexture.get();

    if (status)
    {
        auto& params = gpuImageViewTexture->getCreationParameters().image->getCreationParameters();
        return { params.extent.width, params.extent.height };
    }
    else
    {
        assert(status, _IRR_TEXTURE_MUST_EXIST_.data());
        return { 0, 0 };
    }  
}

const CEGUI::Sizef& IrrlichtBaWTexture::getOriginalDataSize() const
{
    bool status = gpuImageViewTexture.get();

    if (status)
    {
        auto& params = gpuImageViewTexture->getCreationParameters().image->getCreationParameters();
        return { bufferRowLength ? bufferRowLength : params.extent.width, params.extent.height };
    }
    else 
    {
        assert(status, _IRR_TEXTURE_MUST_EXIST_.data());
        return { 0, 0 };
    }
}

const CEGUI::Vector2f& IrrlichtBaWTexture::getTexelScaling() const
{
    bool status = gpuImageViewTexture.get();

    if (status)
    {
        auto& params = gpuImageViewTexture->getCreationParameters().image->getCreationParameters();
        auto width = bufferRowLength ? bufferRowLength : params.extent.width;

        return
        {
            static_cast<float>(width) == 0.f ? 0.f : 1.f / static_cast<float>(width),
            static_cast<float>(params.extent.height) == 0.f ? 0.f : 1.f / static_cast<float>(params.extent.height)
        };
    }
    else
    {
        assert(status, _IRR_TEXTURE_MUST_EXIST_.data());
        return { 0, 0 };
    }
}

void IrrlichtBaWTexture::loadFromFile(const CEGUI::String& filename, const CEGUI::String& resourceGroup)
{
    auto path = resourceGroup + filename;
    auto assetManager = device->getAssetManager();

    auto params = asset::IAssetLoader::SAssetLoadParams();
    auto bundle = assetManager->getAsset(path, params);  /// TODO - we need to have separate caching for CEGUI
    auto cpuImage = asset::IAsset::castDown<asset::ICPUImage>(bundle.getContents().first[0]);
    assert(cpuImage.get(), _IRR_WRONG_CASTING_.data());

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

    cachingName = "";   /// TODO - having seperate caching for CEGUI, we can assign the variables here

    gpuImageViewTexture = device->getVideoDriver()->getGPUObjectsFromAssets(&cpuImageView.get(), &cpuImageView.get() + 1u)->front();
}

void IrrlichtBaWTexture::loadFromMemory(const void* buffer, const CEGUI::Sizef& buffer_size, PixelFormat pixel_format)
{
    asset::E_FORMAT newFormat = getTranslatedFormat(pixel_format);
    auto texelOrBlockByteSize = asset::getTexelOrBlockBytesize(newFormat);

    auto requirements = getMemoryRequirements(buffer_size.d_width * buffer_size.d_height * texelOrBlockByteSize);

    auto gpuBuffer = driver->createGPUBufferOnDedMem(requirements);
    driver->updateBufferRangeViaStagingBuffer(gpuBuffer.get(), 0, requirements.vulkanReqs.size, buffer);

    asset::IImage::SCreationParams imageParams;
    imageParams.flags = static_cast<asset::IImage::E_CREATE_FLAGS>(0);
    imageParams.type = asset::IImage::ET_2D;
    imageParams.format = newFormat;
    imageParams.extent = { buffer_size.d_width, buffer_size.d_height, 1 };
    imageParams.mipLevels = 1;
    imageParams.arrayLayers = 1;

    assert((asset::IImageAssetHandlerBase::calcPitchInBlocks(imageParams.extent.width, texelOrBlockByteSize) == imageParams.extent.width), _IRR_NOT_TIGHTLY_PACKED_.data());
    auto gpuImage = driver->createGPUImage(std::move(imageParams));

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

    driver->copyBufferToImage(gpuBuffer.get(), gpuImage.get(), regions->size(), regions->begin());

    asset::IImageView<video::IGPUImage>::SCreationParams viewParams;
    viewParams.flags = static_cast<decltype(viewParams.flags)>(0);
    viewParams.format = newFormat;
    viewParams.image = gpuImage;
    viewParams.subresourceRange.baseArrayLayer = 0;
    viewParams.subresourceRange.baseMipLevel = 0;
    viewParams.subresourceRange.layerCount = imageParams.arrayLayers;
    viewParams.subresourceRange.levelCount = imageParams.mipLevels;
    viewParams.viewType = decltype(viewParams.viewType)::ET_2D;

    gpuImageViewTexture = driver->createGPUImageView(std::move(viewParams));
}

void IrrlichtBaWTexture::blitFromMemory(const void* sourceData, const CEGUI::Rectf& area)
{
    bool status = gpuImageViewTexture.get();

    if (status)
    {
        auto& viewParams = gpuImageViewTexture->getCreationParameters();
        auto& extent = viewParams.image->getCreationParameters().extent;

        auto requirements = getMemoryRequirements(area.getWidth() * area.getHeight() * asset::getTexelOrBlockBytesize(viewParams.format));

        auto gpuBuffer = driver->createGPUBufferOnDedMem(requirements);
        driver->updateBufferRangeViaStagingBuffer(gpuBuffer.get(), 0, requirements.vulkanReqs.size, sourceData);

        auto regions = core::make_refctd_dynamic_array<core::smart_refctd_dynamic_array<asset::ICPUImage::SBufferCopy>>(1u);
        asset::ICPUImage::SBufferCopy& region = regions->front();
        region.imageSubresource.mipLevel = 0u;
        region.imageSubresource.baseArrayLayer = 0u;
        region.imageSubresource.layerCount = 1u;
        region.bufferOffset = 0u;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0u;
        region.imageOffset = { area.left(), area.top(), 0u };
        region.imageExtent = extent;

        driver->copyBufferToImage(gpuBuffer.get(), gpuImageViewTexture->getCreationParameters().image.get(), regions->size(), regions->begin());
    }
    else
        assert(status, _IRR_TEXTURE_MUST_EXIST_.data());
}

void IrrlichtBaWTexture::blitToMemory(void* targetData)
{
    bool status = gpuImageViewTexture.get();

    if (status)
    {
        auto& viewParams = gpuImageViewTexture->getCreationParameters();
        auto& extent = viewParams.image->getCreationParameters().extent;
        const auto bufferByteSize = asset::getTexelOrBlockBytesize(viewParams.format) * extent.width * extent.height;

        video::IDriverMemoryBacked::SDriverMemoryRequirements requirements = getMemoryRequirements(bufferByteSize);
        auto gpuBuffer = driver->createGPUBufferOnDedMem(requirements);

        auto regions = core::make_refctd_dynamic_array<core::smart_refctd_dynamic_array<asset::ICPUImage::SBufferCopy>>(1u);
        asset::ICPUImage::SBufferCopy& region = regions->front();
        region.imageSubresource.mipLevel = 0u;
        region.imageSubresource.baseArrayLayer = 0u;
        region.imageSubresource.layerCount = 1u;
        region.bufferOffset = 0u;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0u;
        region.imageOffset = { 0, 0, 0u };
        region.imageExtent = extent;

        driver->copyImageToBuffer(viewParams.image.get(), gpuBuffer.get(), regions->size(), regions->begin());
        auto clientBoundMemory = gpuBuffer->getBoundMemory();
        clientBoundMemory->mapMemoryRange(video::IDriverMemoryAllocation::EMCAF_READ, { 0u, bufferByteSize });
        memcpy(targetData, clientBoundMemory->getMappedPointer(), bufferByteSize);
        clientBoundMemory->unmapMemory();
    }
    else
        assert(status, _IRR_TEXTURE_MUST_EXIST_.data());
}

bool IrrlichtBaWTexture::isPixelFormatSupported(const PixelFormat fmt) const
{
    return getTranslatedFormat(fmt);
}

asset::E_FORMAT IrrlichtBaWTexture::getTranslatedFormat(const PixelFormat ceguiFormat)
{
    asset::E_FORMAT newFormat = [&]()
    {
        switch (ceguiFormat)
        {
            case PixelFormat::PF_RGB: return asset::EF_R8G8B8_SRGB;
            case PixelFormat::PF_RGBA: return asset::EF_R8G8B8A8_SRGB;
            case PixelFormat::PF_RGBA_4444: return asset::EF_R4G4B4A4_UNORM_PACK16;
            case PixelFormat::PF_RGB_565: _IRR_FALLTHROUGH;
            case PixelFormat::PF_PVRTC2: _IRR_FALLTHROUGH;
            case PixelFormat::PF_PVRTC4: return asset::EF_UNKNOWN;
            case PixelFormat::PF_RGB_DXT1: return asset::EF_BC1_RGB_SRGB_BLOCK;
            case PixelFormat::PF_RGBA_DXT1: return asset::EF_BC1_RGBA_SRGB_BLOCK;
            case PixelFormat::PF_RGBA_DXT3: return asset::EF_BC2_SRGB_BLOCK;
            case PixelFormat::PF_RGBA_DXT5: return asset::EF_BC3_SRGB_BLOCK;

        default:
            return asset::EF_UNKNOWN;
        }
    }();

    assert(newFormat != asset::EF_UNKNOWN, _IRR_CEGUI_UNSUPPORTED_FORMAT_.data());
    return newFormat;
}