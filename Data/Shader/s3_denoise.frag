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
layout(binding = 8) uniform UBO
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

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;
// layout(location = 1) out vec4 outColor1;
//difference return (0,++)

float radius = 0.003;
//Begin：为测试不同卷积核，在此计算权重，若要优化时可换成直接读取矩阵或数组的值
float GetWeight(float _Range,int i, int j)
{
    float fi = float(i);
    float fj = float(j);

    vec2 vf = vec2(fi, fj);
    // color=color*color;
    float l0 = length(vf);
    l0 = smoothstep(0.0, _Range*1.5, l0);
    l0 = (cos(l0 * PI) + 1.0) / 2;
    l0 = l0 * l0;
    return l0;
}
//End

//值越大表示差异越大
float ev(vec3 a, vec3 b)
{
    return exp(length(a - b)) - 1;
}
float ev(vec4 a, vec4 b)
{
    return exp(length(a - b)) - 1;
}
//值越小表示差异越大
float compare(in vec3 fragPos, in vec3 normal, in vec2 preUV,float evSize)
{

    vec3 preP = texture(prePosition, preUV).rgb;
    vec3 preN = texture(preNormal, preUV).rgb;
    float factor = ev(fragPos, preP) + ev(normal, preN);
    // return clamp(1.05 / exp(factor * evSize)-0.05,0.0,1.0);
    return 1.0 / exp(factor * evSize);
}

vec4 DeAlbedo(vec2 _inUV)
{
    // return texture(rtImage, _inUV) / (texture(samplerAlbedo, _inUV) + 1e-5);
    return texture(rtImage,_inUV);
}

void main()
{

    vec3 fragPos = texture(samplerPosition, inUV).rgb;
    vec4 normalTex = texture(samplerNormal, inUV);
    vec4 albedo = texture(samplerAlbedo, inUV);
    //无物体的区域直接设为背景值
    if (albedo.a < 0.01) {
        outColor = texture(rtImage, inUV);
        return;
    }
    vec3 normal = normalTex.xyz;
   
    vec2 preUV = inUV;
    float deltaTime = ubo.iTime - ubo.upTime;
    if (deltaTime < 0.016) 
    {
        vec4 preFragPos = (ubo.preProj * ubo.preView * vec4(fragPos, 1.0));
        preFragPos = preFragPos / preFragPos.w;
        preUV = preFragPos.xy * 0.5 + 0.5;
    }
    //获取上一帧的结果图像
    vec4 preFr = texture(preFrame, preUV);

    //Begin：计算当前位置的保留系数，值越小表示差异越大，使新图像的权重更大
    // deltaTime *= 0.96;
    float f0 = compare(fragPos, normal, preUV,5.0);
    float factor = deltaTime / (deltaTime + ubo.delta);
    float minimum=0.9*f0;
    float maximum=0.98;
    factor=minimum+(maximum-minimum)*factor;
    //End

    //Begin：对光追结果进行模糊
    vec2 tex_offset = textureSize(preFrame, 0);
    float radius_x = radius * tex_offset.y / tex_offset.x;
    float totalWeight = 0.0;
    int Range = 3;
    if(factor<0.7)
    {
        Range=8;
    }
    float _Range=float(Range);
    vec4 rtColor = vec4(0.0);
    for (int i = -Range; i <= Range; i++) {
        for (int j = -Range; j <= Range; j++) {
            float u = clamp(inUV.x + radius_x * i, radius_x, 1.0 - radius_x);
            float v = clamp(inUV.y + radius * j, radius, 1.0 - radius);
            vec2 _uv = vec2(u, v);
            float weight = GetWeight(_Range,i, j) * compare(fragPos, normal, _uv,1.0);
            vec4 pColor = DeAlbedo(_uv);
            rtColor += pColor * weight;
            totalWeight += weight;
        }
    }
    rtColor /= totalWeight;
    vec4 curColor = rtColor * albedo;
    //End

    // curColor=texture(rtImage,inUV);
    // factor=f0;
    outColor = mix(curColor, preFr, factor);
    float factor1 = ubo.delta / (deltaTime + ubo.delta);
    outColor = clamp(outColor, preFr - factor1, preFr + factor1);
    outColor = max(outColor, vec4(0.0));
}