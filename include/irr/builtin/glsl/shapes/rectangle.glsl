#ifndef _NBL_BUILTIN_GLSL_SHAPES_RECTANGLE_INCLUDED_
#define _NBL_BUILTIN_GLSL_SHAPES_RECTANGLE_INCLUDED_

#include <irr/builtin/glsl/math/functions.glsl>

//
mat4x3 irr_glsl_shapes_getSphericalRectangle(in vec3 offset, in vec3 edge0, in vec3 edge1, in vec3 origin)
{
    const vec3 toRect = offset-origin;
    // the `normalize` cannot be optimized out
    return mat4x3(
        (toRect),
        (toRect+edge0),
        (toRect+edge0+edge1),
        (toRect+edge1)
    );
}

// returns solid angle of a spherical rectangle
// WARNING: can and will return NAN if one or three of the triangle edges are near zero length
// this function is beyond optimized.
float irr_glsl_shapes_SolidAngleOfRectangle(in mat4x3 sphericalVertices)//, out vec4 cos_vertices, out vec4 sin_vertices)//, out float cos_a, out float cos_c, out float csc_b, out float csc_c)
{    
    vec3 edgePlane[4];
    edgePlane[0] = normalize(cross(sphericalVertices[0], sphericalVertices[1]));
    edgePlane[1] = normalize(cross(sphericalVertices[1], sphericalVertices[2]));
    edgePlane[2] = normalize(cross(sphericalVertices[2], sphericalVertices[3]));
    edgePlane[3] = normalize(cross(sphericalVertices[3], sphericalVertices[0]));

    float vertexSphericalAngle[4];
    vertexSphericalAngle[0] = acos(-dot(edgePlane[3],edgePlane[0]));
    vertexSphericalAngle[1] = acos(-dot(edgePlane[0],edgePlane[1]));
    vertexSphericalAngle[2] = acos(-dot(edgePlane[1],edgePlane[2]));
    vertexSphericalAngle[3] = acos(-dot(edgePlane[2],edgePlane[3]));
    return vertexSphericalAngle[0]+vertexSphericalAngle[1]+vertexSphericalAngle[2]+vertexSphericalAngle[3]-2.0*irr_glsl_PI;
}
/*
float irr_glsl_shapes_SolidAngleOfRectangle(in mat3 sphericalVertices)
{
    vec3 dummy0,dummy1;
    float dummy2,dummy3,dummy4,dummy5;
    return irr_glsl_shapes_SolidAngleOfRectangle(sphericalVertices,dummy0,dummy1,dummy2,dummy3,dummy4,dummy5);
}*/
void irr_glsl_AngleSum_addAngle(inout int addPI, inout float cosSum, inout float sinSum, in float cosNextAngle, in float sinNextAngle)
{
    const float newCosSum = cosSum*cosNextAngle-sinSum*sinNextAngle;
    const float newSinSum = cosSum*sinNextAngle+sinSum*cosNextAngle;
    if ((floatBitsToUint(sinSum)^floatBitsToUint(newSinSum))>=0x80000000u)
        addPI++;
    cosSum = newCosSum;
    sinSum = newSinSum;
}
float irr_glsl_AngleSum_resolve(in int addPI, in float cosSum)
{
    const float sum = acos(clamp(cosSum,-1.0,1.0));
    const uint sumAsUint = floatBitsToUint(sum);
    const int oddPI = addPI&0x1;
    return float(addPI+oddPI)*irr_glsl_PI+uintBitsToFloat(sumAsUint^(uint(oddPI)<<31u));
}
// returns solid angle of a rectangle given by its world-space vertices and world-space viewing position
float irr_glsl_shapes_SolidAngleOfRectangle(in vec3 offset, in vec3 edge0, in vec3 edge1, in vec3 origin)
{
    //return irr_glsl_shapes_SolidAngleOfRectangle(irr_glsl_shapes_getSphericalRectangle(offset,edge0,edge1,origin));
    float angleSum = 2.0*irr_glsl_PI;

    const vec3 toRect = offset-origin;
    if (length(toRect)>FLT_MIN)
    {
        const vec3 Z = cross(edge0, edge1);

        vec3 edgePlane[4];
        edgePlane[0] = cross(edge0,toRect);
        edgePlane[3] = cross(toRect,edge1);
        edgePlane[1] = -edgePlane[3]-Z;
        edgePlane[2] = -edgePlane[0]-Z;
        for (int i=0; i<4; i++)
            edgePlane[i] = normalize(edgePlane[i]);

        float sphericalAngleDot[4];
        sphericalAngleDot[0] = -dot(edgePlane[3],edgePlane[0]);
        sphericalAngleDot[1] = -dot(edgePlane[0],edgePlane[1]);
        sphericalAngleDot[2] = -dot(edgePlane[1],edgePlane[2]);
        sphericalAngleDot[3] = -dot(edgePlane[2],edgePlane[3]);

        int addPI = -2;
        float cosSum = sphericalAngleDot[0];
        float sinSum = sqrt(1.0-sphericalAngleDot[0]*sphericalAngleDot[0]);
        for (int i=1; i<4; i++)
            irr_glsl_AngleSum_addAngle(addPI,cosSum,sinSum,sphericalAngleDot[i],sqrt(1.0-sphericalAngleDot[i]*sphericalAngleDot[i]));
        angleSum = irr_glsl_AngleSum_resolve(addPI,cosSum);
    }
    return angleSum;
}

#endif
