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
#ifndef _CEGUIIrrlichtBaWViewportTarget_h_
#define _CEGUIIrrlichtBaWViewportTarget_h_

#include <CEGUI/Rect.h>
#include "../ext/CEGUI/IrrlichtBaW/include/ExtRenderTarget.h"

// Start of CEGUI namespace section
namespace CEGUI
{
/*!
\brief
    Saga3D implementation of a RenderTarget that represents an on-scren viewport.
*/
class IrrlichtBaWViewportTarget : public Saga3DRenderTarget<RenderTarget>
{
public:
    /*!
    \brief
        Construct a default IrrlichtBaWViewportTarget that uses the currently
        defined Saga3D viewport as it's initial area.
    */
    IrrlichtBaWViewportTarget(Saga3DRenderer& owner);

    virtual ~IrrlichtBaWViewportTarget();

    /*!
    \brief
        Construct a IrrlichtBaWViewportTarget that uses the specified Rect as it's
        initial area.

    \param area
        Rect object describing the initial viewport area that should be used for
        the RenderTarget.
    */
    IrrlichtBaWViewportTarget(Saga3DRenderer& owner, const Rectf& area);

    // implementations of RenderTarget interface
    bool isImageryCache() const;
};

} // End of  CEGUI namespace section

#endif  // end of guard _CEGUIIrrlichtBaWViewportTarget_h_
