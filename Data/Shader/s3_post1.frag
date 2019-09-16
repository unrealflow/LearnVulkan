#version 450
const float PI = 3.14159265358979323846;

layout(binding = 0) uniform sampler2D position;
layout(binding = 1) uniform sampler2D normal;
layout(binding = 2) uniform sampler2D albedo;
layout(binding = 3) uniform sampler2D post0;
layout(binding = 4) uniform sampler2D post1;


layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;

float radius=0.005;
int Range=10;
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
void main()
{
    vec3 baseColor=texture(post0,inUV).xyz;

    vec3 inputColor=vec3(0.);
    float totalWeight=0.0;
    vec2 tex_offset = textureSize(post0, 0);
    float aspect=tex_offset.y/tex_offset.x;
    float radius_x=radius*aspect;
    for(int i=-Range;i<=Range;i++)
    {
        for(int j=-Range;j<=Range;j++)
        {
            float u=clamp(inUV.x+radius_x*i,radius_x,1.0-radius_x);
            float v=clamp(inUV.y+radius*j,radius,1.0-radius);
            vec3 pColor=texture(post1,vec2(u,v)).xyz;
            float w=GetWeight(i,j);
            totalWeight+=w;
            inputColor+=w*pColor;
        }
    }
    inputColor = inputColor / totalWeight;
    // inputColor = 2.0 * max((inputColor - 0.2),0.0) * max((inputColor - 0.4),0.0) * inputColor;
    inputColor = max(inputColor, vec3(0.));
    baseColor += inputColor;
    baseColor=pow(baseColor,vec3(0.5));
    outColor=vec4(baseColor,1.0);
}