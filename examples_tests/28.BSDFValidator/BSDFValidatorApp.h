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
#define _IRR_BRDF_VALIDATOR_APP_INCLUDED_

#include <string>
#include <vector>

// Forward Declarations
namespace irr { class IrrlichtDevice; }
namespace irr { namespace io { class IFileSystem; } }
namespace irr { namespace ext { namespace cegui { class GUIManager; } } }
namespace CEGUI { class EventArgs; }

namespace irr
{

class BSDFValidatorApp
{
public:
    BSDFValidatorApp(IrrlichtDevice& device);

    void RenderGUI();

private:
    void EventFunctionDefinitionBrowse(const CEGUI::EventArgs& e);
    
    void LoadDefinitions(const std::string& path);

    static constexpr const char* s_FunctionDefinitionFileDialogTitle = "Select Function Definitions";
    const std::vector<std::string> m_FunctionDefinitionFileDialogFilters =
    {
            "Shaders (*.frag *.vert)",
            "*.frag *.vert"
    };

    ext::cegui::GUIManager* m_GUI;
    io::IFileSystem* m_FileSystem;
};

} // namespace irr

#endif // _IRR_BSDF_VALIDATOR_APP_INCLUDED_
