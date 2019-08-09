#version 460
#extension GL_GOOGLE_include_directive    : enable
#extension GL_NV_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#include "Common.glsl"

layout(location = 0) rayPayloadInNV RP hitValue;
layout(location = 2) rayPayloadNV bool shadowed;
hitAttributeNV vec3 attribs;

layout(binding = 0, set = 0) uniform accelerationStructureNV topLevelAS;
layout(binding = 2, set = 0) uniform CameraProperties 
{
	mat4 viewInverse;
	mat4 projInverse;
	vec4 lightPos;
} cam;
layout(binding = 3, set = 0) buffer Vertices { vec4 v[]; } vertices;
layout(binding = 4, set = 0) buffer Indices { uint i[]; } indices;

layout(set = 0, binding = 101) uniform sampler2D tex;

struct Vertex
{
  vec3 pos;
  vec3 normal;
  vec3 color;
  vec2 uv;
  float _pad0;
};

Vertex unpack(uint index)
{
	vec4 d0 = vertices.v[2 * index + 0];
	vec4 d1 = vertices.v[2 * index + 1];

	Vertex v;
	v.pos = d0.xyz;
	v.normal = vec3(d0.w, d1.x, d1.y);
	v.uv = vec2(d1.z, d1.w);
	v.color=vec3(1.0,1.0,1.0);
	return v;
}
float lo(float src)
{
	float t=1.0;
	return t/(t+src);
}
vec3 lo(vec3 src)
{
	return vec3(lo(src.x),lo(src.y),lo(src.z));
}

float noise(float a)
{
    float k=fract(sin(131.33*a+23.123)*13.1);
    return k;
}

vec3 noise(vec2 uv)
{
    float t1=noise(uv.x);
    float t2=noise(uv.y);
    float t3=noise(t1+t2);
    return normalize(2.0*vec3(noise(t1*uv.x+t2*uv.y),noise(t3+uv.x),noise(t3+uv.y))-1.0);
}
void main()
{
	ivec3 index = ivec3(indices.i[3 * gl_PrimitiveID], indices.i[3 * gl_PrimitiveID + 1], indices.i[3 * gl_PrimitiveID + 2]);

	Vertex v0 = unpack(index.x);
	Vertex v1 = unpack(index.y);
	Vertex v2 = unpack(index.z);

	// Interpolate normal
	const vec3 barycentricCoords = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);
	vec3 normal = normalize(v0.normal * barycentricCoords.x + v1.normal * barycentricCoords.y + v2.normal * barycentricCoords.z);
	// vec2 uv = v0.uv * barycentricCoords.x + v1.uv * barycentricCoords.y + v2.uv * barycentricCoords.z;

	// Basic lighting
	vec3 lightVector = normalize(cam.lightPos.xyz);
	hitValue.color =BRDF(mat.baseColor,lightVector,-gl_WorldRayDirectionNV,normal,vec3(0.6,0.8,0.0),vec3(0.0,0.6,0.8));
	hitValue.color=max(vec3(0.01),hitValue.color);
	// Shadow casting
	float tmin = 0.001;
	float tmax = 100.0;
	vec3 origin = gl_WorldRayOriginNV + gl_WorldRayDirectionNV * gl_HitTNV;
	shadowed = true;  
	// Offset indices to match shadow hit/miss index
	traceNV(topLevelAS, gl_RayFlagsTerminateOnFirstHitNV | gl_RayFlagsOpaqueNV|gl_RayFlagsSkipClosestHitShaderNV, 0xFF, 1, 0, 1, origin, tmin, lightVector, tmax, 2);
	if (shadowed) {
		hitValue.color *= 0.3;
	}
	hitValue.position=origin;
	hitValue.direction=normalize(reflect(gl_WorldRayDirectionNV,normal)+mat.roughness*noise(cam.lightPos.xy+origin.xy));
	float d=dot(hitValue.direction,normal);
	if(d<0)
	{
		hitValue.direction=-hitValue.direction;
	}
}
