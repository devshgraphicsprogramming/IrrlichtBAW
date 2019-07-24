#version 430 core

//layout (location = 0) uniform mat4 uPrevVP;
//layout (location = 1) uniform mat4 uInvVP;
layout (location = 0) uniform vec2 uJitter;

//layout (binding = 0) uniform sampler2D uDepthBuf;
layout (binding = 1) uniform sampler2D uCurrentBuf;
layout (binding = 2) uniform sampler2D uHistoryBuf;
layout (binding = 3) uniform sampler2D uVelocityBuf;

in vec2 TexCoords;

layout (location = 0) out vec4 outScreen;  //what will be visible on screen
layout (location = 1) out vec4 outHistory; //history for next frame

float luminanceFromRGB(in vec3 color)
{
	return dot(color, vec3(0.299, 0.587, 0.114));
}

float chebyshevNorm(in vec3 v)
{
	vec3 v_ = abs(v);
	return max(v_.x, max(v_.y, v_.z));
}

// clips history sample against neighbourhood color space
vec3 constrainHistSample(in vec3 hist, in vec3 aabb_min, in vec3 aabb_max)
{
	if (aabb_min==aabb_max)
		return aabb_min; //return immediately if all neighbourhood samples were same color
	// scale needed to transform unit sphere into ellipsoid insribed into the aabb
	vec3 scale = 0.5*(aabb_max-aabb_min);
	vec3 center = 0.5*(aabb_min+aabb_max);
	
	// translate into ellipsoid's space and scale hist sample by inverse scale
	vec3 hist_ = (hist - center)/scale;
	float ray_len = chebyshevNorm(hist_);
	if (ray_len > 1.0)
	{
		// calculate intersection of unit sphere with ray center->hist_ (i.e. normalize)
		vec3 inter1 = hist_/ray_len;
		// scale back to get intersection with ellipsoid and translate to original space
		return inter1*scale + center;
	}
	else
		return hist; // hist sample is within neighbourhood color space
}

void main()
{
/*
	float depth = texture(uDepthBuf, TexCoords).x; // linearize depth?
	
	vec4 clip = vec4(2.0*vec3(TexCoords, depth) - 1.0, 1.0);
	vec4 world = invVP*clip;
	world = world/world.w;
	
	vec4 q_cs = uPrevVP*world;
	vec2 q_uv = 0.5*(q_cs.xy/q_cs.w) + 0.5;
	vec2 vel = TexCoords - q_uv;
*/
	// it should sample nearest (depth) in 3x3 neighbourhood
	vec2 vel = textureLod(uVelocityBuf, TexCoords, 0.0).xy;
	
	vec2 q_uv = TexCoords - vel;
	
	vec3 current = textureLod(uCurrentBuf, TexCoords, 0.0).rgb;
	vec3 history = textureLod(uHistoryBuf, q_uv, 0.0).rgb;
	vec3 neigh0 = textureLodOffset(uCurrentBuf, TexCoords, 0.0, ivec2(1, 0)).rgb;
	vec3 neigh1 = textureLodOffset(uCurrentBuf, TexCoords, 0.0, ivec2(0, 1)).rgb;
	vec3 neigh2 = textureLodOffset(uCurrentBuf, TexCoords, 0.0, ivec2(-1, 0)).rgb;
	vec3 neigh3 = textureLodOffset(uCurrentBuf, TexCoords, 0.0, ivec2(0, -1)).rgb;

	vec3 aabb_min = min(current, min(neigh0, min(neigh1, min(neigh2, neigh3))));
	vec3 aabb_max = max(current, max(neigh0, max(neigh1, max(neigh2, neigh3))));

	history = constrainHistSample(history, aabb_min, aabb_max);
	
	vec3 current_unjittered = textureLod(uCurrentBuf, TexCoords-uJitter, 0.0).rgb;
	
	// TODO use luma to vary interpolation factor as playdeadgames does
	vec3 color = mix(current_unjittered, history, 0.7);
	
	outHistory = vec4(color, 1.0);
	
	// TODO fallback to motion blur (for fast moving fragments) and this will actually go to screen
	outScreen = vec4(color, 1.0);
}