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
#ifndef _CEGUIIrrlichtBawTexture_h_
#define _CEGUIIrrlichtBawTexture_h_

#include <CEGUI/Texture.h>
#include <CEGUI/String.h>
#include "../ext/CEGUI/IrrlichtBaW/include/ExtRenderer.h"

namespace CEGUI
{
//! Implementation of the CEGUI::Texture class for IrrlichtBaW.
class IrrlichtBaWTexture : public Texture
{
public:
    void setTexture(const irr::core::smart_refctd_ptr<irr::asset::ICPUImageView>&& handle);
    auto getTexture() const { return texture; }
    void setOriginalDataSize(const Sizef& sz);

public:
    // implement CEGUI::Texture interface
    const String& getName() const;
    const Sizef& getSize() const;
    const Sizef& getOriginalDataSize() const;
    const Vector2f& getTexelScaling() const;
    void loadFromFile(const String& filename, const String& resourceGroup);
    void loadFromMemory(const void* buffer, const Sizef& buffer_size,
                                PixelFormat pixel_format);
    void blitFromMemory(const void* sourceData, const Rectf& area);
    void blitToMemory(void* targetData);
    bool isPixelFormatSupported(const PixelFormat fmt) const;

protected:
    // we all need a little help from out friends ;)
    friend Texture& IrrlichtBawRenderer::createTexture(const String&);
    friend Texture& IrrlichtBawRenderer::createTexture(const String&, const String&, const String&);
    friend Texture& IrrlichtBawRenderer::createTexture(const String&, const Sizef&);
    friend void IrrlichtBawRenderer::destroyTexture(Texture&);
    friend void IrrlichtBawRenderer::destroyTexture(const String&);

    //! standard constructor
    IrrlichtBaWTexture(IrrlichtBawRenderer& owner, irr::video::IVideoDriver& driver, const String& name);
    //! construct texture via an image file.
    IrrlichtBaWTexture(IrrlichtBawRenderer& owner, irr::video::IVideoDriver& driver, const String& name,
                  const String& filename, const String& resourceGroup);
    //! construct texture with a specified initial size.
    IrrlichtBaWTexture(IrrlichtBawRenderer& owner, irr::video::IVideoDriver& driver,const String& name, const Sizef& sz);

    //! destructor.
    virtual ~IrrlichtBaWTexture();
    //! updates cached scale value used to map pixels to texture co-ords.
    void updateCachedScaleValues();

    //! Counter used to provide unique texture names.
    static uint32 d_textureNumber;
    //! Reference to the IrrlichtBawRenderer that created this texture
    IrrlichtBawRenderer& d_owner;
    //! Video driver we're to use.
    irr::video::IVideoDriver& d_driver;
    //! Handle to underlying texture.
    irr::core::smart_refctd_ptr<irr::asset::ICPUImageView> texture;
    //! Size of the texture.
    Sizef d_size;
    //! original pixel of size data loaded into texture
    Sizef d_dataSize;
    //! cached pixel to texel mapping scale values.
    Vector2f d_texelScaling;
    //! Name this texture was created with.
    const String d_name;
};

} // CEGUI namespace

#endif  // _CEGUIIrrlichtBawTexture_h_
