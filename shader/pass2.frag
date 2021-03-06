#version 460
#extension GL_GOOGLE_include_directive : enable
#include "Switch.glsl"
#include "Common.glsl"


layout (input_attachment_index = 0, binding = 0) uniform subpassInput samplerPosition;
layout (input_attachment_index = 1, binding = 1) uniform subpassInput samplerNormal;
layout (set = 0, binding = 2) uniform sampler2D samplerAlbedo;
layout (set = 0, binding = 3) uniform sampler2D pass1;
layout(set = 0, binding = 4) uniform UBO
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
layout (set = 0, binding = 5) uniform sampler2D prePass2;

// const vec2 w=vec2(0.001);

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

vec3 RGBToYCoCg(vec3 c)
{
	return vec3(
		0.5*c.g+0.25*(c.r+c.b),
		0.5*(c.r-c.b),
		0.5*c.g-0.25*(c.r+c.b)
	);
}
vec3 YCoCgToRGB(vec3 c)
{
	return vec3(
		c.x+c.y-c.z,
		c.x+c.z,
		c.x-c.y-c.z
	);
}
float noise11( float a )
{
  vec2 p=vec2(a,sin(a * 930.1 + 4929.7) * (a+23.3280));
	vec2 p2 = fract(vec2(p) / vec2(3.07965, 7.4235));
    p2 += dot(p2.yx, p2.xy+19.19);
	return fract(p2.x * p2.y);
}
vec3 ClipAABB(vec3 preSample, vec3 aabbMin, vec3 aabbMax)
{
	vec3 p_clip = 0.5 * (aabbMax + aabbMin);
	vec3 e_clip = 0.5 * (aabbMax - aabbMin);

	vec3 v_clip = preSample - p_clip;
	vec3 v_unit = v_clip.xyz / e_clip;
	vec3 a_unit = abs(v_unit);
	float ma_unit = max(a_unit.x, max(a_unit.y, a_unit.z));

	if (ma_unit > 1.0)
		return p_clip + v_clip / ma_unit;
	else
		return preSample;// point inside aabb
}
vec4 ClipAABB(vec4 preSample, vec4 aabbMin, vec4 aabbMax)
{
	preSample.xyz=ClipAABB(preSample.xyz,aabbMin.xyz,aabbMax.xyz);
	preSample.w=clamp(preSample.w,aabbMin.w,aabbMax.w);
	return preSample;
}	

vec2 UnpackFloat(float x)
{
	return unpackHalf2x16(floatBitsToUint(x));
}
float PackFloat(vec2 x)
{
	return uintBitsToFloat(packHalf2x16(x));
}

void UnpackTex(vec3 data,out vec3 a,out vec3 b)
{
	vec2 k0=UnpackFloat(data.x);
	vec2 k1=UnpackFloat(data.y);
	vec2 k2=UnpackFloat(data.z);
	a=vec3(k0.x,k1.x,k2.x);
	b=vec3(k0.y,k1.y,k2.y);
}
vec3 PackTex(vec3 a,vec3 b)
{
	float x=PackFloat(vec2(a.x,b.x));
	float y=PackFloat(vec2(a.y,b.y));
	float z=PackFloat(vec2(a.z,b.z));
	return vec3(x,y,z);
}

void main() 
{
#ifdef USE_TAA
	// Read G-Buffer values from previous sub pass
	vec4 fragPos = subpassLoad(samplerPosition);
	vec4 normal = subpassLoad(samplerNormal);
	vec4 albedo = texture(samplerAlbedo,inUV);
	const ivec2 texSize=textureSize(pass1,0);
	const vec2 w=1.0/texSize;
    vec4 curColor=texture(pass1,inUV);
	// if(fragPos.w<1e-4||abs(inUV.x-0.5)>(0.5-w.x)||abs(inUV.y-0.5)>(0.5-w.y))
	// {
	// 	outColor=curColor;
	// 	// outColor=vec4(1.0);
	// 	return;
	// }

	vec2 preUV = inUV;
    float deltaTime = ubo.iTime - ubo.upTime;
    if (deltaTime < 0.016) 
    {
		vec2 motion = unpackHalf2x16(floatBitsToUint(normal.w));
		preUV=inUV+motion;
    }
	vec4 preColor=texture(prePass2,preUV);
#ifdef USE_BLOOM

	float bloom_radius=0.1;
	int count=8;
	for(int i=1;i<=8;i++)
	{
		vec2 bias=bloom_radius*vec2(noise11(ubo.iTime+inUV.x+10.0*D_x[i]),noise11(ubo.iTime+inUV.y+10.0*D_y[i]));
		vec3 lum3=texture(pass1,inUV+bias).xyz;
		vec3 lum4=texture(pass1,inUV-bias).xyz;
		curColor.xyz+=0.2*(clamp(lum3-0.9,0.0,0.1)+clamp(lum4-0.9,0.0,0.1));
	}


#endif
	// vec4 preColor=vec4(1.0);
	// vec3 preAlbedo=vec3(1.0);
	// UnpackTex(preData.xyz,preColor.xyz,preAlbedo);
#ifdef USE_CLIP
	vec4 minColor=curColor;
	minColor.xyz=RGBToYCoCg(curColor.xyz);
	vec4 maxColor=minColor;
	vec4 aveg=vec4(0.0);
	vec4 var=vec4(0.0);
	int R=2;
	for(int u=-R;u<=R;u++)
	{
		for(int v=-R;v<=R;v++)
		{
			// vec3 albData=texture(samplerAlbedo,inUV+vec2(u,v)*w).xyz;
			vec4 texData=texture(pass1,inUV+vec2(u,v)*w);

			vec4 data=texData;
			data.xyz=RGBToYCoCg(texData.xyz);

			minColor=min(data,minColor);
			maxColor=max(data,maxColor);
			aveg+=data;
			var+=data*data;
		}
	}
	float N=float(R)*2.0+1.0;
	N=N*N;
	aveg/=N;
	var=sqrt(var/N-aveg*aveg);

	preColor.xyz=RGBToYCoCg(preColor.xyz);

	float factor1 = 1.01*ubo.delta / (deltaTime + ubo.delta);
    minColor = ClipAABB(minColor, preColor - factor1, preColor + factor1);
    maxColor = ClipAABB(maxColor, preColor - factor1, preColor + factor1);
    aveg = ClipAABB(aveg, minColor, maxColor);

	// preColor=ClipAABB(preColor,minColor,maxColor);
	vec4 v_k=5.0*var;
	preColor=ClipAABB(preColor,aveg-v_k,aveg+v_k);
	preColor.xyz=YCoCgToRGB(preColor.xyz);

	// outColor=mix(curColor,preColor,max(0.95,1.0-factor1));
	outColor=mix(curColor,preColor,0.99);
#else 
	outColor=mix(curColor,preColor,0.99);
#endif
	// outColor=clamp(outColor,0.0,1.0);
	// vec3 outAlbedo=mix(albedo.xyz,preAlbedo.xyz,0.98);
	// outAlbedo=albedo.xyz;
	// outColor = curColor;
	// outColor.xyz=PackTex(outColor.xyz,outAlbedo);
	// outColor=albedo;
#else 
	outColor=texture(pass1,inUV);
#endif
	outColor.a=0.299 * outColor.r + 0.587 * outColor.g + 0.114 * outColor.b;
}
