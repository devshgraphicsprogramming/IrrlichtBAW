/***********************************************************************
    created:    Tue Jan 08 2019
    author:     Manh Nguyen Tien
*************************************************************************/
/***************************************************************************
 *   Copyright (C) 2004 - 2009 Paul D Turner & The CEGUI Development Team
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
#include "../ext/CEGUI/IrrlichtBaW/include/ExtImageCodec.h"
#include "../ext/CEGUI/IrrlichtBaW/include/ExtTexture.h"
#include "CEGUI/Exceptions.h"
#include "CEGUI/Size.h"
#include <irrlicht.h>

// Start of CEGUI namespace section
namespace CEGUI
{
//----------------------------------------------------------------------------//
IrrlichtBaWImageCodec::IrrlichtBaWImageCodec(irr::video::IVideoDriver& driver) :
    ImageCodec("IrrlichtBaWImageCodec - "
               "Integrated ImageCodec using the IrrlichtBaW engine."),
    d_driver(driver)
{
}

//----------------------------------------------------------------------------//
Texture* IrrlichtBaWImageCodec::load(const RawDataContainer& data, Texture* result)
{
    auto textureHandle = d_driver.createTexture((unsigned char*)(data.getDataPtr()), data.getSize());

    // load the resulting image into the texture
    CEGUI_TRY
    {
       auto texture = static_cast<IrrlichtBaWTexture*>(result);
       texture->setTexture(textureHandle);
    }
    CEGUI_CATCH (...)
    {
        // cleanup when there's an exception
        d_driver.destroyTexture(textureHandle);
        CEGUI_RETHROW;
    }

    return result;
}

//----------------------------------------------------------------------------//

} // End of  CEGUI namespace section
