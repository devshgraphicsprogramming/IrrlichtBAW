#ifndef _NBL_BUILTIN_GLSL_SAMPLING_SPHERICAL_RECTANGLE_INCLUDED_
#define _NBL_BUILTIN_GLSL_SAMPLING_SPHERICAL_RECTANGLE_INCLUDED_

#include <irr/builtin/glsl/math/quaternions.glsl>

#include <irr/builtin/glsl/shapes/rectangle.glsl>

// WARNING: can and will return NAN if ???
// this function could use some more optimizing
vec3 irr_glsl_sampling_generateSphericalRectangleSample(out float rcpPdf, in mat4x3 sphericalVertices, in vec2 u)
{
    rcpPdf = irr_glsl_shapes_SolidAngleOfRectangle(sphericalVertices);
    return sphericalVertices[0];
    /*
    // this part literally cannot be optimized further
    float negSinSubSolidAngle,negCosSubSolidAngle;
    irr_glsl_sincos(rcpPdf*u.x-irr_glsl_PI,negSinSubSolidAngle,negCosSubSolidAngle);

	const float p = negCosSubSolidAngle*sin_vertices[0]-negSinSubSolidAngle*cos_vertices[0];
	const float q = -negSinSubSolidAngle*sin_vertices[0]-negCosSubSolidAngle*cos_vertices[0];
    
    // TODO: we could optimize everything up and including to the first slerp, because precision here is just godawful
	float u_ = q - cos_vertices[0];
	float v_ = p + sin_vertices[0]*cos_c;

	const float cosAngleAlongAC = clamp(((v_*q - u_*p)*cos_vertices[0] - v_) / ((v_*p + u_*q)*sin_vertices[0]), -1.0, 1.0); // TODO: get rid of this clamp (by improving the precision here)

    // the slerps could probably be optimized by sidestepping `normalize` calls and accumulating scaling factors
	vec3 C_s = irr_glsl_slerp_impl_impl(sphericalVertices[0], sphericalVertices[2]*csc_b, cosAngleAlongAC);

    const float cosBC_s = dot(C_s,sphericalVertices[1]);
	const float cosAngleAlongBC_s = 1.0+cosBC_s*u.y-u.y;

	return irr_glsl_slerp_impl_impl(sphericalVertices[1], C_s*inversesqrt(1.0-cosBC_s*cosBC_s), cosAngleAlongBC_s);
    */
}
vec3 irr_glsl_sampling_generateSphericalRectangleSample(out float rcpPdf, in vec3 offset, in vec3 edge0, in vec3 edge1, in vec3 origin, in vec2 u)
{
    //return irr_glsl_sampling_generateSphericalRectangleSample(rcpPdf,irr_glsl_shapes_getSphericalRectangle(offset,edge0,edge1,origin),u);
    rcpPdf = irr_glsl_shapes_SolidAngleOfRectangle(offset,edge0,edge1,origin);
    return normalize(offset-origin);
}

#endif
