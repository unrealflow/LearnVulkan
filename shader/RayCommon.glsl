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

layout(set = 0, binding = 4) buffer IndexCount { uint i[MAX_MESH]; }
indexCount;


layout(set = 0, binding = LOC_INDEX) buffer TotalIndice { uint i[]; }
t_indices;

layout(set = 0, binding = LOC_VERTEX) buffer TotalVertice { vec4 v[]; }
t_vertices;

layout(set = 0, binding = LOC_UNIFORM) buffer TotalMats { Mat m[]; }
t_mats;
layout(set = 0, binding = LOC_DIFFUSE ) uniform sampler2D  t_tex[MAX_MESH];

struct Vertex {
    vec3 pos;
    vec3 normal;
    vec2 uv;
    vec3 color;
    float mat;
};

Vertex Unpack(uint index, uint meshID)
{
    vec4 d0 = t_vertices.v[2 * index];
    vec4 d1 = t_vertices.v[2 * index + 1];
    // vec4 d2 = vertices0.v[3 * index + 2];
    Vertex v;
    v.pos = d0.xyz;
    v.normal = vec3(d0.w, d1.xy);
    v.uv = d1.zw;
    // v.color = d2.xyz;
    // v.mat = d2.w;
    return v;
}
ivec3 GetIndices(uint primID)
{
    ivec3 index;
    index = ivec3(t_indices.i[primID * 3], t_indices.i[primID * 3 + 1], t_indices.i[primID * 3 + 2]);
    return index;
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
    // Mat m=t_mats.m[id];
    // if(id==2)
    // {
    //     m.transmission=1.0;
    // }
    // return m;
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