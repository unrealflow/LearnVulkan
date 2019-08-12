#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 color;
layout(location = 4) in float mat;


layout(set=0,binding=0) uniform UBO
    { 
        mat4 model;
        mat4 view;
        mat4 projection;
    } uboVS;

layout(location = 0) out vec2 p_uv;
layout(location =1) out vec4 p_pos;
layout(location =2) out vec4 p_n;


void main() {
    // gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    p_uv=uv;
    p_pos=uboVS.model*vec4(position,1.0);
    p_n=vec4(normal,0.0);
    gl_Position=uboVS.projection*uboVS.view*p_pos;
    // fragColor = colors[gl_VertexIndex];
    
}