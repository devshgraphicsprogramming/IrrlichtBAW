/***********************************************************************
    created:    Sat Jan 16 2019
    author:     Manh Nguyen Tien
*************************************************************************/
/***************************************************************************
 *   Copyright (C) 2004 - 2010 Paul D Turner & The CEGUI Development Team
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
#ifndef _CEGUISaga3DRenderTarget_h_
#define _CEGUISaga3DRenderTarget_h_

#include "../../RenderTarget.h"
#include "CEGUI/RendererModules/Saga3D/Renderer.h"
#include "../../Rect.h"
#include <Saga.h>

#if defined(_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable : 4251)
#endif

// Start of CEGUI namespace section
namespace CEGUI
{
//! Intermediate RenderTarget 
template<typename T = RenderTarget>
class SAGA3D_GUIRENDERER_API Saga3DRenderTarget : public T
{
public:
    //! Constructor
    Saga3DRenderTarget(Saga3DRenderer& owner);

    //! Destructor
    virtual ~Saga3DRenderTarget();

    // implement parts of CEGUI::RenderTarget interface
    void draw(const GeometryBuffer& buffer);
    void draw(const RenderQueue& queue);
    void setArea(const Rectf& area);
    const Rectf& getArea() const;
    void activate();
    void deactivate();
    void unprojectPoint(const GeometryBuffer& buff,
                        const Vector2f& p_in, Vector2f& p_out) const;

    bool isImageryCache() const { return false; }

protected:
    //! helper that initialises the cached matrix
    virtual void updateMatrix() const;

    //! Saga3DRenderer object that owns this RenderTarget
    Saga3DRenderer& d_owner;
    //! Saga's video driver we are using
    saga::video::IVideoDriver& d_driver;
    //! Holds defined area for the RenderTarget
    Rectf d_area;
    //! Tangent of the y FOV half-angle; used to calculate viewing distance.
    static const double d_yfov_tan;
    //! Saved copy of projection matrix
    mutable glm::mat4 d_matrix;
    //! True if saved matrix is up to date
    mutable bool d_matrixValid;
    //! Tracks viewing distance (this is set up at the same time as d_matrix)
    mutable double d_viewDistance;
};

} // End of  CEGUI namespace section

#if defined(_MSC_VER)
#   pragma warning(pop)
#endif

#endif  // end of guard _CEGUISaga3DRenderTarget_h_
