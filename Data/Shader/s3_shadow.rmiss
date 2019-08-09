#version 460
#extension GL_GOOGLE_include_directive    : enable
#extension GL_NV_ray_tracing : require
#include "Common.glsl"
layout(location = 2) rayPayloadInNV bool shadowed;

void main()
{
	shadowed = false;
}