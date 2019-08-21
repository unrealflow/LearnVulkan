#version 450

layout(binding = 0) uniform sampler2D  samplerPosition;
layout(binding = 1) uniform sampler2D  samplerNormal;
// layout(input_attachment_index = 2, binding = 2) uniform subpassInput samplerAlbedo;
layout(binding = 2) uniform sampler2D samplerAlbedo;
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
} curVP;

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;
//difference return (0,++) 
float ev(vec3 a, vec3 b)
{
	return exp(1.0*length(a-b)/length(a+b)) - 1;
}
float ev(vec4 a, vec4 b)
{
	return exp(1.0*length(a-b)/length(a+b)) - 1;
}
const float evSize=10.0;
float compare(in vec3 fragPos,in vec3 normal,in vec2 UV)
{
	
	vec3 preP = texture(prePosition, inUV).rgb;
	vec3 preN = texture(preNormal, inUV).rgb;
	float factor = ev(fragPos, preP) + ev(normal, preN);
	return 1/(exp(factor*evSize));
}
const float fitler=0.002;

vec4 DeAlbedo(vec2 _inUV)
{
	return texture(rtImage, _inUV)/texture(samplerAlbedo,_inUV);
}

void main()
{
	// vec2 preUV=(preVP.proj*preVP.view* curVP.viewInverse*curVP.projInverse* vec4(inUV,0.0,0.0)).xy;
	// Read G-Buffer values from previous sub pass
	vec3 fragPos = texture(samplerPosition,inUV).rgb;
	vec3 normal = texture(samplerNormal,inUV).rgb;
	vec4 albedo = texture(samplerAlbedo,inUV);

	float f0= compare(fragPos,normal,inUV);
	float f1= compare(fragPos,normal,inUV+vec2(0.0,fitler));
	float f2= compare(fragPos,normal,inUV-vec2(0.0,fitler));
	float f3= compare(fragPos,normal,inUV+vec2(fitler,0.0));
	float f4= compare(fragPos,normal,inUV-vec2(fitler,0.0));
	float f5= 0.3*compare(fragPos,normal,inUV+vec2(fitler,fitler));
	float f6= 0.3*compare(fragPos,normal,inUV-vec2(fitler,fitler));
	float f7= 0.3*compare(fragPos,normal,inUV+vec2(fitler,-fitler));
	float f8= 0.3*compare(fragPos,normal,inUV-vec2(fitler,-fitler));
	float total=f0+f1+f2+f3+f4+f5+f6+f7+f8;

	vec4 preFr = texture(preFrame, inUV);

	vec4 rtColor0 = DeAlbedo(inUV);
	vec4 rtColor1= DeAlbedo(inUV+vec2(0.0,fitler));
	vec4 rtColor2= DeAlbedo(inUV-vec2(0.0,fitler));
	vec4 rtColor3= DeAlbedo(inUV+vec2(fitler,0.0));
	vec4 rtColor4= DeAlbedo(inUV-vec2(fitler,0.0));
	vec4 rtColor5= DeAlbedo(inUV+vec2(fitler,fitler));
	vec4 rtColor6= DeAlbedo(inUV-vec2(fitler,fitler));
	vec4 rtColor7= DeAlbedo(inUV+vec2(fitler,-fitler));
	vec4 rtColor8= DeAlbedo(inUV-vec2(fitler,-fitler));

	float factor=max(f0-0.1,0);
	vec4 rtColor = (rtColor0*f0+rtColor1*f1+rtColor2*f2+rtColor3*f3+rtColor4*f4
					+rtColor5*f5+rtColor6*f6+rtColor7*f7+rtColor8*f8)/total;
	vec4 curColor = rtColor*albedo;

	// outColor=rtColor0;
	outColor = mix(curColor, preFr, factor);
	outColor=clamp(outColor,preFr-0.1,preFr+0.05);
	// outColor=albedo;
	// outColor=texture(rtImage,inUV);
	// outColor=vec4(fragPos,1.0);
}