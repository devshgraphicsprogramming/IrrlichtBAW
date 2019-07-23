#version 430 core

layout (location = 0) in vec3 vPos;

layout (location = 0) uniform mat4 MVP;
layout (location = 1) uniform mat4 prevMVP;

out vec2 vVelocity;

void main()
{
	vec4 p_cs = MVP*vec4(vPos, 1.0);
	gl_Position = p_cs;
	
	// convert to ndc
	p_cs /= p_cs.w;
	vec4 q_cs = prevMVP*vec4(vPos, 1.0);
	q_cs /= q_cs.w;
	
	// convert to screen space and calculate velocity
	vVelocity = 0.5*(p_cs.xy - q_cs.xy); // 0.5*(p-q) is same as (0.5*p + 0.5) - (0.5*q + 0.5)
}