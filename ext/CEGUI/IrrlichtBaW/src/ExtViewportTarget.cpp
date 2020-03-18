/***********************************************************************
    created:    Tue Mar 12 2019
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
#include "../ext/CEGUI/IrrlichtBaW/include/ExtViewportTarget.h"
#include <CEGUI/RenderQueue.h>
#include <CEGUI/GeometryBuffer.h>
#include <CEGUI/Exceptions.h>

// Start of CEGUI namespace section
namespace CEGUI
{
//----------------------------------------------------------------------------//
IrrlichtBaWViewportTarget::IrrlichtBaWViewportTarget(IrrlichtBaWRenderer& owner) :
    IrrlichtBaWRenderTarget<RenderTarget>(owner)
{
    // viewport area defaults to whatever the current IrrlichtBaW viewport is set to
    const auto& driver = owner.getDevice().getVideoDriver();

    const Rectf init_area(0, 0,
                    static_cast<float>(driver->getWidth()), driver->getHeight());

    setArea(init_area);
}

//----------------------------------------------------------------------------//
IrrlichtBaWViewportTarget::IrrlichtBaWViewportTarget(IrrlichtBaWRenderer& owner,
    const Rectf& area) :
        IrrlichtBaWRenderTarget<RenderTarget>(owner)
{
    setArea(area);
}

//----------------------------------------------------------------------------//
IrrlichtBaWViewportTarget::~IrrlichtBaWViewportTarget()
{
}

//----------------------------------------------------------------------------//
bool IrrlichtBaWViewportTarget::isImageryCache() const
{
    return false;
}

//----------------------------------------------------------------------------//


} // End of  CEGUI namespace section

//----------------------------------------------------------------------------//
// Implementation of base class
#include "./RenderTarget.inl"


