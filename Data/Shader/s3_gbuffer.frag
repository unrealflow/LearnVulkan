#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_separate_shader_objects : enable
#include "Common.glsl"

layout(location = 0) in vec2 p_uv;
layout(location = 1) in vec4 p_pos;
layout(location = 2) in vec4 p_n;

layout(set = 0, binding = LOC_UNIFORM) uniform Material { Mat m; }mat;

layout(set = 0, binding = LOC_DIFFUSE) uniform sampler2D tex;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outPosition;
layout(location = 2) out vec4 outNormal;
layout(location = 3) out vec4 outAlbedo;

void main()
{
    if (mat.m.useTex > 0) {
        outColor = vec4(texture(tex, p_uv).xyz, 1.0);
    } else {
        outColor = vec4(mat.m.baseColor, 1.0);
    }

    outPosition = p_pos;
    outNormal = p_n;
    outAlbedo = outColor;
}