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

		IrrlichtBaWRenderTarget(irr::core::smart_refctd_ptr<irr::IrrlichtDevice> _device);
		virtual ~IrrlichtBaWRenderTarget();

		virtual void draw(const CEGUI::GeometryBuffer& buffer) override;

		virtual void draw(const CEGUI::RenderQueue& queue) override;

		virtual void setArea(const CEGUI::Rectf& area) override;

		virtual const CEGUI::Rectf& getArea() const override;

		virtual bool isImageryCache() const override;

		virtual void activate() override;

		virtual void deactivate() override;

		virtual void unprojectPoint(const CEGUI::GeometryBuffer& buff, const CEGUI::Vector2f& p_in, CEGUI::Vector2f& p_out) const override;

	private:

		irr::core::smart_refctd_ptr<irr::IrrlichtDevice> device;
		irr::video::IVideoDriver* driver;

		//! helper that initialises the cached matrix
		void updateMatrix() const;

		//! OpenGLRendererBase that created this object
		//OpenGLRendererBase& d_owner; ?? TODO

		//! holds defined area for the RenderTarget
		CEGUI::Rectf d_area;
		//! tangent of the y FOV half-angle; used to calculate viewing distance.
		static const double d_yfov_tan;
		//! saved copy of projection matrix
		mutable core::matrix4SIMD d_matrix;
		//! true if saved matrix is up to date
		mutable bool d_matrixValid;
		//! tracks viewing distance (this is set up at the same time as d_matrix)
		mutable double d_viewDistance;
};

} // namespace cegui
} // namespace ext
} // namespace irr

#endif // _IRR_EXT_IRRLICHT_BAW_RENDER_TARGET_INCLUDED_
