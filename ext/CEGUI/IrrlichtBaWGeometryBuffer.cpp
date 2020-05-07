#include "IrrlichtBaWGeometryBuffer.hpp"
#include "IrrlichtBaWRenderer.hpp"

using namespace irr;
using namespace asset;
using namespace video;
using namespace ext;
using namespace cegui;

void IrrlichtBaWGeometryBuffer::draw() const
{

}

void IrrlichtBaWGeometryBuffer::setTranslation(const CEGUI::Vector3f& v)
{

}

void IrrlichtBaWGeometryBuffer::setRotation(const CEGUI::Quaternion& r)
{

}

void IrrlichtBaWGeometryBuffer::setPivot(const CEGUI::Vector3f& p)
{

}

void IrrlichtBaWGeometryBuffer::setClippingRegion(const CEGUI::Rectf& region)
{

}

void IrrlichtBaWGeometryBuffer::appendVertex(const CEGUI::Vertex& vertex)
{

}

void IrrlichtBaWGeometryBuffer::appendGeometry(const CEGUI::Vertex* const vbuff, CEGUI::uint vertex_count)
{

}

void IrrlichtBaWGeometryBuffer::setActiveTexture(CEGUI::Texture* texture)
{

}

void IrrlichtBaWGeometryBuffer::reset()
{

}

CEGUI::Texture* IrrlichtBaWGeometryBuffer::getActiveTexture() const
{

}

CEGUI::uint IrrlichtBaWGeometryBuffer::getVertexCount() const
{

}

CEGUI::uint IrrlichtBaWGeometryBuffer::getBatchCount() const
{

}

void IrrlichtBaWGeometryBuffer::setRenderEffect(CEGUI::RenderEffect* effect)
{

}

CEGUI::RenderEffect* IrrlichtBaWGeometryBuffer::getRenderEffect()
{

}

void IrrlichtBaWGeometryBuffer::setBlendMode(const CEGUI::BlendMode mode)
{

}

CEGUI::BlendMode IrrlichtBaWGeometryBuffer::getBlendMode() const
{

}

void IrrlichtBaWGeometryBuffer::setClippingActive(const bool active)
{

}

bool IrrlichtBaWGeometryBuffer::isClippingActive() const
{

}

void IrrlichtBaWGeometryBuffer::createGpuPipeline()
{
	auto gpuVertexShader = driver->createGPUSpecializedShader
	(
		driver->createGPUShader(std::move(core::make_smart_refctd_ptr<ICPUShader>(IrrlichtBaWRenderer::getDefaultGLSLVertexShader().data()))).get(),
		ISpecializedShader::SInfo({}, nullptr, "main", ISpecializedShader::ESS_VERTEX)
	);

	auto gpuFragmentShader = driver->createGPUSpecializedShader
	(
		driver->createGPUShader(std::move(core::make_smart_refctd_ptr<ICPUShader>(IrrlichtBaWRenderer::getDefaultGLSLFragmentShader().data()))).get(),
		ISpecializedShader::SInfo({}, nullptr, "main", ISpecializedShader::ESS_FRAGMENT)
	);

	IGPUSpecializedShader* shaders[2] = { gpuVertexShader.get(), gpuFragmentShader.get() };

	SVertexInputParams vertexInputParams;	// TODO take from geometry stuff

	SPrimitiveAssemblyParams primitiveAssemblyParams;

	SBlendParams blendParams;
	blendParams.logicOpEnable = false;
	blendParams.logicOp = ELO_NO_OP;
	for (size_t i = 0ull; i < SBlendParams::MAX_COLOR_ATTACHMENT_COUNT; i++)
		blendParams.blendParams[i].attachmentEnabled = (i == 0ull);
	SRasterizationParams rasterParams;
	rasterParams.faceCullingMode = EFCM_NONE;
	rasterParams.depthCompareOp = ECO_ALWAYS;
	rasterParams.minSampleShading = 1.f;
	rasterParams.depthWriteEnable = false;
	rasterParams.depthTestEnable = false;

	auto gpuPipelineLayout = driver->createGPUPipelineLayout(nullptr, nullptr, nullptr, nullptr, nullptr, IrrlichtBaWRenderer::getGpuDesriptorSetLayout3());

	gpuPipeline = driver->createGPURenderpassIndependentPipeline(nullptr, std::move(gpuPipelineLayout), shaders, shaders + sizeof(shaders) / sizeof(IGPUSpecializedShader*), vertexInputParams, blendParams, primitiveAssemblyParams, rasterParams);
}