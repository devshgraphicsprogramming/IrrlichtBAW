#version 430 core

in vec4 Color; //per vertex output color, will be interpolated across the triangle
in vec3 Normal;
in vec3 lightDir;
in vec3 currPos_cs;
in vec3 prevPos_cs;

layout(location = 0) out vec4 pixelColor;
layout(location = 1) out vec2 velocity;

void main()
{
    pixelColor = Color*max(dot(normalize(Normal),normalize(lightDir)),0.0);
	vec2 prevPos_ndc = prevPos_cs.xy/prevPos_cs.z; //z is really w component, see vertex shader
	vec2 currPos_ndc = currPos_cs.xy/currPos_cs.z;
	velocity = 0.5*(currPos_ndc - prevPos_ndc);
}
