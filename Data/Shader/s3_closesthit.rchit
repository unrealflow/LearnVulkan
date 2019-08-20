#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_NV_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#include "Common.glsl"

layout(location = 0) rayPayloadInNV RP hitValue;
layout(location = 2) rayPayloadNV bool shadowed;
hitAttributeNV vec3 attribs;

layout(binding = 0, set = 0) uniform accelerationStructureNV topLevelAS;

struct Vertex
{
	vec3 pos;
	vec3 normal;
	vec2 uv;
	vec3 color;
	float mat;
};


float lo(float src)
{
	float t = 1.0;
	return t / (t + src);
}
vec3 lo(vec3 src)
{
	return vec3(lo(src.x), lo(src.y), lo(src.z));
}

float noise(float a)
{
	float k = fract(sin(131.33 * a + 23.123) * 13.1);
	return k;
}

vec3 norm_noise(vec2 uv)
{
	float t1 = noise(uv.x);
	float t2 = noise(uv.y);
	float t3 = noise(t1 + t2);
	return 0.577*(2.0 * vec3(noise(t1 * uv.x + t2 * uv.y), noise(t3 + uv.x), noise(t3 + uv.y)) - 1.0);
}
float pw5(float x)
{
	float k=x*x;
	return k*k*x;
}
vec3 noise(vec2 uv)
{
	vec3 t=norm_noise(uv);
	float l=(length(t));
	return l*t;
}
void shader(Mat _mat,sampler2D _tex,Vertex v0,Vertex v1,Vertex v2)
{
	// Interpolate normal
	const vec3 barycentricCoords = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);
	vec3 normal = normalize(v0.normal * barycentricCoords.x + v1.normal * barycentricCoords.y + v2.normal * barycentricCoords.z);
	vec2 uv = v0.uv * barycentricCoords.x + v1.uv * barycentricCoords.y + v2.uv * barycentricCoords.z;
	vec3 origin = gl_WorldRayOriginNV + gl_WorldRayDirectionNV * gl_HitTNV;
	// Basic lighting
	Light light0=GetLight(0);
	vec3 lightVector = normalize(light0.pos+light0.radius*noise(light0.pos.yx + origin.xy)-origin);
	vec3 baseColor=_mat.baseColor;
	if(_mat.useTex>0)
	{
		baseColor*=texture(_tex,uv).xyz;
	}
	hitValue.color = BRDF(_mat, baseColor, lightVector, -gl_WorldRayDirectionNV, normal, vec3(0.6, 0.8, 0.0), vec3(0.0, 0.6, 0.8));
	// Shadow casting
	float tmin = 0.001;
	float tmax = 100.0;
	shadowed = true;
	// Offset indices to match shadow hit/miss index
	traceNV(topLevelAS, gl_RayFlagsTerminateOnFirstHitNV | gl_RayFlagsOpaqueNV | gl_RayFlagsSkipClosestHitShaderNV, 0xFF, 1, 0, 1, origin, tmin, lightVector, tmax, 2);
	if (shadowed)
	{
		hitValue.color *= 0.3;
	}
	hitValue.color +=_mat.emission*baseColor;
	hitValue.position = origin;
	normal = normalize(normal + _mat.roughness * noise(light0.pos.xy + origin.xy +hitValue.bias));
	hitValue.direction = reflect(gl_WorldRayDirectionNV, normal);
}

Vertex Unpack(uint index,uint meshID)
{
	vec4 d0,d1;
	switch(meshID)
	{
		case 0:
			d0 = vertices0.v[2 * index];
			d1 = vertices0.v[2 * index + 1];
			break;
#ifdef USE_MESH_INFO_1
		case 1:
			d0 = vertices1.v[2 * index];
			d1 = vertices1.v[2 * index + 1];
			break;
#endif
#ifdef USE_MESH_INFO_2
		case 2:
			d0 = vertices2.v[2 * index];
			d1 = vertices2.v[2 * index + 1];
			break;
#endif
		default:
			d0 = vertices0.v[0];
			d1 = vertices0.v[1];
			break;
	}
	// vec4 d2 = vertices0.v[3 * index + 2];
	Vertex v;
	v.pos = d0.xyz;
	v.normal = vec3(d0.w, d1.xy);
	v.uv = d1.zw;
	// v.color = d2.xyz;
	// v.mat = d2.w;
	return v;
}
ivec3 GetIndices(uint primID,uint meshID)
{
	ivec3 index;
	switch(meshID)
	{
		case 0:
			index=ivec3(indices0.i[3 * primID], indices0.i[3 * primID + 1], indices0.i[3 * primID + 2]);
			break;
#ifdef USE_MESH_INFO_1
		case 1:
			index=ivec3(indices1.i[3 * primID], indices1.i[3 * primID + 1], indices1.i[3 * primID + 2]);
			break;
#endif
#ifdef USE_MESH_INFO_2
		case 2:
			index=ivec3(indices2.i[3 * primID], indices2.i[3 * primID + 1], indices2.i[3 * primID + 2]);
			break;
#endif
		default:
			index=ivec3(indices0.i[3 * primID], indices0.i[3 * primID + 1], indices0.i[3 * primID + 2]);
			break;
	}
	return index;
}
void main()
{
	// ivec3 index = ivec3(indices0.i[3 * gl_PrimitiveID], indices0.i[3 * gl_PrimitiveID + 1], indices0.i[3 * gl_PrimitiveID + 2]);
	ivec2 tp=GetMeshID(gl_PrimitiveID);
	uint meshID=tp.x;
	uint primID=tp.y;
	// uint primID=gl_PrimitiveID;
	
	ivec3 index = GetIndices(primID,meshID);
	Vertex v0 = Unpack(index.x,meshID);
	Vertex v1 = Unpack(index.y,meshID);
	Vertex v2 = Unpack(index.z,meshID);

	switch(meshID)
	{
		case 0:
			shader(mat_0.m,tex0,v0,v1,v2);
			break;
#ifdef USE_MESH_INFO_1
		case 1:
			shader(mat_1.m,tex1,v0,v1,v2);
			break;
#endif
#ifdef USE_MESH_INFO_2		
		case 2:
			shader(mat_2.m,tex2,v0,v1,v2);
			break;
#endif
		default:
			shader(mat_0.m,tex0,v0,v1,v2);
			break;
	}
}
