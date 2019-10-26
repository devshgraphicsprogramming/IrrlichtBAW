/*

MIT License

Copyright (c) 2019 Achal Pandey

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include "BSDFValidatorApp.h"

#include "ShaderManager.h"
#include "IrrlichtDevice.h"
#include "../../ext/CEGUI/ExtCEGUI.h"

#include <iostream>
#include <memory>

namespace irr
{

BSDFValidatorApp::BSDFValidatorApp(IrrlichtDevice* device)
    : m_GUI(ext::cegui::createGUIManager(device)),
      m_FileSystem(device->getFileSystem()),
      m_VideoDriver(device->getVideoDriver()),
      m_ShaderManager(new irr::ShaderManager(m_VideoDriver->getGPUProgrammingServices()))
{
    m_GUI->init();
    m_GUI->createRootWindowFromLayout(
        ext::cegui::readWindowLayout("../../media/BSDFValidator/MainWindow.layout")
    );

    // Setup Load Function Definitions Button
    auto root = m_GUI->getRootWindow();
    auto loadDefinitionsButton = static_cast<::CEGUI::PushButton*>(
        root->getChild("LoadDefinitionsButton"));
    loadDefinitionsButton->subscribeEvent(::CEGUI::PushButton::EventClicked,
        ::CEGUI::Event::Subscriber(&BSDFValidatorApp::EventFunctionDefinitionBrowse, this));

    /* !!!....TEMPORARY....!!! */
    // This whole mesh business is just to check if the custom shaders
    // are woring as intended. It will not make it to the final version.
#include "irr/irrpack.h"
    struct VertexStruct
    {
        float position[3]; /// uses float hence need 4 byte alignment
    } PACK_STRUCT;
#include "irr/irrunpack.h"

    // Create mesh.
    VertexStruct vertices[4] =
    {
        { -0.5f, -0.5f, 0.f },
        { 0.5f, -0.5f, 0.f },
        { 0.5f, 0.5f, 0.f },
        { -0.5f, 0.5f, 0.f }
    };

    uint16_t indices[6] =
    {
        0, 1, 2,
        2, 3, 0
    };

    // Upload mesh to the GPU
    auto upStreamBuff = m_VideoDriver->getDefaultUpStreamingBuffer();
    const void* dataToPlace[2] = { vertices, indices };
    uint32_t offsets[2] = { video::StreamingTransientDataBufferMT<>::invalid_address,video::StreamingTransientDataBufferMT<>::invalid_address };
    uint32_t alignments[2] = { sizeof(decltype(vertices[0u])),sizeof(decltype(indices[0u])) };
    uint32_t sizes[2] = { sizeof(vertices),sizeof(indices) };
    upStreamBuff->multi_place(2u, (const void* const*)dataToPlace, offsets, sizes, alignments);

    if (upStreamBuff->needsManualFlushOrInvalidate())
    {
        auto upStreamMem = upStreamBuff->getBuffer()->getBoundMemory();
        m_VideoDriver->flushMappedMemoryRanges({ video::IDriverMemoryAllocation::MappedMemoryRange(upStreamMem, offsets[0],sizes[0]),video::IDriverMemoryAllocation::MappedMemoryRange(upStreamMem,offsets[1],sizes[1]) });
    }

    m_Mesh = core::make_smart_refctd_ptr<video::IGPUMeshBuffer>();

    {
        // Define vertex attribute layout. 
        auto desc = m_VideoDriver->createGPUMeshDataFormatDesc();

        {
            auto buff = core::smart_refctd_ptr<video::IGPUBuffer>(upStreamBuff->getBuffer());

            // Position attribute.
            desc->setVertexAttrBuffer(core::smart_refctd_ptr(buff), asset::EVAI_ATTR0, asset::EF_R32G32B32_SFLOAT, sizeof(VertexStruct), 0);

            desc->setIndexBuffer(std::move(buff));
        }

        m_Mesh->setIndexBufferOffset(offsets[1]);
        m_Mesh->setIndexType(asset::EIT_16BIT);
        m_Mesh->setIndexCount(6);
        m_Mesh->setMeshDataAndFormat(std::move(desc));
    }

    m_VideoDriver->setTransform(video::E4X3TS_WORLD, core::matrix4x3());

    upStreamBuff->multi_free(2u, (uint32_t*)&offsets, (uint32_t*)&sizes, m_VideoDriver->placeFence());

}

BSDFValidatorApp::~BSDFValidatorApp()
{
    delete m_ShaderManager;
}

void BSDFValidatorApp::RenderGUI()
{
    m_GUI->render();
}

void BSDFValidatorApp::RenderMesh()
{
    m_Material.MaterialType = m_ShaderManager->GetShader();
    m_VideoDriver->setMaterial(m_Material);

    /* !!!....TEMPORARY....!!! */
    m_VideoDriver->drawMeshBuffer(m_Mesh.get());
}

void BSDFValidatorApp::EventFunctionDefinitionBrowse(const CEGUI::EventArgs& e)
{
    const auto p = m_GUI->openFileDialog(s_FunctionDefinitionFileDialogTitle, m_FunctionDefinitionFileDialogFilters);
    if (p.first)
    {
        std::string functionDefinitions = LoadDefinitions(p.second);
        if (!functionDefinitions.empty())
            m_ShaderManager->UpdateShader(functionDefinitions);
        else
            std::cout << "Function Definitions are empty" << std::endl;
    }
}

std::string BSDFValidatorApp::LoadDefinitions(const std::string& path)
{
    std::cout << "The file to read is at path: " << path << std::endl;

    io::IReadFile* file = m_FileSystem->createAndOpenFile(path.c_str());
    std::unique_ptr<unsigned char> buffer(new unsigned char[file->getSize()]);
    file->read(buffer.get(), file->getSize());

    // Print and assemble function definitions
    // Printing is for debugging purposes - it'll be removed later.
    std::cout << "The function definitions are: " << std::endl;
    std::string functionDefinitions = "";
    for (unsigned int i = 0; i < file->getSize(); i++)
    {
        functionDefinitions += buffer.get()[i];
        std::cout << buffer.get()[i];
    }
        
    std::cout << std::endl;
    return functionDefinitions;
}

}   // irr