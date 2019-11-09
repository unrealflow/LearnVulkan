#version 450
const float PI = 3.14159265358979323846;

layout(binding = 0) uniform sampler2D position;
layout(binding = 1) uniform sampler2D normal;
layout(binding = 2) uniform sampler2D albedo;
layout(binding = 3) uniform sampler2D post;
layout(binding = 4) uniform sampler2D preFrame;


layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;


void main()
{
    vec3 baseColor=texture(post,inUV).xyz;

    float gray = baseColor.x * 0.299 + baseColor.y * 0.587 + baseColor.z * 0.114;
    outColor=0.5*smoothstep(0.3,1.0,gray)*vec4(baseColor,1.0);
}