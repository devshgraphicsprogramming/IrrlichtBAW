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

#include "../ext/CEGUI/IrrlichtBaW/include/ExtGeometryBuffer.h"
#include "../ext/CEGUI/IrrlichtBaW/include/ExtTexture.h"
#include "CEGUI/Vertex.h"
#include "CEGUI/RenderEffect.h"

#include <iostream>

using namespace irr;
using namespace asset;
using namespace video;

// Start of CEGUI namespace section
namespace CEGUI
{
//----------------------------------------------------------------------------//
    IrrlichtBawGeometryBuffer::IrrlichtBawGeometryBuffer(IrrlichtBaWRenderer& owner) :
    d_owner(owner),
    d_driver(*owner.getDevice()->getVideoDriver()),
    d_smgr(*owner.getDevice()->getSceneManager()),
    d_activeTexture(0),
    d_clipRect(0, 0, 0, 0),
    d_clippingActive(true),
    d_translation(0, 0, 0),
    d_rotation(),
    d_pivot(0, 0, 0),
    d_matrixValid(false),
    d_effect(0)
{
    /*

    TODO 

    d_mesh = core::make_smart_refctd_ptr<irr::asset::ICPUMesh>();
    auto meshBuffer = std::make_unique<scene::CGPUMeshBuffer>();
    d_mesh->addMeshBuffer(std::move(meshBuffer));
    d_node = d_smgr.createSceneNode(d_mesh);
    */
}

//----------------------------------------------------------------------------//
IrrlichtBawGeometryBuffer::~IrrlichtBawGeometryBuffer()
{
    /*

    TODO

    auto renderPass = d_owner.getRenderPass();
    d_smgr.unregisterNode(d_node, renderPass);
    d_smgr.removeNode(d_node);
    d_smgr.removeMesh(d_mesh);
    d_driver.destroyShaderBuffer(d_buffer);
    */
}

//----------------------------------------------------------------------------//
void IrrlichtBawGeometryBuffer::draw() const
{
    if (d_vertices.empty()) return;
    if (!d_matrixValid)
        updateMatrix();

    auto matrix = d_owner.getViewProjectionMatrix() * d_matrix;
    d_driver.updateShaderUniform(d_owner.getUniform(), &matrix);

    auto renderPass = d_owner.getRenderPass();
    d_smgr.registerNode(d_node, renderPass);

    auto& passInfo = d_driver.getRenderPass(renderPass);
    const auto& viewPort = d_owner.getActiveViewport();
    passInfo.Scissor = {
        static_cast<int>(d_clipRect.left()),
        static_cast<int>(viewPort.getHeight() - d_clipRect.bottom()),
        static_cast<int>(d_clipRect.getWidth()),
        static_cast<int>(d_clipRect.getHeight())
    };
    const int pass_count = d_effect ? d_effect->getPassCount() : 1;
    for (int pass = 0; pass < pass_count; ++pass)
    {
        // set up RenderEffect
        if (d_effect)
            d_effect->performPreRenderFunctions(pass);

        // draw the batches
        size_t pos = 0;
        BatchList::const_iterator i = d_batches.begin();
        for ( ; i != d_batches.end(); ++i)
        {
            if (i->clip)
                passInfo.ScissorTest = true;
            else
                passInfo.ScissorTest = false;

            auto cmdBufferInfo = d_driver.createIndirectBuffer();
            SIndirectCommand cmd;
            cmd.VertexOffset = pos;
            cmd.VertexCount = i->vertexCount;
            cmdBufferInfo.Commands.push_back(std::move(cmd));
            auto cmdBuffer = d_driver.createResource(std::move(cmdBufferInfo));
            d_mesh->getMeshBuffer().setIndirectDrawBuffer(cmdBuffer);

            d_driver.begin();
            d_driver.beginPass(renderPass);
            d_driver.bindShaderUniform(d_owner.getUniform(), 0);
            if (i->texture != NULL_GPU_RESOURCE_HANDLE)
                d_driver.bindTexture(i->texture, 1);
            d_driver.draw();
            d_driver.endPass();
            d_driver.end();
            d_driver.submit();

            pos += i->vertexCount;
        }
    }

    // clean up RenderEffect
    if (d_effect)
        d_effect->performPostRenderFunctions();
    d_smgr.unregisterNode(d_node, renderPass);
}

//----------------------------------------------------------------------------//
void IrrlichtBawGeometryBuffer::setTranslation(const Vector3f& v)
{
    d_translation = v;
    d_matrixValid = false;
}

//----------------------------------------------------------------------------//
void IrrlichtBawGeometryBuffer::setRotation(const Quaternion& r)
{
    d_rotation = r;
    d_matrixValid = false;
}

//----------------------------------------------------------------------------//
void IrrlichtBawGeometryBuffer::setPivot(const Vector3f& p)
{
    d_pivot = p;
    d_matrixValid = false;
}

//----------------------------------------------------------------------------//
void IrrlichtBawGeometryBuffer::setClippingRegion(const Rectf& region)
{
    d_clipRect.top(ceguimax(0.0f, region.top()));
    d_clipRect.bottom(ceguimax(0.0f, region.bottom()));
    d_clipRect.left(ceguimax(0.0f, region.left()));
    d_clipRect.right(ceguimax(0.0f, region.right()));
}

//----------------------------------------------------------------------------//
void IrrlichtBawGeometryBuffer::appendVertex(const Vertex& vertex)
{
    appendGeometry(&vertex, 1);
}

//----------------------------------------------------------------------------//
void IrrlichtBawGeometryBuffer::appendGeometry(const Vertex* const vbuff,
                                        uint vertex_count)
{
    const auto t = d_activeTexture ? d_activeTexture->getTexture() : NULL_GPU_RESOURCE_HANDLE;

    if (d_batches.empty() ||
        d_batches.back().texture != t ||
        d_batches.back().clip != d_clippingActive)
    {
        BatchInfo batch = {t, 0, d_clippingActive};
        d_batches.push_back(std::move(batch));
    }
    d_batches.back().vertexCount += vertex_count;

    IrrlichtBaWVertex vertex;
    for (uint i = 0; i < vertex_count; ++i)
    {
        const auto& v = vbuff[i];
        vertex.position = { v.position.d_x, v.position.d_y, v.position.d_z };
        vertex.uv = { v.tex_coords.d_x, v.tex_coords.d_y };
        vertex.color = {
            v.colour_val.getRed(),
            v.colour_val.getGreen(),
            v.colour_val.getBlue(),
            v.colour_val.getAlpha()
        };
        d_vertices.push_back(std::move(vertex));
    }
    updateBuffer();
}

//----------------------------------------------------------------------------//
void IrrlichtBawGeometryBuffer::setActiveTexture(Texture* texture)
{
    d_activeTexture = static_cast<IrrlichtBaWTexture*>(texture);
}

//----------------------------------------------------------------------------//
void IrrlichtBawGeometryBuffer::reset()
{
    d_batches.clear();
    d_vertices.clear();
    d_activeTexture = nullptr;
}

//----------------------------------------------------------------------------//
Texture* IrrlichtBawGeometryBuffer::getActiveTexture() const
{
    return d_activeTexture;
}

//----------------------------------------------------------------------------//
uint IrrlichtBawGeometryBuffer::getVertexCount() const
{
    return d_vertices.size();
}

//----------------------------------------------------------------------------//
uint IrrlichtBawGeometryBuffer::getBatchCount() const
{
    return d_batches.size();
}

//----------------------------------------------------------------------------//
void IrrlichtBawGeometryBuffer::setRenderEffect(RenderEffect* effect)
{
    d_effect = effect;
}

//----------------------------------------------------------------------------//
RenderEffect* IrrlichtBawGeometryBuffer::getRenderEffect()
{
    return d_effect;
}

//----------------------------------------------------------------------------//
void IrrlichtBawGeometryBuffer::setClippingActive(const bool active)
{
    d_clippingActive = active;
}

//----------------------------------------------------------------------------//
bool IrrlichtBawGeometryBuffer::isClippingActive() const
{
    return d_clippingActive;
}

//----------------------------------------------------------------------------//
void IrrlichtBawGeometryBuffer::updateMatrix() const
{
    glm::mat4& modelMatrix = d_matrix;
    modelMatrix = glm::mat4(1.f);

    const glm::vec3 final_trans(d_translation.d_x + d_pivot.d_x,
                                d_translation.d_y + d_pivot.d_y,
                                d_translation.d_z + d_pivot.d_z);

    modelMatrix = glm::translate(modelMatrix, final_trans);

    glm::quat rotationQuat = glm::quat(d_rotation.d_w, d_rotation.d_x, d_rotation.d_y, d_rotation.d_z);
    glm::mat4 rotation_matrix = glm::mat4_cast(rotationQuat);

    modelMatrix = modelMatrix * rotation_matrix;

    glm::vec3 transl = glm::vec3(-d_pivot.d_x, -d_pivot.d_y, -d_pivot.d_z);
    glm::mat4 translMatrix = glm::translate(glm::mat4(1.f), transl);
    modelMatrix =  modelMatrix * translMatrix;

    d_matrixValid = true;
}

//----------------------------------------------------------------------------//
void IrrlichtBawGeometryBuffer::updateBuffer()
{
    if (d_buffer != video::NULL_GPU_RESOURCE_HANDLE)
        d_driver.destroyShaderBuffer(d_buffer);
    auto bufferInfo = d_driver.createShaderBuffer();
    bufferInfo.VertexBufferBind = true;
    bufferInfo.Size = (3 + 2 + 4) * sizeof(float) * d_vertices.size();
    auto buffer = d_driver.createResource(std::move(bufferInfo));
    d_driver.updateShaderBuffer(buffer, d_vertices.data());
    d_buffer = buffer;
    auto& meshBuffer = static_cast<scene::CGPUMeshBuffer&>(d_mesh->getMeshBuffer(0));
    meshBuffer.setVertexBuffer(d_buffer);
    meshBuffer.setVertexCount(d_vertices.size());
    d_node->setPipeline(d_owner.getPipeline());
}

//----------------------------------------------------------------------------//

} // End of  CEGUI namespace section
