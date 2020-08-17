#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_separate_shader_objects : enable
#include "Switch.glsl"
#include "Common.glsl"

layout (set = 0,  binding = 0) uniform sampler2D samplerPosition;
layout (set = 0,  binding = 1) uniform sampler2D samplerNormal;
layout (set = 0,  binding = 2) uniform sampler2D samplerAlbedo;
layout(set = 0, binding = 3) uniform UBO
{
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 jitterProj;
    mat4 preView;
    mat4 preProj;
    mat4 viewInverse;
    mat4 projInverse;
    float iTime;
    float delta;
    float upTime;
    uint lightCount;
}
ubo;
layout (set = 0,  binding = 4) uniform sampler2D rtImage;
layout(set = 0, binding = LOC_UNIFORM) uniform Material { Mat m; }mat[3];

layout(set = 0, binding = LOC_DIFFUSE) uniform sampler2D tex[3];

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

vec3 PosToUV(vec3 pos)
{
	vec4 p=vec4(pos,1.0);
	p=ubo.jitterProj*ubo.view*p;
	p/=p.w;
	p.xy=p.xy*0.5+0.5;
	return p.xyz;
}

void main() 
{
	// Read G-Buffer values from previous sub pass
	// vec4 fragPos = texture(samplerPosition,inUV);
	// vec4 normal = texture(samplerNormal,inUV);
	vec4 albedo = texture(samplerAlbedo,inUV);
	// if(fragPos.w<1e-5){
	// 	outColor=albedo;
	// 	return;
	// }
#ifdef USE_BLUR	
	vec2 tex_offset = textureSize(rtImage, 0);
	vec2 w=1.0/tex_offset;
	vec3 color=texture(rtImage,inUV).xyz/(albedo.xyz+1e-5);
	vec3 aveg=vec3(0.0);
	float var=0.0;
	int R=1;
	for(int i=-R;i<=R;i++)
	{
		// int j=0;
		for(int j=-R;j<=R;j++)
		{
			vec2 uv=inUV+w*vec2(i,j);
			vec3 lum=texture(rtImage,uv).xyz/(albedo.xyz+1e-5);
			aveg+=lum;
			var+=dot(lum,lum);
		}
	}
	float N=float(R)*2.0+1.0;
	N=N*N;
	aveg/=N;
	var=sqrt(abs(var/N-dot(aveg,aveg)));
	// outColor=vec4(mix(color,aveg,0.5),var);
	outColor=vec4(color,var);
#else
	outColor=texture(rtImage,inUV);
	outColor.w=0.0;
#endif
}

