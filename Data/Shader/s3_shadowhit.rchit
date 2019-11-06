#version 460
#extension GL_GOOGLE_include_directive    : enable
#extension GL_NV_ray_tracing : require
#include "RayCommon.glsl"
layout(location = 2) rayPayloadInNV float shadowed;



void main()
{
    uint meshID = GetMeshID(gl_PrimitiveID);
	Mat mat=GetMat(meshID);
	shadowed = mat.transmission;
}