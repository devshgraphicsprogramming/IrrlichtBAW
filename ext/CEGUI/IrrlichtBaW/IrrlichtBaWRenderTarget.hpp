#ifndef _IRR_EXT_IRRLICHT_BAW_RENDER_TARGET_INCLUDED_
#define _IRR_EXT_IRRLICHT_BAW_RENDER_TARGET_INCLUDED_

#include <irrlicht.h>
#include <CEGUI/RenderTarget.h>
#include <CEGUI/RenderQueue.h>

namespace irr
{
namespace ext
{
namespace cegui
{

class IrrlichtBaWRenderTarget final : public CEGUI::RenderTarget
{
	public:

		IrrlichtBaWRenderTarget();
		virtual ~IrrlichtBaWRenderTarget();

		virtual void draw(const CEGUI::GeometryBuffer& buffer) override;

		virtual void draw(const CEGUI::RenderQueue& queue) override;

		virtual void setArea(const CEGUI::Rectf& area) override;

		virtual const CEGUI::Rectf& getArea() const override;

		virtual bool isImageryCache() const override;

		virtual void activate() override;

		virtual void deactivate() override;

		virtual void unprojectPoint(const CEGUI::GeometryBuffer& buff, const CEGUI::Vector2f& p_in, CEGUI::Vector2f& p_out) const override;
};

} // namespace cegui
} // namespace ext
} // namespace irr

#endif // _IRR_EXT_IRRLICHT_BAW_RENDER_TARGET_INCLUDED_
