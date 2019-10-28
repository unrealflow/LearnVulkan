#version 450
const float PI = 3.14159265358979323846;
layout(binding = 0) uniform sampler2D samplerPosition;
layout(binding = 1) uniform sampler2D samplerNormal;
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
}
preVP;
layout(binding = 9) uniform CameraProperties
{
    mat4 viewInverse;
    mat4 projInverse;
    float iTime;
    float delta;
    float upTime;
    uint ligthCount;
}
curVP;

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outColor1;
//difference return (0,++)

float radius=0.003;
int Range=3;
float _Range=float(Range);
float GetWeight(int i,int j)
{
    float fi=float(i);
    float fj=float(j);
    
    vec2 vf=vec2(fi,fj);
    // color=color*color;
    float l0=length(vf);
    l0=smoothstep(0.0,_Range,l0);
    l0=(cos(l0*PI)+1.0)/2;
    l0=l0*l0;
    return l0;
}
//值越大表示差异越大
float ev(vec3 a, vec3 b)
{
    return exp(0.5 * length(a - b) ) - 1;
}
float ev(vec4 a, vec4 b)
{
    return exp(0.5 * length(a - b) ) - 1;
}
const float evSize = 10.0;
//值越小表示差异越大
float compare(in vec3 fragPos, in vec3 normal, in vec2 preUV)
{

    vec3 preP = texture(prePosition, preUV).rgb;
    vec3 preN = texture(preNormal, preUV).rgb;
    float factor = ev(fragPos, preP) + ev(normal, preN);
    return 1 / (exp(factor * evSize));
}

vec4 DeAlbedo(vec2 _inUV)
{
    return texture(rtImage, _inUV) / (texture(samplerAlbedo, _inUV)+1e-5);
}

void main()
{

    vec3 fragPos = texture(samplerPosition, inUV).rgb;
    vec4 normalTex = texture(samplerNormal, inUV);
    //无物体的区域直接设为背景值
    if(normalTex.a<0.1)
    {
        outColor=outColor1=texture(rtImage,inUV);
        return;
    }
    vec3 normal=normalTex.xyz;
    vec4 albedo = texture(samplerAlbedo, inUV);
    
    // vec4 preFragPos = (preVP.proj * preVP.view * vec4(fragPos,1.0));
    // preFragPos = preFragPos / preFragPos.w;
    // vec2 preUV = preFragPos.xy * 0.5 + 0.5;


    vec2 tex_offset = textureSize(preFrame, 0);
    float radius_x=radius*tex_offset.y/tex_offset.x;
    float totalWeight=0.0;

    //Begin：对光追结果进行模糊
    vec4 rtColor=vec4(0.0);
    for(int i=-Range;i<=Range;i++)
    {
        for(int j=-Range;j<=Range;j++)
        {
            float u=clamp(inUV.x+radius_x*i,radius_x,1.0-radius_x);
            float v=clamp(inUV.y+radius*j,radius,1.0-radius);
            vec2 _uv=vec2(u,v);
            float weight=GetWeight(i,j)*compare(fragPos, normal, _uv);
            vec4 pColor=DeAlbedo(_uv);
            rtColor+=pColor*weight;
            totalWeight+=weight;
        }
    }  
    rtColor /= totalWeight;
    vec4 curColor = rtColor * albedo;
    //End
    
    //获取上一帧的结果图像
    vec4 preFr = texture(preFrame, inUV);

    //Begin：计算当前位置的保留系数，值越小表示差异越大，使新图像的权重更大
    float deltaTime = curVP.iTime - curVP.upTime;
    deltaTime*=0.96;
    float f0=compare(fragPos, normal, inUV);
    float factor = max(f0, 0) * deltaTime / (deltaTime + curVP.delta);
    //End

    // curColor=texture(rtImage,inUV);
    outColor = mix(curColor, preFr, factor);
    float factor1 = curVP.delta / (deltaTime + curVP.delta);
    outColor = clamp(outColor, preFr - factor1, preFr + factor1);
    outColor1=max(outColor,vec4(0.0));
}