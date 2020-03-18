/***********************************************************************
    created:    Thu Dec 27 2018
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
#ifndef _CEGUIIrrlichtBawGeometryBuffer_h_
#define _CEGUIIrrlichtBawGeometryBuffer_h_

#include "../ext/CEGUI/IrrlichtBaW/include/ExtRenderer.h"
#include <CEGUI/GeometryBuffer.h>
#include <CEGUI/Rect.h>
#include <CEGUI/Colour.h>
#include <CEGUI/Vertex.h>
#include <CEGUI/Quaternion.h>
#include <irrlicht.h>

#include <utility>
#include <vector>

// Start of CEGUI namespace section
namespace CEGUI
{
    //! Implementation of CEGUI::GeometryBuffer for the IrrlichtBaw engine
    class IrrlichtBawGeometryBuffer : public GeometryBuffer
    {
    public:
        IrrlichtBawGeometryBuffer(IrrlichtBaWRenderer& owner);
        virtual ~IrrlichtBawGeometryBuffer();

        const auto& getMatrix() const { return d_matrix; }

        // implement CEGUI::GeometryBuffer interface.
        void draw() const;
        void setTranslation(const Vector3f& v);
        void setRotation(const Quaternion& r);
        void setPivot(const Vector3f& p);
        void setClippingRegion(const Rectf& region);
        void appendVertex(const Vertex& vertex);
        void appendGeometry(const Vertex* const vbuff, uint vertex_count);
        void setActiveTexture(Texture* texture);
        void reset();
        Texture* getActiveTexture() const;
        uint getVertexCount() const;
        uint getBatchCount() const;
        void setRenderEffect(RenderEffect* effect);
        RenderEffect* getRenderEffect();
        void setClippingActive(const bool active);
        bool isClippingActive() const;

    protected:
        void updateMatrix() const;
        void updateBuffer();

        struct IrrlichtBawVertex
        {
            core::vector3df position;
            core::vector2df uv;
            core::vector4df_SIMD color;
        };

        //! Type to track info for per-texture sub batches of geometry
        struct BatchInfo
        {
            irr::core::smart_refctd_ptr<irr::asset::ICPUImageView> texture;
            std::uint32_t vertexCount;
            bool clip;
        };
        //! IrrlichtBawRenderer object that owns this geometry buffer
        IrrlichtBaWRenderer& d_owner;
        //! IrrlichtBaW's video driver we're to use.
        irr::video::IVideoDriver& d_driver;
        //! Asset manager
        irr::scene::ISceneManager& d_smgr;
        //! Geometry's mesh
        irr::core::smart_refctd_ptr<irr::asset::ICPUMesh> d_mesh;
        //! Texture that is set as active
        IrrlichtBaWTexture* d_activeTexture;
        //! type of container that tracks BatchInfos.
        typedef std::vector<BatchInfo> BatchList;
        //! rectangular clip region
        Rectf d_clipRect;
        //! whether clipping will be active for the current batch
        bool d_clippingActive;
        //! translation vector
        Vector3f d_translation;
        //! rotation quaternion
        Quaternion d_rotation;
        //! pivot point for rotation
        Vector3f d_pivot;
        //! model matrix cache
        mutable core::matrix4SIMD d_matrix;
        //! true when d_matrix is valid and up to date
        mutable bool d_matrixValid;
        //! RenderEffect that will be used by the GeometryBuffer
        RenderEffect* d_effect;
        //! Type of container used to queue the geometry
        typedef std::vector<IrrlichtBawVertex> VertexList;
        //! Type of container used for indexes
        typedef std::vector<std::uint32_t> IndexList;
        //! List of texture batches added to the geometry buffer
        BatchList d_batches;
        //! Container where added geometry is stored.
        VertexList d_vertices;
    };
} // CEGUI namespace

#endif  // end of guard _CEGUIIrrlichtBawGeometryBuffer_h_
