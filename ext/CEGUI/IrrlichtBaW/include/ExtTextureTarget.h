/***********************************************************************
    created:    Sat Jan 16 2019
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
#ifndef _CEGUISaga3DTextureTarget_h_
#define _CEGUISaga3DTextureTarget_h_

#include "../../TextureTarget.h"
#include "CEGUI/RendererModules/Saga3D/RenderTarget.h"

#if defined(_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable : 4250)
#endif

// Start of CEGUI namespace section
namespace CEGUI
{
//! CEGUI::TextureTarget implementation for the Saga3D engine.
class SAGA3D_GUIRENDERER_API Saga3DTextureTarget : public Saga3DRenderTarget<TextureTarget>
{
public:
    //! Constructor.
    Saga3DTextureTarget(Saga3DRenderer& owner);
    //! Destructor.
    virtual ~Saga3DTextureTarget();

    // implementation of RenderTarget interface
    bool isImageryCache() const;
    // implement CEGUI::TextureTarget interface.
    void clear();
    Texture& getTexture() const;
    void declareRenderSize(const Sizef& sz);
    bool isRenderingInverted() const;

protected:
    //! helper to generate unique texture names
    static String generateTextureName();
    //! static data used for creating texture names
    static uint s_textureNumber;
    //! default / initial size for the underlying texture.
    static const float DEFAULT_SIZE;
    //! The irrlicht render target texture we'll be drawing to
    saga::video::TextureHandle d_texture;
    //! This wraps d_texture so it can be used by the core CEGUI lib.
    Saga3DTexture* d_CEGUITexture;
    //! Render pass for manipulating this render target
    saga::video::RenderPassHandle d_pass;
};

} // End of  CEGUI namespace section

#if defined(_MSC_VER)
#   pragma warning(pop)
#endif

#endif  // end of guard _CEGUISaga3DTextureTarget_h_
