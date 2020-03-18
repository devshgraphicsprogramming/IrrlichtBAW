/***********************************************************************
    created:    Thu Dec 27 2018
    author:     Manh Nguyen Tien
*************************************************************************/
/***************************************************************************
 *   Copyright (C) 2004 - 2011 Paul D Turner & The CEGUI Development Team
 *
 *   Permission is hereby granted, free of charge, to any person obtaining
 *   a copy of this software and associated documentation files (the
 *   "Software"), to deal in the Software without restriction, including
 *   without limitation the rights to use, copy, modify, merge, publish,
 *   distribute, sublicense, and/or sell copies of the Software, and to
 *   permit persons to whom the Software is furnished to do so, subject to
 *   the following conditions:
 *
 *   The above copyright notice and this permission notice shall be
 *   included in all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 *   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 *   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *   OTHER DEALINGS IN THE SOFTWARE.
 ***************************************************************************/

#include "../ext/CEGUI/IrrlichtBaW/include/ExtGeometryBuffer.h"
#include "../ext/CEGUI/IrrlichtBaW/include/ExtImageCodec.h"
#include "../ext/CEGUI/IrrlichtBaW/include/ExtRenderer.h"
#include "../ext/CEGUI/IrrlichtBaW/include/ExtRenderTarget.h"
#include "../ext/CEGUI/IrrlichtBaW/include/ExtShaders.h"
#include "../ext/CEGUI/IrrlichtBaW/include/ExtTexture.h"
#include "../ext/CEGUI/IrrlichtBaW/include/ExtTextureTarget.h"
#include "../ext/CEGUI/IrrlichtBaW/include/ExtViewportTarget.h"

#include "CEGUI/ImageCodec.h"
#include "CEGUI/Exceptions.h"
#include "CEGUI/System.h"
#include "CEGUI/DefaultResourceProvider.h"
#include "CEGUI/Logger.h"
#include <algorithm>

using namespace irr;
using namespace asset;
using namespace video;

