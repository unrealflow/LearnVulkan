#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
// layout(location = 3) in vec3 color;
// layout(location = 4) in float mat;

layout(set = 0, binding = 0) uniform UBO
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
}
ubo;

layout(location = 0) out vec2 p_uv;
layout(location = 1) out vec4 p_pos;
layout(location = 2) out vec4 p_n;
layout(location = 3) out vec4 fragPos;
layout(location = 4) out vec4 prePos;


void main()
{
    // gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    p_uv = uv;
    p_pos = ubo.model * vec4(position, 1.0);
    p_n = vec4(normal, 0.0);
    fragPos= ubo.jitterProj * ubo.view * p_pos;
    prePos=ubo.preProj * ubo.preView * p_pos;
    gl_Position=fragPos;
    // fragColor = colors[gl_VertexIndex];
}