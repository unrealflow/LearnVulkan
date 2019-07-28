#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 p_uv;

layout(set=0,binding = 1) uniform sampler2D tex;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = vec4(texture(tex,p_uv).xyz,1.0);
}