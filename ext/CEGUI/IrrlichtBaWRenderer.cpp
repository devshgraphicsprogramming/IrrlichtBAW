#include "IrrlichtBaWRenderer.hpp"

using namespace irr;
using namespace asset;
using namespace video;
using namespace ext;
using namespace cegui;

CEGUI::String IrrlichtBaWRenderer::d_rendererID = "IrrlichtBaW Renderer";

IrrlichtBaWRenderer::IrrlichtBaWRenderer(irr::core::smart_refctd_ptr<irr::IrrlichtDevice> _device)
	: device(_device), driver(device->getVideoDriver())
{
	initializeDescriptorSetLayout();
}

IrrlichtBaWRenderer::~IrrlichtBaWRenderer()
{

}

CEGUI::RenderTarget& IrrlichtBaWRenderer::getDefaultRenderTarget()
{

}

CEGUI::GeometryBuffer& IrrlichtBaWRenderer::createGeometryBuffer()
{

}

void IrrlichtBaWRenderer::destroyGeometryBuffer(const CEGUI::GeometryBuffer& buffer)
{

}

void IrrlichtBaWRenderer::destroyAllGeometryBuffers()
{

}

CEGUI::TextureTarget* IrrlichtBaWRenderer::createTextureTarget()
{

}

void IrrlichtBaWRenderer::destroyTextureTarget(CEGUI::TextureTarget* target)
{

}

void IrrlichtBaWRenderer::destroyAllTextureTargets()
{

}

CEGUI::Texture& IrrlichtBaWRenderer::createTexture(const CEGUI::String& name)
{

}

CEGUI::Texture& IrrlichtBaWRenderer::createTexture(const CEGUI::String& name, const CEGUI::String& filename, const CEGUI::String& resourceGroup)
{

}

CEGUI::Texture& IrrlichtBaWRenderer::createTexture(const CEGUI::String& name, const CEGUI::Sizef& size)
{

}

void IrrlichtBaWRenderer::destroyTexture(CEGUI::Texture& texture)
{

}

void IrrlichtBaWRenderer::destroyAllTextures()
{

}

CEGUI::Texture& IrrlichtBaWRenderer::getTexture(const CEGUI::String& name) const
{

}

bool IrrlichtBaWRenderer::isTextureDefined(const CEGUI::String& name) const
{

}

void IrrlichtBaWRenderer::beginRendering()
{

}

void IrrlichtBaWRenderer::endRendering()
{

}

void IrrlichtBaWRenderer::setDisplaySize(const CEGUI::Sizef& size)
{

}

const CEGUI::Sizef& IrrlichtBaWRenderer::getDisplaySize() const
{

}

const CEGUI::Vector2f& IrrlichtBaWRenderer::getDisplayDPI() const
{

}

CEGUI::uint IrrlichtBaWRenderer::getMaxTextureSize() const
{

}

const CEGUI::String& IrrlichtBaWRenderer::getIdentifierString() const
{
	return d_rendererID;
}

void IrrlichtBaWRenderer::initializeDescriptorSetLayout()
{
	IGPUDescriptorSetLayout::SBinding binding{ 0u, EDT_COMBINED_IMAGE_SAMPLER, 1u, IGPUSpecializedShader::ESS_FRAGMENT, nullptr };
	gpuDescriptorSetLayout = driver->createGPUDescriptorSetLayout(&binding, &binding + 1u);
}