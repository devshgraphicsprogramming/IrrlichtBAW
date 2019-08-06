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

vec3 RGB_YCoCg(vec3 c)
{
	return vec3(
		 c.x*0.25 + c.y*0.5 + c.z*0.25,
		 c.x*0.5 - c.z*0.5,
		-c.x*0.25 + c.y*0.5 - c.z*0.25
	);
}
vec3 YCoCg_RGB(vec3 c)
{
	return 
	clamp(vec3(
		c.x + c.y - c.z,
		c.x + c.z,
		c.x - c.y - c.z), vec3(0.0), vec3(1.0));
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
vec2 closestFragmentOffsetIn3x3(in vec2 uv)
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
	
	return (closest.xy)/SCREEN_SIZE;
}

//returns sample in coords passed in `uv` param and writes AABB to 2 output params
//all outputs are in YCoCg color model
vec3 get2x2NeighbourhoodColorAABB(in vec2 uv, out vec3 aabb_min, out vec3 aabb_max)
{
	mat3x4 nb;
	nb[0] = textureGather(uCurrentBuf, uv, 0);
	nb[1] = textureGather(uCurrentBuf, uv, 1);
	nb[2] = textureGather(uCurrentBuf, uv, 2);
	
	mat4x3 nb_T = transpose(nb);
	nb_T[0] = RGB_YCoCg(nb_T[0]);
	nb_T[1] = RGB_YCoCg(nb_T[1]);
	nb_T[2] = RGB_YCoCg(nb_T[2]);
	nb_T[3] = RGB_YCoCg(nb_T[3]);
	aabb_min = min(nb_T[0], min(nb_T[1], min(nb_T[2], nb_T[3])));
	aabb_max = max(nb_T[0], max(nb_T[1], max(nb_T[2], nb_T[3])));
	
	return nb_T[3];
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
	vec2 vel = textureLod(uVelocityBuf, TexCoords+closestFragmentOffsetIn3x3(TexCoords), 0.0).xy;
	
	vec2 q_uv = TexCoords - vel;
	
	vec3 history = RGB_YCoCg(textureLod(uHistoryBuf, q_uv, 0.0).rgb);
	vec3 aabb_min;
	vec3 aabb_max;
	vec3 current = get2x2NeighbourhoodColorAABB(TexCoords-uJitter, aabb_min, aabb_max);

	history = constrainHistSample(history, aabb_min, aabb_max);
	
	float lum = current.x;
	float lum_hist = history.x;
	// Note: tweakable!
	float weight = abs(lum-lum_hist)/max(lum, max(lum_hist, 0.2));
	weight = 1.0 - weight;
	weight = weight*weight;
	
	float lerpFactor = mix(0.5, 0.9, weight);
	
	vec3 color = YCoCg_RGB(mix(current, history, lerpFactor));
	
	outHistory = vec4(color, 1.0);
	
	//Note: tweakable!
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