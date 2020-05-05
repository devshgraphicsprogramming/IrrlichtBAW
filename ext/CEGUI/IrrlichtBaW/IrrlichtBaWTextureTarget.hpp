#ifndef _IRR_EXT_IRRLICHT_BAW_TEXTURE_TARGET_INCLUDED_
#define _IRR_EXT_IRRLICHT_BAW_TEXTURE_TARGET_INCLUDED_

#include <irrlicht.h>
#include <CEGUI/TextureTarget.h>

namespace irr
{
namespace ext
{
namespace cegui
{

class IrrlichtBaWTextureTarget final : public CEGUI::TextureTarget
{
	public:

		IrrlichtBaWTextureTarget();
		virtual ~IrrlichtBaWTextureTarget();

        virtual void clear() override;

        virtual CEGUI::Texture& getTexture() const override;

        virtual void declareRenderSize(const CEGUI::Sizef& sz) override;

        virtual bool isRenderingInverted() const override;
};

} // namespace cegui
} // namespace ext
} // namespace irr

#endif // _IRR_EXT_IRRLICHT_BAW_TEXTURE_TARGET_INCLUDED_
