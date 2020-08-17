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

	// vec2 tex_offset = textureSize(pass2, 0);
	// vec2 w=1.0/tex_offset;

	outColor=texture(pass2,inUV);

	// vec3 res=vec3(0.0);
	// vec3 var=vec3(0.0);

	// float weight=0.0;
	// for(int u=-1;u<=1;u++)
	// {
	// 	for(int v=-1;v<=1;v++)
	// 	{
	// 		float kw=wx[u+1]*wy[v+1];
	// 		vec3 data=texture(pass2,inUV+vec2(u,v)*w).xyz;
	// 		res+=data*kw;
	// 		weight+=kw;
	// 	}
	// }
	// res/=weight;
	// outColor.xyz=outColor.xyz+(outColor.xyz-res)*(0.2+1.0*(outColor.w));
	float exposure=1.5;
	outColor=1.0-exp(-outColor*outColor*exposure);
	// outColor.xyz=1.0/(1.0+1e-5-outColor.xyz*outColor.xyz)-1;
    outColor=pow(outColor,vec4(0.45));
	// outColor.w=1.0;
	// outColor=vec4(outColor.w);
	// outColor=vec4(motion*100.0,0.0,1.0);
}
