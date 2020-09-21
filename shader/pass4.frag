#version 460
#extension GL_GOOGLE_include_directive : enable
#include "Switch.glsl"

layout (set = 0, binding = 0) uniform sampler2D samplerPosition;
layout (set = 0, binding = 1) uniform sampler2D samplerNormal;
layout (set = 0, binding = 2) uniform sampler2D samplerAlbedo;
layout (set = 0, binding = 3) uniform sampler2D pass2;

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

void main() 
{
	outColor=texture(pass2,inUV);
}
