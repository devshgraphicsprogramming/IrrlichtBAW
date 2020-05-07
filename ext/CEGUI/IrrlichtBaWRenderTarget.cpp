#include "IrrlichtBaWRenderTarget.hpp"
#include "IrrlichtBaWGeometryBuffer.hpp"

using namespace irr;
using namespace ext;
using namespace cegui;

const double IrrlichtBaWRenderTarget::d_yfov_tan = 0.267949192431123;
const CEGUI::String IrrlichtBaWRenderTarget::EventNamespace = "?";		// TODO
const CEGUI::String IrrlichtBaWRenderTarget::EventAreaChanged = "?";	// TODO

IrrlichtBaWRenderTarget::IrrlichtBaWRenderTarget(irr::core::smart_refctd_ptr<irr::IrrlichtDevice> _device)
	: device(_device), /* d_owner(owner) */ d_area(0, 0, 0, 0), d_matrixValid(false), d_viewDistance(0)
{

}

IrrlichtBaWRenderTarget::~IrrlichtBaWRenderTarget()
{

}

void IrrlichtBaWRenderTarget::draw(const CEGUI::GeometryBuffer& buffer)
{
	buffer.draw();
}

void IrrlichtBaWRenderTarget::draw(const CEGUI::RenderQueue& queue)
{
	queue.draw();
}

void IrrlichtBaWRenderTarget::setArea(const CEGUI::Rectf& area)
{
	d_area = area;
	d_matrixValid = false;

	CEGUI::RenderTargetEventArgs args(this);
	fireEvent(CEGUI::RenderTarget::EventAreaChanged, args);
}

const CEGUI::Rectf& IrrlichtBaWRenderTarget::getArea() const
{
	return d_area;
}

bool IrrlichtBaWRenderTarget::isImageryCache() const 
{

}

void IrrlichtBaWRenderTarget::activate() 
{
	driver->setViewPort(irr::core::recti(d_area.left(), d_area.top(), d_area.getWidth(), d_area.getHeight()));

	if (!d_matrixValid)
		updateMatrix();

	//d_owner.setViewProjectionMatrix(d_matrix);			TODO
	//d_owner.setActiveRenderTarget(this);					TODO
}

void IrrlichtBaWRenderTarget::deactivate() 
{

}

void IrrlichtBaWRenderTarget::unprojectPoint(const CEGUI::GeometryBuffer& buff, const CEGUI::Vector2f& p_in, CEGUI::Vector2f& p_out) const 
{

}