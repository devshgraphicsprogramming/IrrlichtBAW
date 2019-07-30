#version 430 core

//layout (location = 0) uniform mat4 uPrevVP;
//layout (location = 1) uniform mat4 uInvVP;
layout (location = 0) uniform vec2 uJitter;

layout (binding = 0) uniform sampler2D uDepthBuf;
layout (binding = 1) uniform sampler2D uCurrentBuf;
layout (binding = 2) uniform sampler2D uHistoryBuf;
layout (binding = 3) uniform sampler2D uVelocityBuf;

in vec2 TexCoords;

layout (location = 0) out vec4 outScreen;  //what will be visible on screen
layout (location = 1) out vec4 outHistory; //history for next frame

#define SCREEN_SIZE vec2(1280.0, 720.0)

float luminanceFromRGB(in vec3 color)
{
	return dot(color, vec3(0.299, 0.587, 0.114));
}

float chebyshevNorm(in vec3 v)
{
	vec3 v_ = abs(v);
	return max(v_.x, max(v_.y, v_.z));
}

#define invlerp(T, a, b, v) clamp((v - a)/(b - a), T(0.0), T(1.0))

// clips history sample against neighbourhood color space
vec3 constrainHistSample(in vec3 hist, in vec3 aabb_min, in vec3 aabb_max)
{
	//if (any(aabb_min==aabb_max)) // commented-out because for some reason it's not enough in some cases, that's why there's max(scale,) in division
	//	return aabb_min; //return immediately if all neighbourhood samples were same color
	
	// scale needed to transform unit sphere into ellipsoid insribed into the aabb
	vec3 scale = 0.5*(aabb_max-aabb_min);
	vec3 center = 0.5*(aabb_min+aabb_max);
	
	// translate into ellipsoid's space and scale hist sample by inverse scale
	vec3 hist_ = (hist - center)/max(scale, vec3(0.0001));
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

//finds texel offset (relative to `uv`) of depth-wise closest fragment in 3x3 neighbourhood
ivec2 closestFragmentOffsetIn3x3(in vec2 uv)
{
	vec3 d0 = vec3(-1.0, 1.0, textureLodOffset(uDepthBuf, uv, 0.0, ivec2(-1, 1)).x);
	vec3 d1 = vec3(0.0, 1.0, textureLodOffset(uDepthBuf, uv, 0.0, ivec2(0, 1)).x);
	vec3 d2 = vec3(1.0, 1.0, textureLodOffset(uDepthBuf, uv, 0.0, ivec2(1, 1)).x);
	vec3 d3 = vec3(-1.0, 0.0, textureLodOffset(uDepthBuf, uv, 0.0, ivec2(-1, 0)).x);
	vec3 d4 = vec3(0.0, 0.0, textureLod(uDepthBuf, uv, 0.0).x);
	vec3 d5 = vec3(1.0, 0.0, textureLodOffset(uDepthBuf, uv, 0.0, ivec2(1, 0)).x);
	vec3 d6 = vec3(-1.0, -1.0, textureLodOffset(uDepthBuf, uv, 0.0, ivec2(-1, -1)).x);
	vec3 d7 = vec3(0.0, -1.0, textureLodOffset(uDepthBuf, uv, 0.0, ivec2(0, -1)).x);
	vec3 d8 = vec3(1.0, -1.0, textureLodOffset(uDepthBuf, uv, 0.0, ivec2(1, -1)).x);
	
	//less = further from screen
#define _CLOSEST(a, b) mix(a, b, a.z>b.z)
	vec3 closest = d0;
	closest = _CLOSEST(closest, d1);
	closest = _CLOSEST(closest, d2);
	closest = _CLOSEST(closest, d3);
	closest = _CLOSEST(closest, d4);
	closest = _CLOSEST(closest, d5);
	closest = _CLOSEST(closest, d6);
	closest = _CLOSEST(closest, d7);
	closest = _CLOSEST(closest, d8);
#undef _CLOSEST
	
	return ivec2(closest.xy);
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
	// TODO test later whether closestFragmentOffsetIn3x3() is worth it
	vec2 vel = textureLodOffset(uVelocityBuf, TexCoords, 0.0, closestFragmentOffsetIn3x3(TexCoords)).xy;
	
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
	
	float lum = luminanceFromRGB(current_unjittered);
	float lum_hist = luminanceFromRGB(history);
	// Note: tweakable!
	float weight = abs(lum-lum_hist)/max(lum, max(lum_hist, 0.2));
	weight = 1.0 - weight;
	weight = weight*weight;
	
	float lerpFactor = mix(0.5, 0.9, weight);
	
	vec3 color = mix(current_unjittered, history, lerpFactor);
	
	outHistory = vec4(color, 1.0);
	
#define VEL_NO_BLUR 20.0
#define VEL_ONLY_BLUR 75.0
	lerpFactor = invlerp(float, VEL_ONLY_BLUR, VEL_NO_BLUR, length(SCREEN_SIZE*vel));
	if (lerpFactor != 1.0)
	{
		vec3 blurColor = current;
		vec2 tap = vel*0.333;
		//for (int i = -3; i > 0; ++i)
		//	blurColor += textureLod(uCurrentBuf, TexCoords + i*tap, 0.0).rgb;
		blurColor += textureLod(uCurrentBuf, TexCoords + -3.0*tap, 0.0).rgb;
		blurColor += textureLod(uCurrentBuf, TexCoords + -2.0*tap, 0.0).rgb;
		blurColor += textureLod(uCurrentBuf, TexCoords + -1.0*tap, 0.0).rgb;
		//for (int i = 1; i < 4; ++i)
		//	blurColor += textureLod(uCurrentBuf, TexCoords + i*tap, 0.0).rgb;
		blurColor += textureLod(uCurrentBuf, TexCoords + 1.0*tap, 0.0).rgb;
		blurColor += textureLod(uCurrentBuf, TexCoords + 2.0*tap, 0.0).rgb;
		blurColor += textureLod(uCurrentBuf, TexCoords + 3.0*tap, 0.0).rgb;
		
		blurColor *= 0.1428; // blurColor /= 7.0;
		
		color = mix(blurColor, color, lerpFactor);
	}
	
	outScreen = vec4(color, 1.0);
}