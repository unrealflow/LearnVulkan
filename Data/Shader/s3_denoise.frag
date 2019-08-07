#version 450

layout(input_attachment_index = 0, binding = 0) uniform subpassInput samplerPosition;
layout(input_attachment_index = 1, binding = 1) uniform subpassInput samplerNormal;
layout(input_attachment_index = 2, binding = 2) uniform subpassInput samplerAlbedo;
// layout(binding = 2) uniform sampler2D samplerAlbedo;
layout(binding = 3) uniform sampler2D rtImage;

layout(binding = 4) uniform sampler2D preFrame;
layout(binding = 5) uniform sampler2D prePosition;
layout(binding = 6) uniform sampler2D preNormal;
layout(binding = 7) uniform sampler2D preAlbedo;
layout(binding = 8) uniform VP
{
	mat4 view;
	mat4 proj;
}preVP;
layout(binding = 9) uniform CameraProperties 
{
	mat4 viewInverse;
	mat4 projInverse;
	vec4 lightPos;
} curVP;

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;

float ev(vec3 a, vec3 b)
{
	return exp(1.0*length(a-b)/length(a+b)) - 1;
}
float ev(vec4 a, vec4 b)
{
	return exp(1.0*length(a-b)/length(a+b)) - 1;
}
void main()
{
	// vec2 preUV=(preVP.proj*preVP.view* curVP.viewInverse*curVP.projInverse* vec4(inUV,0.0,0.0)).xy;
	// Read G-Buffer values from previous sub pass
	vec3 fragPos = subpassLoad(samplerPosition).rgb;
	vec3 normal = subpassLoad(samplerNormal).rgb;
	vec4 albedo = subpassLoad(samplerAlbedo);

	vec3 preP = texture(prePosition, inUV).rgb;
	vec3 preN = texture(preNormal, inUV).rgb;
	vec4 preA = texture(preAlbedo, inUV);

	float factor = ev(fragPos, preP) + ev(normal, preN) + ev(albedo, preA);
	factor=exp(factor)-1;
	vec4 rtColor = texture(rtImage, inUV);
	vec4 preFr = texture(preFrame, inUV);

	outColor = rtColor * albedo;
	outColor = mix(preFr, outColor, (factor + 0.05) / (1 + factor));
	// outColor=vec4(fragPos,1.0);
}