// Start of CEGUI namespace section
namespace CEGUI
{
//----------------------------------------------------------------------------//
String IrrlichtBaWRenderer::d_rendererID(
        "CEGUI::IrrlichtBaWRenderer - The IrrlichtBaW renderer.");

//----------------------------------------------------------------------------//
IrrlichtBaWRenderer& IrrlichtBaWRenderer::bootstrapSystem(IrrlichtDevice& device, const int abi)
{
    System::performVersionTest(CEGUI_VERSION_ABI, abi, CEGUI_FUNCTION_NAME);

    if (System::getSingletonPtr())
        CEGUI_THROW(InvalidRequestException(
            "CEGUI::System object is already initialised."));

    IrrlichtBaWRenderer& renderer = create(device);
    
    DefaultResourceProvider* rp(CEGUI_NEW_AO DefaultResourceProvider());

    IrrlichtBaWImageCodec& ic = createIrrlichtBaWImageCodec(*(device.getVideoDriver()));
    System::create(renderer, rp, static_cast<XMLParser*>(0), 0);

    return renderer;
}

//----------------------------------------------------------------------------//
void IrrlichtBaWRenderer::destroySystem()
{
    
    System* sys;
    if (!(sys = System::getSingletonPtr()))
        CEGUI_THROW(InvalidRequestException(
            "CEGUI::System object is not created or was already destroyed."));

    IrrlichtBaWRenderer* renderer = static_cast<IrrlichtBaWRenderer*>(sys->getRenderer());
    ResourceProvider* rp = sys->getResourceProvider();

    ImageCodec* ic = &(sys->getImageCodec());

    System::destroy();
    CEGUI_DELETE_AO ic;
    CEGUI_DELETE_AO rp;
    destroy(*renderer);
}

//----------------------------------------------------------------------------//
IrrlichtBaWImageCodec& IrrlichtBaWRenderer::createIrrlichtBaWImageCodec(irr::video::IVideoDriver& driver)
{
    return *new IrrlichtBaWImageCodec(driver);
}

//----------------------------------------------------------------------------//
IrrlichtBaWRenderer& IrrlichtBaWRenderer::create(irr::IrrlichtDevice& device, const int abi)
{
    System::performVersionTest(CEGUI_VERSION_ABI, abi, CEGUI_FUNCTION_NAME);

    return *CEGUI_NEW_AO IrrlichtBaWRenderer(device);
}

//----------------------------------------------------------------------------//
void IrrlichtBaWRenderer::destroy(IrrlichtBaWRenderer& renderer)
{
    CEGUI_DELETE_AO &renderer;
}

//----------------------------------------------------------------------------//
RenderTarget& IrrlichtBaWRenderer::getDefaultRenderTarget()
{
    return *d_defaultTarget;
}

//----------------------------------------------------------------------------//
GeometryBuffer& IrrlichtBaWRenderer::createGeometryBuffer()
{
    IrrlichtBaWGeometryBuffer* gb = CEGUI_NEW_AO IrrlichtBaWGeometryBuffer(*this);

    d_geometryBuffers.push_back(gb);
    return *gb;
}

//----------------------------------------------------------------------------//
void IrrlichtBaWRenderer::destroyGeometryBuffer(const GeometryBuffer& buffer)
{
    GeometryBufferList::iterator i = std::find(d_geometryBuffers.begin(),
                                               d_geometryBuffers.end(),
                                               &buffer);

    if (d_geometryBuffers.end() != i)
    {
        d_geometryBuffers.erase(i);
        CEGUI_DELETE_AO &buffer;
    }
}

//----------------------------------------------------------------------------//
void IrrlichtBaWRenderer::destroyAllGeometryBuffers()
{
    while (!d_geometryBuffers.empty())
        destroyGeometryBuffer(**d_geometryBuffers.begin());
}

//----------------------------------------------------------------------------//
TextureTarget* IrrlichtBaWRenderer::createTextureTarget()
{
    TextureTarget* tt = CEGUI_NEW_AO IrrlichtBaWTextureTarget(*this);
    d_textureTargets.push_back(tt);
    return tt;
}

//----------------------------------------------------------------------------//
void IrrlichtBaWRenderer::destroyTextureTarget(TextureTarget* target)
{
    TextureTargetList::iterator i = std::find(d_textureTargets.begin(),
                                              d_textureTargets.end(),
                                              target);

    if (d_textureTargets.end() != i)
    {
        d_textureTargets.erase(i);
        CEGUI_DELETE_AO target;
    }
}

//----------------------------------------------------------------------------//
void IrrlichtBaWRenderer::destroyAllTextureTargets()
{
    while (!d_textureTargets.empty())
        destroyTextureTarget(*d_textureTargets.begin());
}

//----------------------------------------------------------------------------//
Texture& IrrlichtBaWRenderer::createTexture(const String& name)
{
    throwIfNameExists(name);

    IrrlichtBaWTexture* t = CEGUI_NEW_AO IrrlichtBaWTexture(*this, d_driver, name);
    d_textures[name] = t;

    logTextureCreation(name);

    return *t;
}

//----------------------------------------------------------------------------//
Texture& IrrlichtBaWRenderer::createTexture(const String& name, const String& filename,
                                     const String& resourceGroup)
{
    throwIfNameExists(name);

    IrrlichtBaWTexture* t = CEGUI_NEW_AO IrrlichtBaWTexture(*this, d_driver, name, filename, resourceGroup);
    d_textures[name] = t;

    logTextureCreation(name);

    return *t;
}

//----------------------------------------------------------------------------//
Texture& IrrlichtBaWRenderer::createTexture(const String& name, const Sizef& size)
{
    throwIfNameExists(name);

    IrrlichtBaWTexture* t = CEGUI_NEW_AO IrrlichtBaWTexture(*this, d_driver, name, size);
    d_textures[name] = t;

    logTextureCreation(name);

    return *t;
}

//----------------------------------------------------------------------------//
void IrrlichtBaWRenderer::throwIfNameExists(const String& name) const
{
    if (d_textures.find(name) != d_textures.end())
        CEGUI_THROW(AlreadyExistsException(
            "[IrrlichtBaWRenderer] Texture already exists: " + name));
}

//----------------------------------------------------------------------------//
void IrrlichtBaWRenderer::logTextureCreation(const String& name)
{
    Logger* logger = Logger::getSingletonPtr();
    if (logger)
        logger->logEvent("[IrrlichtBaWRenderer] Created texture: " + name);
}

//----------------------------------------------------------------------------//
void IrrlichtBaWRenderer::destroyTexture(Texture& texture)
{
    destroyTexture(texture.getName());
}

//----------------------------------------------------------------------------//
void IrrlichtBaWRenderer::destroyTexture(const String& name)
{
    TextureMap::iterator i = d_textures.find(name);

    if (d_textures.end() != i)
    {
        logTextureDestruction(name);
        CEGUI_DELETE_AO i->second;
        d_textures.erase(i);
    }
}

//----------------------------------------------------------------------------//
void IrrlichtBaWRenderer::logTextureDestruction(const String& name)
{
    Logger* logger = Logger::getSingletonPtr();
    if (logger)
        logger->logEvent("[IrrlichtBaWRenderer] Destroyed texture: " + name);
}

//----------------------------------------------------------------------------//
void IrrlichtBaWRenderer::destroyAllTextures()
{
    while (!d_textures.empty())
        destroyTexture(d_textures.begin()->first);
}

//----------------------------------------------------------------------------//
Texture& IrrlichtBaWRenderer::getTexture(const String& name) const
{
    TextureMap::const_iterator i = d_textures.find(name);
    
    if (i == d_textures.end())
        CEGUI_THROW(UnknownObjectException(
            "Texture does not exist: " + name));

    return *i->second;
}

//----------------------------------------------------------------------------//
bool IrrlichtBaWRenderer::isTextureDefined(const String& name) const
{
    return d_textures.find(name) != d_textures.end();
}

//----------------------------------------------------------------------------//
void IrrlichtBaWRenderer::beginRendering()
{
}

//----------------------------------------------------------------------------//
void IrrlichtBaWRenderer::endRendering()
{
}

//----------------------------------------------------------------------------//
const Sizef& IrrlichtBaWRenderer::getDisplaySize() const
{
    return d_displaySize;
}

//----------------------------------------------------------------------------//
const Vector2f& IrrlichtBaWRenderer::getDisplayDPI() const
{
    return d_displayDPI;
}

//----------------------------------------------------------------------------//
uint IrrlichtBaWRenderer::getMaxTextureSize() const
{
    return d_maxTextureSize;
}

//----------------------------------------------------------------------------//
const String& IrrlichtBaWRenderer::getIdentifierString() const
{
    return d_rendererID;
}

//----------------------------------------------------------------------------//
IrrlichtBaWRenderer::IrrlichtBaWRenderer(core::smart_refctd_ptr<irr::IrrlichtDevice> device) :
    d_device(device),
    d_driver(device->getVideoDriver()),
    d_displayDPI(96, 96),
    d_maxTextureSize(2048)
{
    constructor_impl();

    /*

    TODO - PIPELINES, SHADERS, ETC

    auto uniformInfo = d_driver.createShaderUniform();
    uniformInfo.Size = sizeof(glm::mat4);
    d_uniform = d_driver.createResource(std::move(uniformInfo));

    auto depthTextureInfo = d_driver.createTexture();
    depthTextureInfo.Format = E_PIXEL_FORMAT::DEPTH_32;
    depthTextureInfo.IsRenderTarget = true;
    depthTextureInfo.IsDepthAttachment = true;
    depthTextureInfo.Width = d_driver.getWidth();
    depthTextureInfo.Height = d_driver.getHeight();

    auto renderTargetInfo = d_driver.createTexture();
    renderTargetInfo.Format = E_PIXEL_FORMAT::RGBA8;
    renderTargetInfo.Width = d_driver.getWidth();
    renderTargetInfo.Height = d_driver.getHeight();
    renderTargetInfo.IsRenderTarget = true;

    d_passInfo = d_driver.createRenderPass();
    d_passInfo.UseDefaultAttachments = false;
    d_passInfo.ColorAttachments[0] = d_driver.createTexture(STexture{renderTargetInfo});
    d_passInfo.DepthStencilAttachment = d_driver.createTexture(std::move(depthTextureInfo));
    d_passInfo.Viewport = {
        0.f, static_cast<float>(d_driver.getHeight()),
        static_cast<float>(d_driver.getWidth()), -static_cast<float>(d_driver.getHeight())
    };

    d_colorAttachment = d_passInfo.ColorAttachments.at(0);
    d_depthAttachment = d_passInfo.DepthStencilAttachment;

    renderTargetInfo.Format = E_PIXEL_FORMAT::BGRA8;
    d_screen = d_driver.createTexture(STexture{renderTargetInfo});

    d_passInfo.State.Colors[0] = { E_ATTACHMENT_STATE::LOAD, {} };
    d_passInfo.State.Depth = { E_ATTACHMENT_STATE::CLEAR, 1.f };
    d_pass = d_driver.createResource(SRenderPass{d_passInfo});

    auto pipelineInfo = d_driver.createPipeline();
    pipelineInfo.Rasterizer.CullMode = E_CULL_MODE::FRONT_FACE;
    auto shaderInfo = d_driver.createShader();

    shaderInfo.VSSource = IrrlichtBaWVertexShader;
    shaderInfo.FSSource = IrrlichtBaWFragmentShader;

    pipelineInfo.Shaders = d_driver.createResource(std::move(shaderInfo));
    pipelineInfo.Layout.Attributes[0][0] = {
        E_ATTRIBUTE_TYPE::POSITION,
        E_ATTRIBUTE_FORMAT::FLOAT3,
    };
    pipelineInfo.Layout.Attributes[0][1] = {
        E_ATTRIBUTE_TYPE::TEXTURE_COORDINATE,
        E_ATTRIBUTE_FORMAT::FLOAT2,
    };
    pipelineInfo.Layout.Attributes[0][2] = {
        E_ATTRIBUTE_TYPE::COLOR,
        E_ATTRIBUTE_FORMAT::FLOAT4,
    };
    pipelineInfo.Rasterizer.FrontFaceMode = saga::video::E_FRONT_FACE_MODE::CLOCKWISE;
    d_pipeline = d_driver.createResource(std::move(pipelineInfo));

    */
}

//----------------------------------------------------------------------------//
void IrrlichtBaWRenderer::present()
{
    /*
    not sure what about it now
    
    d_driver.begin();
    d_driver.blitTexture(
        d_passInfo.ColorAttachments[0], d_screen
    );
    d_driver.end();
    d_driver.submit();
    d_driver.present(d_screen);
    */
}

//----------------------------------------------------------------------------//
void IrrlichtBaWRenderer::setActiveRenderTarget(RenderTarget* t)
{
    /*
    not sure what about it now

    d_activeRenderTarget = t;
    const auto& rect = getActiveViewport();
    d_passInfo.Viewport = { rect.left(), rect.top(), rect.getWidth(), rect.getHeight() };

    d_driver.destroyRenderPass(d_pass);
    d_pass = d_driver.createResource(SRenderPass{d_passInfo});
    */
}

//----------------------------------------------------------------------------//
void IrrlichtBaWRenderer::bindColorAttachment(const irr::core::smart_refctd_ptr<irr::asset::ICPUImageView> t)
{
    /*
    descriptor sets probably

    d_passInfo.ColorAttachments[0] = t;
    */
}

//----------------------------------------------------------------------------//
const Rectf& IrrlichtBaWRenderer::getActiveViewport() const
{
    return d_activeRenderTarget->getArea();
}

//----------------------------------------------------------------------------//
IrrlichtBaWRenderer::~IrrlichtBaWRenderer()
{
    destroyAllGeometryBuffers();
    destroyAllTextureTargets();
    destroyAllTextures();

    delete d_defaultTarget;
}

//----------------------------------------------------------------------------//
void IrrlichtBaWRenderer::constructor_impl()
{
    // create default target & rendering root (surface) that uses it
    d_defaultTarget = CEGUI_NEW_AO IrrlichtBaWViewportTarget(*this);
}

//----------------------------------------------------------------------------//
void IrrlichtBaWRenderer::setDisplaySize(const Sizef& sz)
{
    if (sz != d_displaySize)
    {
        d_displaySize = sz;

        // FIXME: This is probably not the right thing to do in all cases.
        Rectf area(d_defaultTarget->getArea());
        area.setSize(sz);
        d_defaultTarget->setArea(area);
    }
}

//----------------------------------------------------------------------------//

} // CEGUI namespace

//----------------------------------------------------------------------------//
// Implementation of template base class
#include "./RenderTarget.inl"

