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
#include "../ext/CEGUI/IrrlichtBaW/include/ExtTexture.h"
#include "CEGUI/Exceptions.h"
#include "CEGUI/ImageCodec.h"
#include "CEGUI/System.h"

using namespace irr;
using namespace asset;
using namespace video;

// Start of CEGUI namespace section
namespace CEGUI
{
//----------------------------------------------------------------------------//
uint32 IrrlichtBaWTexture::d_textureNumber = 0;

//----------------------------------------------------------------------------//
const String& IrrlichtBaWTexture::getName() const
{
    return d_name;
}

//----------------------------------------------------------------------------//
const Sizef& IrrlichtBaWTexture::getSize() const
{
    return d_size;
}

//----------------------------------------------------------------------------//
const Sizef& IrrlichtBaWTexture::getOriginalDataSize() const
{
    return d_dataSize;
}

//----------------------------------------------------------------------------//
const Vector2f& IrrlichtBaWTexture::getTexelScaling() const
{
    return d_texelScaling;
}

//----------------------------------------------------------------------------//
void IrrlichtBaWTexture::loadFromFile(const String& filename, const String& resourceGroup)
{
    // get and check existence of CEGUI::System object
    System* sys = System::getSingletonPtr();
    if (!sys)
        CEGUI_THROW(RendererException(
            "CEGUI::System object has not been created!"));

    // load file to memory via resource provider
    RawDataContainer texFile;
    sys->getResourceProvider()->loadRawDataContainer(filename, texFile, resourceGroup);

    Texture* res = sys->getImageCodec().load(texFile, this);

    // unload file data buffer
    sys->getResourceProvider()->unloadRawDataContainer(texFile);

    // throw exception if data was load loaded to texture.
    if (!res)
        CEGUI_THROW(RendererException(
            sys->getImageCodec().getIdentifierString() +
            " failed to load image '" + filename + "'."));
}

//----------------------------------------------------------------------------//
void IrrlichtBaWTexture::setTexture(const saga::video::TextureHandle handle)
{
    d_texture = handle;
    const auto& texture = d_driver.getTexture(d_texture);
    d_size = d_dataSize = Sizef(
       static_cast<float>(texture.Width),
       static_cast<float>(texture.Height));

   updateCachedScaleValues();
}

//----------------------------------------------------------------------------//
void IrrlichtBaWTexture::loadFromMemory(const void* buffer,
                                 const Sizef& buffer_size,
                                 PixelFormat format)
{
    auto textureInfo = d_driver.createTexture();
    textureInfo.Format = E_PIXEL_FORMAT::RGBA8;
    textureInfo.Width = buffer_size.d_width;
    textureInfo.Height = buffer_size.d_height;
    auto& textureData = textureInfo.Contents[0][0].Data;
    std::size_t size = buffer_size.d_width * buffer_size.d_height * 4;
    textureData.reserve(size);
    memcpy(textureData.data(), buffer, size);
    auto t = d_driver.createTexture(std::move(textureInfo));
    setTexture(t);
}

//----------------------------------------------------------------------------//
void IrrlichtBaWTexture::blitFromMemory(const void* /*sourceData*/, const Rectf& /*area*/)
{
    // do nothing
}

//----------------------------------------------------------------------------//
void IrrlichtBaWTexture::blitToMemory(void* /*targetData*/)
{
    // do nothing
}

//----------------------------------------------------------------------------//
IrrlichtBaWTexture::IrrlichtBaWTexture(IrrlichtBaWRenderer& owner, saga::video::IVideoDriver& driver, const String& name) :
    d_owner(owner),
    d_driver(driver),
    d_size(0, 0),
    d_dataSize(0, 0),
    d_texelScaling(0, 0),
    d_name(name)
{

}

//----------------------------------------------------------------------------//
IrrlichtBaWTexture::IrrlichtBaWTexture(IrrlichtBaWRenderer& owner, saga::video::IVideoDriver& driver, const String& name,
                             const String& filename, const String& resourceGroup) :
    d_owner(owner),
    d_driver(driver),
    d_size(0, 0),
    d_dataSize(0, 0),
    d_texelScaling(0, 0),
    d_name(name)
{
    loadFromFile(filename, resourceGroup);
}

//----------------------------------------------------------------------------//
IrrlichtBaWTexture::IrrlichtBaWTexture(IrrlichtBaWRenderer& owner, saga::video::IVideoDriver& driver, const String& name, const Sizef& sz) :
    d_owner(owner),
    d_driver(driver),
    d_size(0, 0),
    d_dataSize(0, 0),
    d_texelScaling(0, 0),
    d_name(name)
{
    d_size.d_width = sz.d_width;
    d_size.d_height = sz.d_height;
    d_dataSize = sz;
    updateCachedScaleValues();
}

//----------------------------------------------------------------------------//
IrrlichtBaWTexture::~IrrlichtBaWTexture()
{
    d_driver.destroyTexture(d_texture);
}

//----------------------------------------------------------------------------//
void IrrlichtBaWTexture::setOriginalDataSize(const Sizef& sz)
{
    d_dataSize = sz;
    updateCachedScaleValues();
}

//----------------------------------------------------------------------------//
void IrrlichtBaWTexture::updateCachedScaleValues()
{
    //
    // calculate what to use for x scale
    //
    const float orgW = d_dataSize.d_width;
    const float texW = d_size.d_width;

    // if texture and original data width are the same, scale is based
    // on the original size.
    // if texture is wider (and source data was not stretched), scale
    // is based on the size of the resulting texture.
    d_texelScaling.d_x = 1.0f / ((orgW == texW) ? orgW : texW);

    //
    // calculate what to use for y scale
    //
    const float orgH = d_dataSize.d_height;
    const float texH = d_size.d_height;

    // if texture and original data height are the same, scale is based
    // on the original size.
    // if texture is taller (and source data was not stretched), scale
    // is based on the size of the resulting texture.
    d_texelScaling.d_y = 1.0f / ((orgH == texH) ? orgH : texH);
}

//----------------------------------------------------------------------------//
bool IrrlichtBaWTexture::isPixelFormatSupported(const PixelFormat) const
{
    return true;
}

//----------------------------------------------------------------------------//

} // End of  CEGUI namespace section
