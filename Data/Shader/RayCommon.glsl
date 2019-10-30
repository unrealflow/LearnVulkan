#ifndef _RAY_COMMON_GLSL_
#define _RAY_COMMON_GLSL_
#include "Common.glsl"
#define MAX_MESH 50

layout(set = 0, binding = 2) uniform UBO
    { 
        mat4 model;
        mat4 view;
        mat4 proj;
        mat4 jitterProj;
        mat4 preView;
        mat4 preProj;
        mat4 viewInverse;
        mat4 projInverse;
        float iTime;
        float delta;
        float upTime;
        uint lightCount;
    } cam;

struct MeshInfo
{
    uint indexCount;
    uint vertexOffset;
};
struct Light {
    float type;
    vec3 pos;
    vec3 dir;
    vec3 color;
    float radius;
    float atten;
};
layout(set = 0, binding = 4) buffer IndexCount { uint i[MAX_MESH]; }
indexCount;

layout(set = 0, binding = LOC_LIGHT) buffer Lights { vec4 l[]; }
lights;

layout(set = 0, binding = LOC_INDEX) buffer TotalIndice { uint i[]; }
t_indices;

layout(set = 0, binding = LOC_VERTEX) buffer TotalVertice { vec4 v[]; }
t_vertices;

layout(set = 0, binding = LOC_UNIFORM) buffer TotalMats { Mat m[]; }
t_mats;
layout(set = 0, binding = LOC_DIFFUSE ) uniform sampler2D  t_tex[MAX_MESH];


Light GetLight(uint index)
{
    vec4 l0 = lights.l[index * 3];
    vec4 l1 = lights.l[index * 3 + 1];
    vec4 l2 = lights.l[index * 3 + 2];
    Light l;
    l.type = l0.x;
    l.pos = l0.yzw;
    l.dir = l1.xyz;
    l.color = vec3(l1.w, l2.x, l2.y);
    l.radius = l2.z;
    l.atten = l2.w;
    return l;
}
uint GetMeshID(uint id)
{
    // float f = float(id);
    
    uint f = id * 3;
    for(uint i=0;i<MAX_MESH;i++)
    {
        if(f<indexCount.i[i]){
            return i;
        }else
        {
            f-=indexCount.i[i];
        }
    }
    return 0;
}

Mat GetMat(uint id)
{
    return t_mats.m[id];
}

struct RP {
    vec3 color;
    vec3 position;
    vec3 direction;
    float bias;
    vec3 kS;
};
#endif