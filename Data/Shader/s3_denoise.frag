#version 450

layout(binding = 0) uniform sampler2D samplerPosition;
layout(binding = 1) uniform sampler2D samplerNormal;
// layout(input_attachment_index = 2, binding = 2) uniform subpassInput samplerAlbedo;
layout(binding = 2) uniform sampler2D samplerAlbedo;

layout(binding = 3) uniform sampler2D rtImage;
layout(binding = 4) uniform sampler2D prePosition;
layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;

void main()
{
	// Read G-Buffer values from previous sub pass
	vec3 fragPos = texture(samplerPosition,inUV).rgb;
	vec3 normal = texture(samplerNormal,inUV).rgb;
	// vec4 albedo = subpassLoad(samplerAlbedo);
	vec4 albedo=texture(samplerAlbedo,inUV);
	vec4 rtColor = texture(rtImage, inUV);
	vec4 prePos=texture(prePosition,inUV);
	outColor = rtColor * prePos;
	// outColor=vec4(fragPos,1.0);
}