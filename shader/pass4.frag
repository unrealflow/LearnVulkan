#version 460
#extension GL_GOOGLE_include_directive : enable
#include "Switch.glsl"

layout (set = 0, binding = 0) uniform sampler2D samplerPosition;
layout (set = 0, binding = 1) uniform sampler2D samplerNormal;
layout (set = 0, binding = 2) uniform sampler2D samplerAlbedo;
layout (set = 0, binding = 3) uniform sampler2D pass3;

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

vec3 Saturation(vec3 src,float s)
{
	float luma=dot(src,vec3(0.2126729,0.7151522,0.0721750));
	return luma+s*(src-luma);
}
void main() 
{
	outColor=texture(pass3,inUV);
	outColor.xyz=Saturation(outColor.xyz,1.5);
}
