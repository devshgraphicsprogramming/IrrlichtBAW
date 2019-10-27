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

#ifndef _IRR_SHADER_MANAGER_INCLUDED_
#define _IRR_SHADER_MANAGER_INCLUDED_

#include "irrlicht.h"

#include <string>

class ShaderManager : public irr::core::IReferenceCounted
{
public:
    ShaderManager(irr::video::IGPUProgrammingServices* services);

    irr::video::E_MATERIAL_TYPE GetShader() const;
    void UpdateShader(const std::string& functionDefinitions);
    void ResetShader();

private:
    // void CheckCompilationErrors();

    irr::video::IGPUProgrammingServices* m_Services;
    std::string m_FragmentShaderSource;

    static constexpr const char* VERTEX_SHADER_SOURCE =
        R"(#version 430 core

layout (location = 0) in vec3 vPosition;

void main()
{    
    gl_Position = vec4(vPosition, 1.0);
}
    )";
    static constexpr const char* FRAGMENT_SHADER_SOURCE =
        R"(#version 430 core

layout (location = 0) out vec4 pixelColor;

vec3 bsdf_cos_eval();
vec3 bsdf_cos_sample();
vec3 bsdf_cos_sample_probability();

void main()
{
    pixelColor = vec4(1.0, 1.0, 0.0, 1.0);
}
    )";
};

#endif  // _IRR_SHADER_MANAGER_INCLUDED_
