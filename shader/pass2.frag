#version 460

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

	// vec4 preColor=vec4(1.0);
	// vec3 preAlbedo=vec3(1.0);
	// UnpackTex(preData.xyz,preColor.xyz,preAlbedo);

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

	float factor1 = ubo.delta / (deltaTime + ubo.delta);
    minColor = clamp(minColor, preColor - factor1, preColor + factor1);
    maxColor = clamp(maxColor, preColor - factor1, preColor + factor1);
    aveg = clamp(aveg, minColor, maxColor);

	preColor=clamp(preColor,minColor,maxColor);
	vec4 v_k=1.0*var;
	preColor=clamp(preColor,aveg-v_k,aveg+v_k);
	preColor.xyz=YCoCgToRGB(preColor.xyz);

	outColor=mix(curColor,preColor,max(0.95,1.0-factor1));
	// outColor=clamp(outColor,0.0,1.0);
	// vec3 outAlbedo=mix(albedo.xyz,preAlbedo.xyz,0.98);
	// outAlbedo=albedo.xyz;
	// outColor = curColor;
	// outColor.xyz=PackTex(outColor.xyz,outAlbedo);
	// outColor=albedo;
}
