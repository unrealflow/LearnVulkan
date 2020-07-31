#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_separate_shader_objects : enable
#include "Common.glsl"

layout(early_fragment_tests) in;
layout(location = 0) in vec2 p_uv;
layout(location = 1) in vec4 p_pos;
layout(location = 2) in vec4 p_n;
layout(location = 3) in vec4 fragPos;
layout(location = 4) in vec4 prePos;



// layout(set = 0, binding = 1) uniform sampler2D rtImage;

layout(set = 0, binding = LOC_UNIFORM) uniform Material { Mat m; }mat;

layout(set = 0, binding = LOC_DIFFUSE) uniform sampler2D tex;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outAlbedo;

void main()
{
    if (mat.m.useTex > 0) {
        outAlbedo = vec4(texture(tex, p_uv).xyz, mat.m.index);
    } else {
        outAlbedo = vec4(mat.m.baseColor, mat.m.index);
    }
    // vec2 inUV=(fragPos/fragPos.w).xy*0.5+0.5;
    outPosition = p_pos;
    outPosition.w=fragPos.z/fragPos.w;
    vec4 curPosition = fragPos/fragPos.w;
    vec4 prePosition = prePos/prePos.w;
    float motion=uintBitsToFloat(packHalf2x16((prePosition.xy-curPosition.xy)*0.5));
    outNormal = vec4(p_n.xyz,motion);
    // outAlbedo=vec4((prePosition.xy-curPosition.xy)*0.5,0.0,1.0);
}