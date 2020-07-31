#ifndef _COMMON_GLSL_
#define _COMMON_GLSL_

#define LOC_UNIFORM 10
#define LOC_DIFFUSE 20
#define LOC_STRIDE 5
#define LOC_VERTEX 12
#define LOC_INDEX 13
#define LOC_LIGHT 50

//Size: vec4 * 5
struct Mat {
    vec3 baseColor;
    float metallic;

    float subsurface;
    float specular;
    float roughness;
    float specularTint;

    float anisotropic;
    float sheen;
    float sheenTint;
    float clearcoat;
    
    float clearcoatGloss;
    float emission;
    float IOR;
    float transmission;

    float index;
    float useTex;
    float _PAD1_;
    float _PAD2_;
};

layout(set = 0, binding = LOC_LIGHT)  buffer Lights { vec4 l[]; }
lights;

struct Light {
    float type;
    vec3 pos;
    vec3 dir;
    vec3 color;
    float radius;
    float atten;
};
const float D_x[20] = { 0.000000f, 0.500000f, 0.250000f, 0.750000f, 0.125000f, 0.625000f, 0.375000f, 0.875000f, 0.062500f, 0.562500f, 0.312500f, 0.812500f, 0.187500f, 0.687500f, 0.437500f, 0.937500f, 0.031250f, 0.531250f, 0.281250f, 0.781250f };
const float D_y[20] = { 0.000000f, 0.333333f, 0.666667f, 0.111111f, 0.444444f, 0.777778f, 0.222222f, 0.555556f, 0.888889f, 0.037037f, 0.370370f, 0.703704f, 0.148148f, 0.481482f, 0.814815f, 0.259259f, 0.592593f, 0.925926f, 0.074074f, 0.407407f };
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

#endif