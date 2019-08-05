#version 450

layout (input_attachment_index = 0, binding = 0) uniform subpassInput samplerposition;
layout (input_attachment_index = 1, binding = 1) uniform subpassInput samplerNormal;
layout (input_attachment_index = 2, binding = 2) uniform subpassInput samplerAlbedo;

layout(binding =3) uniform sampler2D rtImage;
layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

void main() 
{
	// Read G-Buffer values from previous sub pass
	vec3 fragPos = subpassLoad(samplerposition).rgb;
	vec3 normal = subpassLoad(samplerNormal).rgb;
	vec4 albedo = subpassLoad(samplerAlbedo);
	
	vec3 mix1=mix(fragPos,normal,0.5);
	vec4 mix2 =mix(albedo,texture(rtImage,inUV),0.67);
	outColor=mix(vec4(mix1,1.0),mix2,0.6);
}