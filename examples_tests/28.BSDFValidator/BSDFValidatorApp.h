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

#ifndef _IRR_BSDF_VALIDATOR_APP_INCLUDED_
#define _IRR_BSDF_VALIDATOR_APP_INCLUDED_

#include "ShaderManager.h"
#include "irrlicht.h"

#include <string>
#include <vector>

// Forward Declarations
namespace irr { namespace ext { namespace cegui { class GUIManager; } } }
namespace CEGUI { class EventArgs; }

class BSDFValidatorApp
{
public:
    BSDFValidatorApp(irr::IrrlichtDevice* device);

    void RenderGUI();
    void RenderMesh();

private:
    void EventFunctionDefinitionBrowse(const CEGUI::EventArgs& e);
    void EventResetFunctionDefinitions(const CEGUI::EventArgs& e);
    
    std::string LoadDefinitions(const std::string& path);

    static constexpr const char* s_FunctionDefinitionFileDialogTitle = "Select Function Definitions";
    const std::vector<std::string> m_FunctionDefinitionFileDialogFilters =
    {
            "Shaders (*.glsl)",
            "*.glsl"
    };

    irr::ext::cegui::GUIManager* m_GUI;
    irr::io::IFileSystem* m_FileSystem;
    irr::video::IVideoDriver* m_VideoDriver;
    irr::core::smart_refctd_ptr<ShaderManager> m_ShaderManager;
    irr::video::SGPUMaterial m_Material;

    /* !!!....TEMPORARY....!!! */
    irr::core::smart_refctd_ptr<irr::video::IGPUMeshBuffer> m_Mesh;
};

#endif // _IRR_BSDF_VALIDATOR_APP_INCLUDED_
