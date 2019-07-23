#version 430 core

in vec2 vVelocity;

layout (location = 0) out vec4 Vel;

void main()
{
	Vel = vec4(vVelocity, 0.0, 1.0);
}