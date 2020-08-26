#version 460
#extension GL_GOOGLE_include_directive : enable
#include "Switch.glsl"

layout (set = 0, binding = 0) uniform sampler2D samplerPosition;
layout (set = 0, binding = 1) uniform sampler2D samplerNormal;
layout (set = 0, binding = 2) uniform sampler2D samplerAlbedo;
layout (set = 0, binding = 3) uniform sampler2D pass0;

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

//sobel边缘检测算子
const float kernel_sobel_y[3][3] = {
    {-1, -2, -1},
    {0, 0, 0},
    {1, 2, 1}};

const float kernel_sobel_x[3][3] = {
    {-1, 0, 1},
    {-2, 0, 2},
    {-1, 0, 1}};

vec3 blur(sampler2D tex, vec2 uv,vec2 w)
{
	if(abs(uv.x-0.5)>(0.5-w.x)||abs(uv.y-0.5)>(0.5-w.y)){
		return vec3(0.0);
	}
	vec3 res=vec3(0.0);
	for(int u=-1;u<=1;u++)
	{
		for(int v=-1;v<=1;v++)
		{
			vec3 data=texture(tex,uv+vec2(u,v)*w).xyz;
			res+=data;
		}
	}
	return res/9.0;
}

vec2 GradientPos(sampler2D tex, vec2 uv,vec2 w)
{
	if(abs(uv.x-0.5)>(0.5-w.x)||abs(uv.y-0.5)>(0.5-w.y)){
		return vec2(0.0);
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
	return vec2(length(res0),length(res1));
}

float WeightPos(vec2 Gx,vec3 Px,vec3 Py,vec2 ij)
{
	float R_p=1.0;
	float f0=distance(Px,Py);
	float f1=length(Gx*ij)*R_p+1e-5;
	return exp(-f0/f1);
}
float WeightNormal(vec3 Nx,vec3 Ny)
{
	float f=max(0.0,dot(Nx,Ny));
	float R_n=128.0;
	for(int i=0;i<5;i++)
	{
		f=f*f*f;
	}
	return f;
}
float WeightLum(float Vx,vec3 Lx,vec3 Ly)
{
	float R_l=4.0;
	float f0=distance(Lx,Ly);
	float f1=R_l*Vx+1e-5;
	return exp(-f0/f1);
	// return 1.0;
}
void main() 
{
#ifdef USE_BLUR
	// Read G-Buffer values from previous sub pass
	vec3 fragPos = texture(samplerPosition,inUV).rgb;
	vec3 normal = texture(samplerNormal,inUV).rgb;
	vec3 albedo = texture(samplerAlbedo,inUV).xyz;
	vec4 color=texture(pass0,inUV);

	vec2 tex_offset = textureSize(pass0, 0);
	vec2 w=1.0/tex_offset;
	vec2 G_x=GradientPos(samplerPosition,inUV,w);

	vec4 aveg=vec4(0.0);
	float weight=1e-5;
	int R=2;
	// int i=0;

	for(int i=-R;i<=R;i++)
	{
		for(int j=-R;j<=R;j++)
		{
			vec2 uv=inUV+w*vec2(i,j);
			vec4 L_y=texture(pass0,uv);
			vec3 P_y = texture(samplerPosition,inUV).rgb;
			vec3 N_y = texture(samplerNormal,inUV).rgb;
			float w_p=WeightPos(G_x,fragPos,P_y,vec2(i,j));
			float w_n=WeightNormal(normal,N_y);
			float w_l=WeightLum(color.w,color.xyz,L_y.xyz);
			float w=w_p*w_n*w_l;
			aveg+=w*L_y;
			weight+=w;
		}
	}

	float N=float(R)*2.0+1.0;
	N=N*N;
	aveg/=weight;
	outColor=aveg;
	outColor.xyz*=albedo.xyz;
#else
	outColor=texture(pass0,inUV);
#endif
}
