#version 460
#extension GL_GOOGLE_include_directive    : enable
#extension GL_NV_ray_tracing : require
#include "RayCommon.glsl"

layout(location = 0) rayPayloadInNV RP hitValue;

void main()
{
    hitValue.color = vec3(-1.0, 0.0, 0.0);
}