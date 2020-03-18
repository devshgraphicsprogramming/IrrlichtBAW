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
#include "../ext/CEGUI/IrrlichtBaW/include/ExtTextureTarget.h"
#include "../ext/CEGUI/IrrlichtBaW/include/ExtTexture.h"
#include "CEGUI/PropertyHelper.h"

// Start of CEGUI namespace section
namespace CEGUI
{
//----------------------------------------------------------------------------//
uint IrrlichtBaWTextureTarget::s_textureNumber = 0;
const float IrrlichtBaWTextureTarget::DEFAULT_SIZE = 128.0f;

//----------------------------------------------------------------------------//
IrrlichtBaWTextureTarget::IrrlichtBaWTextureTarget(IrrlichtBaWRenderer& owner) :
    IrrlichtBaWRenderTarget<TextureTarget>(owner),
    d_CEGUITexture(0)
{
    auto renderTargetInfo = d_driver.createTexture();
    renderTargetInfo.Format = saga::video::E_PIXEL_FORMAT::RGBA8;
    renderTargetInfo.Width = DEFAULT_SIZE;
    renderTargetInfo.Height = DEFAULT_SIZE;
    renderTargetInfo.IsRenderTarget = true;
    d_texture = d_driver.createTexture(std::move(renderTargetInfo));

    d_CEGUITexture = static_cast<IrrlichtBaWTexture*>(
        &d_owner.createTexture(generateTextureName()));
    d_CEGUITexture->setTexture(d_texture);

    // setup area and cause the initial texture to be generated.
    declareRenderSize(Sizef(DEFAULT_SIZE, DEFAULT_SIZE));

    // auto pass = d_driver.createRenderPass();
    // pass.ColorAttachments[0] = d_texture;
    // pass.State.Colors[0] = {
    //     saga::video::E_RENDER_PASS_STATE::CLEAR,
    //     { 0.f, 0.f, 0.0f, 1.f }
    // };
    // pass.DrawGeometry = false;
    // pass.UseDefaultAttachments = false;

    // d_pass = d_driver.createResource(std::move(pass));
}

//----------------------------------------------------------------------------//
IrrlichtBaWTextureTarget::~IrrlichtBaWTextureTarget()
{
    d_owner.destroyTexture(*d_CEGUITexture);
    d_driver.destroyRenderPass(d_pass);
}

//----------------------------------------------------------------------------//
bool IrrlichtBaWTextureTarget::isImageryCache() const
{
    return true;
}

//----------------------------------------------------------------------------//
void IrrlichtBaWTextureTarget::clear()
{
    d_driver.enqueuePass(d_pass);
}

//----------------------------------------------------------------------------//
Texture& IrrlichtBaWTextureTarget::getTexture() const
{
    return *d_CEGUITexture;
}

//----------------------------------------------------------------------------//
void IrrlichtBaWTextureTarget::declareRenderSize(const Sizef& sz)
{
    setArea(Rectf(d_area.getPosition(), sz));
}

//----------------------------------------------------------------------------//
bool IrrlichtBaWTextureTarget::isRenderingInverted() const
{
    return false;
}

//----------------------------------------------------------------------------//
String IrrlichtBaWTextureTarget::generateTextureName()
{
    String tmp("_IrrlichtBaW_tt_tex_");
    tmp.append(PropertyHelper<uint>::toString(s_textureNumber++));

    return tmp;
}
//----------------------------------------------------------------------------//

} // End of  CEGUI namespace section

//----------------------------------------------------------------------------//
// Implementation of template base class
#include "./RenderTarget.inl"

