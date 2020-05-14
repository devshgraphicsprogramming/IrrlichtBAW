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

        const CEGUI::String& getName() const override;

        const CEGUI::Sizef& getSize() const override;

        const CEGUI::Sizef& getOriginalDataSize() const override;

        const CEGUI::Vector2f& getTexelScaling() const override;

        void loadFromFile(const CEGUI::String& filename, const CEGUI::String& resourceGroup) override;

        void loadFromMemory(const void* buffer, const CEGUI::Sizef& buffer_size, PixelFormat pixel_format) override;

        void blitFromMemory(const void* sourceData, const CEGUI::Rectf& area) override;

        void blitToMemory(void* targetData) override;

        bool isPixelFormatSupported(const PixelFormat fmt) const override;

    private:

        static inline asset::E_FORMAT getTranslatedFormat(const PixelFormat ceguiFormat);

        core::smart_refctd_ptr<video::IGPUImageView> gpuImageViewTexture;

        irr::core::smart_refctd_ptr<irr::IrrlichtDevice> device;
        irr::video::IVideoDriver* driver;
        // OpenGLRendererBase& d_owner; TODO
        std::string cachingName;
        size_t bufferRowLength;
};

} // namespace cegui
} // namespace ext
} // namespace irr

#endif // _IRR_EXT_IRRLICHT_BAW_TEXTURE_INCLUDED_
