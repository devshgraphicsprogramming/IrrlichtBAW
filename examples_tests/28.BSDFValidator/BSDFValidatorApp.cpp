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

#include "IrrlichtDevice.h"
#include "../../ext/CEGUI/ExtCEGUI.h"

#include <iostream>
#include <memory>

namespace irr
{

BSDFValidatorApp::BSDFValidatorApp(IrrlichtDevice& device)
    : m_GUI(ext::cegui::createGUIManager(&device)),
      m_FileSystem(device.getFileSystem())
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
}

void BSDFValidatorApp::RenderGUI()
{
    m_GUI->render();
}

void BSDFValidatorApp::EventFunctionDefinitionBrowse(const CEGUI::EventArgs& e)
{
    const auto p = m_GUI->openFileDialog(s_FunctionDefinitionFileDialogTitle, m_FunctionDefinitionFileDialogFilters);
    if (p.first)
    {
        LoadDefinitions(p.second);
    }
}

void BSDFValidatorApp::LoadDefinitions(const std::string& path)
{
    std::cout << "The file to read is at path: " << path << std::endl;

    io::IReadFile* file = m_FileSystem->createAndOpenFile(path.c_str());
    std::unique_ptr<unsigned char> buffer(new unsigned char[file->getSize()]);
    file->read(buffer.get(), file->getSize());

    std::cout << "The function definitions are: " << std::endl;
    for (unsigned int i = 0; i < file->getSize(); i++)
        std::cout << buffer.get()[i];
    std::cout << std::endl;
}

}   // irr