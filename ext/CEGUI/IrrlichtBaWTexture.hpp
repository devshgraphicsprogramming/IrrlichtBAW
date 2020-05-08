#ifndef _IRR_EXT_IRRLICHT_BAW_TEXTURE_INCLUDED_
#define _IRR_EXT_IRRLICHT_BAW_TEXTURE_INCLUDED_

#include <irrlicht.h>
#include <CEGUI/Texture.h>

namespace irr
{
namespace ext
{
namespace cegui
{

class IrrlichtBaWTexture final : public CEGUI::Texture
{
	public:

		IrrlichtBaWTexture(irr::core::smart_refctd_ptr<irr::IrrlichtDevice> _device);
		virtual ~IrrlichtBaWTexture();

        virtual const CEGUI::String& getName() const override;

        virtual const CEGUI::Sizef& getSize() const override;

        virtual const CEGUI::Sizef& getOriginalDataSize() const override;

        virtual const CEGUI::Vector2f& getTexelScaling() const override;

        virtual void loadFromFile(const CEGUI::String& filename, const CEGUI::String& resourceGroup) override;

        virtual void loadFromMemory(const void* buffer, const CEGUI::Sizef& buffer_size, PixelFormat pixel_format) override;

        virtual void blitFromMemory(const void* sourceData, const CEGUI::Rectf& area) override;

        virtual void blitToMemory(void* targetData) override;

        virtual bool isPixelFormatSupported(const PixelFormat fmt) const override;

        static constexpr const auto textureColorAttachment = video::EFAP_COLOR_ATTACHMENT0;

    private:

        irr::video::IFrameBuffer* createFrameBuffer(const void* data, asset::E_FORMAT colorAttachmentFormat, size_t width, size_t height);
        irr::video::IFrameBuffer* createFrameBuffer(core::smart_refctd_ptr<video::IGPUImageView> gpuImageView);

        irr::core::smart_refctd_ptr<irr::IrrlichtDevice> device;
        irr::video::IVideoDriver* driver;
        // OpenGLRendererBase& d_owner; TODO
        irr::video::IFrameBuffer* frameBuffer;
        std::string cachingName;
        size_t bufferRowLength;
};

} // namespace cegui
} // namespace ext
} // namespace irr

#endif // _IRR_EXT_IRRLICHT_BAW_TEXTURE_INCLUDED_
