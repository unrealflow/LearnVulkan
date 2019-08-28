#ifndef _COMMON_GLSL_
#define _COMMON_GLSL_

#define LOC_UNIFORM 10
#define LOC_DIFFUSE 11
#define LOC_STRIDE 5
#define LOC_VERTEX 12
#define LOC_INDEX 13
#define LOC_LIGHT 50
#define MAX_MESH 6
#define USE_MESH_INFO_1
#define USE_MESH_INFO_2
//Size: vec4 * 4
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
    float useTex;
};
struct Light {
    float type;
    vec3 pos;
    vec3 dir;
    vec3 color;
    float radius;
    float atten;
};

#endif