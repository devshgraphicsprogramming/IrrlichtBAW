#include "IrrlichtBaWTexture.hpp"

using namespace irr;
using namespace ext;
using namespace cegui;

IrrlichtBaWTexture::IrrlichtBaWTexture(irr::core::smart_refctd_ptr<irr::IrrlichtDevice> _device)
    : device(_device)
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
    bool status = gpuTexture.get();

    if (status)
    {
        auto& params = gpuTexture->getCreationParameters().image->getCreationParameters();
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
    bool status = gpuTexture.get();

    if (status)
    {
        auto& params = gpuTexture->getCreationParameters().image->getCreationParameters();
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
    bool status = gpuTexture.get();

    if (status)
    {
        auto& params = gpuTexture->getCreationParameters().image->getCreationParameters();
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

    gpuTexture = device->getVideoDriver()->getGPUObjectsFromAssets(&cpuImageView.get(), &cpuImageView.get() + 1u)->front();
    
}

void IrrlichtBaWTexture::loadFromMemory(const void* buffer, const CEGUI::Sizef& buffer_size, PixelFormat pixel_format)
{
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

    asset::IImage::SCreationParams imageParams;
    imageParams.flags = static_cast<asset::IImage::E_CREATE_FLAGS>(0);
    imageParams.type = asset::IImage::ET_2D;
    imageParams.format = newFormat;
    imageParams.extent = { buffer_size.d_width, buffer_size.d_height, 1 };
    imageParams.mipLevels = 1;
    imageParams.arrayLayers = 1;

    auto gpuImage = device->getVideoDriver()->createGPUImage(std::move(imageParams));
    // TODO copy buffer to image

    asset::IImageView<video::IGPUImage>::SCreationParams viewParams;
    viewParams.flags = static_cast<decltype(viewParams.flags)>(0);
    viewParams.format = newFormat;
    viewParams.image = gpuImage;
    viewParams.subresourceRange.baseArrayLayer = 0;
    viewParams.subresourceRange.baseMipLevel = 0;
    viewParams.subresourceRange.layerCount = 1;
    viewParams.subresourceRange.levelCount = 1;
    viewParams.viewType = decltype(viewParams.viewType)::ET_2D;

    gpuTexture = driver->createGPUImageView(std::move(viewParams));
}

void IrrlichtBaWTexture::blitFromMemory(const void* sourceData, const CEGUI::Rectf& area)
{

}

void IrrlichtBaWTexture::blitToMemory(void* targetData)
{

}

bool IrrlichtBaWTexture::isPixelFormatSupported(const PixelFormat fmt) const
{
    return true;
}