/***********************************************************************
    created:    Tue Mar 12 2019
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
#ifndef _CEGUISaga3DShaders_h_
#define _CEGUISaga3DShaders_h_

// Start of CEGUI namespace section
namespace CEGUI
{

    const char Saga3DVertexShader[] = R"(
    #version 450
    layout (location = 0) in vec3 position;
    layout (location = 1) in vec2 uv;
    layout (location = 2) in vec4 color;

    layout (location = 0) out vec2 out_uv;
    layout (location = 1) out vec4 out_color;

    layout (binding = 0) uniform Transform {
        mat4 matrix;
    } transform;

    void main() {
        out_uv = uv;
        out_color = color;
        gl_Position = transform.matrix * vec4(position, 1.0);
        gl_Position.y = -gl_Position.y;
    })";

    const char Saga3DFragmentShader[] = R"(
    #version 450
    layout (binding = 1) uniform sampler2D texture0;
    layout (location = 0) in vec2 uv;
    layout (location = 1) in vec4 color;
    layout (location = 0) out vec4 fragColor;

    void main() {
        fragColor = texture(texture0, uv) * color;
    })";

} // End of  CEGUI namespace section

#endif // end of guard _CEGUISaga3DShaders_h_