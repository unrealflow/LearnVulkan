#version 460
#extension GL_GOOGLE_include_directive : enable
#include "Switch.glsl"

layout (set = 0, binding = 0) uniform sampler2D samplerPosition;
layout (set = 0, binding = 1) uniform sampler2D samplerNormal;
layout (set = 0, binding = 2) uniform sampler2D samplerAlbedo;
layout (set = 0, binding = 3) uniform sampler2D pass2;

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;
const float wx[3]={1.0,2.0,1.0};
const float wy[3]={1.0,2.0,1.0};
//sobel边缘检测算子
const float kernel_sobel_y[3][3] = {
    {-1, -2, -1},
    {0, 0, 0},
    {1, 2, 1}};

const float kernel_sobel_x[3][3] = {
    {-1, 0, 1},
    {-2, 0, 2},
    {-1, 0, 1}};

float th_filter(sampler2D tex, vec2 uv,vec2 w)
{
	if(abs(uv.x-0.5)>(0.5-w.x)||abs(uv.y-0.5)>(0.5-w.y)){
		return 0.0;
	}
	vec3 res0=vec3(0.0);
	vec3 res1=vec3(0.0);
	for(int u=-1;u<=1;u++)
	{
		for(int v=-1;v<=1;v++)
		{
			vec3 data=texture(tex,uv+vec2(u,v)*w).xyz;
			res0+=data*kernel_sobel_x[u+1][v+1];
			res1+=data*kernel_sobel_y[u+1][v+1];
		}
	}
	float res=sqrt(dot(res0,res0)+dot(res1,res1));
	return res;
}

void main() 
{
	outColor=texture(pass2,inUV);
	
#ifdef USE_FXAA
	vec2 tex_offset = textureSize(pass2, 0);
	vec2 w=1.0/tex_offset;
	float NW=texture(pass2,inUV+vec2(-1.0,1.0)*w).a;
	float NE=texture(pass2,inUV+vec2(1.0,1.0)*w).a;
	float SW=texture(pass2,inUV+vec2(-1.0,-1.0)*w).a;
	float SE=texture(pass2,inUV+vec2(1.0,-1.0)*w).a;

	float MaxLuma = max(max(NW, NE),max( SW, SE));
	float MinLuma = min(min(NW, NE),min( SW, SE));
	float Contrast = max(MaxLuma,outColor.a) - min(MinLuma, outColor.a);
	// if(Contrast < max(0.05, MaxLuma * 0.125))
	if(Contrast>=0.01&&Contrast <= max(0.05, MaxLuma * 0.25))
	{

		vec2 Dir=vec2(0.0);
		Dir.x = (SW + SE) - (NW + NE);
		Dir.y = (NW + SW) - (NE + SE);
		Dir.xy = normalize(Dir.xy);

		vec4 P0 = texture(pass2,inUV+ Dir * 0.5*w);
		vec4 P1 = texture(pass2,inUV- Dir * 0.5*w);

		float MinDir = min(abs(Dir.x), abs(Dir.y)) * 8.0;
		vec2 NewDir = clamp(Dir / MinDir, vec2(-2.0), vec2(2.0));
		vec4 Q0 = texture(pass2,inUV + NewDir * 2.0*w);
		vec4 Q1 = texture(pass2,inUV - NewDir * 2.0*w);
		vec4 R0 = (P0 + P1 + Q0 + Q1) * 0.25;
		vec4 R1 = (P0 + P1) * 0.5;
		if(R0.a < MinLuma || R0.a > MaxLuma)
		{
			outColor=R1;
		}
		else
		{
			outColor=R0;
		}
	}
#endif

	float exposure=1.5;
	outColor=1.0-exp(-outColor*outColor*exposure);
    outColor=pow(outColor,vec4(0.45));
}
