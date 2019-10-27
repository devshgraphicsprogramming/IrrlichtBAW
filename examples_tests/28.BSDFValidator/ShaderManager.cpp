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

#include "ShaderManager.h"

#include "SMaterial.h"
#include "IGPUProgrammingServices.h"

#include <iostream>
#include <string>

namespace irr
{

ShaderManager::ShaderManager(irr::video::IGPUProgrammingServices* services)
    : m_Services(services), m_FragmentShaderSource(FRAGMENT_SHADER_SOURCE)
{}

irr::video::E_MATERIAL_TYPE ShaderManager::GetShader() const
{
    return static_cast<irr::video::E_MATERIAL_TYPE>(m_Services->addHighLevelShaderMaterial(
        VERTEX_SHADER_SOURCE, nullptr, nullptr, nullptr,
        m_FragmentShaderSource.c_str()
    ));
}

void ShaderManager::UpdateShader(const std::string& functionDefinitions)
{
    // Check for shader compile errors before appending
    m_FragmentShaderSource += '\n';
    m_FragmentShaderSource += functionDefinitions;

    /* !!!....TEMPORARY....!!! */
    std::cout << "Updated Shader:" << std::endl;
    std::cout << m_FragmentShaderSource << std::endl;
}

void ShaderManager::ResetShader()
{
    m_FragmentShaderSource = FRAGMENT_SHADER_SOURCE;
}

}   // namespace irr