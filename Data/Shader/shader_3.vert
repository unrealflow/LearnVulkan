#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec2 p_uv;



void main() {
    // gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    p_uv=uv;
    gl_Position=vec4(position,1.0);
    // fragColor = colors[gl_VertexIndex];
    
}