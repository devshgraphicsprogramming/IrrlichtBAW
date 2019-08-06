#version 430 core

layout (location = 0) uniform mat4 MVP;
layout (location = 1) uniform mat4 prevMVP;
layout (location = 2) uniform vec3 cameraPos;

layout(location = 0) in vec4 vPos; //only a 3d position is passed from irrlicht, but last (the W) coordinate gets filled with default 1.0
layout(location = 1) in vec4 vCol; //only a 3d position is passed from irrlicht, but last (the W) coordinate gets filled with default 1.0
layout(location = 3) in vec3 vNormal;

out vec4 Color; //per vertex output color, will be interpolated across the triangle
out vec3 Normal;
out vec3 lightDir;
out vec3 currPos_cs;
out vec3 prevPos_cs;

void main()
{
	vec4 currPos = MVP*vPos;
    gl_Position = currPos;
    Color = vec4(1.0);
    Normal = normalize(vNormal); //have to normalize twice because of normal quantization
    lightDir = cameraPos-vPos.xyz;
	prevPos_cs = (prevMVP*vPos).xyw;
	currPos_cs = currPos.xyw;
}
