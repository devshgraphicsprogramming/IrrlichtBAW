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

		IrrlichtBaWTexture();
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
};

} // namespace cegui
} // namespace ext
} // namespace irr

#endif // _IRR_EXT_IRRLICHT_BAW_TEXTURE_INCLUDED_
