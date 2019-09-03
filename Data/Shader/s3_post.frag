#version 450

layout(binding = 0) uniform sampler2D position;
layout(binding = 1) uniform sampler2D normal;
layout(binding = 2) uniform sampler2D albedo;
layout(binding = 3) uniform sampler2D post;


layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;

float radius=0.001;
int Range=10;
float _Range=10.0;
vec3 GetWeight(int i,int j,in vec3 color)
{
    float fi=float(i);
    float fj=float(j);
    
    vec2 vf=vec2(fi,fj);
    // color=color*color;
    float l0=length(vf);
    l0=smoothstep(_Range,0.0,l0);
    float l1=length(color);
    l1=smoothstep(0.0,1.732,l1);
    return l0*l1*color;
}
const float gamma=2.2;
void main()
{
    vec3 baseColor=texture(post,inUV).xyz;

    vec3 inputColor=vec3(0.);
    vec3 totalWeight=vec3(0.0);
    vec2 tex_offset = textureSize(post, 0);
    float aspect=tex_offset.y/tex_offset.x;
    float radius_x=radius*aspect;
    for(int i=-Range;i<=Range;i++)
    {
        for(int j=-Range;j<=Range;j++)
        {
            float u=clamp(inUV.x+radius_x*i,radius_x,1.0-radius_x);
            float v=clamp(inUV.y+radius*j,radius,1.0-radius);
            vec3 pColor=texture(post,vec2(u,v)).xyz;
            vec3  w=GetWeight(i,j,pColor);
            totalWeight+=w;
            inputColor+=w*pColor;
        }
    }
    inputColor = inputColor / totalWeight;
    inputColor = 2.0 * (inputColor - 0.2) * (inputColor - 0.4) * inputColor;
    inputColor = max(inputColor, vec3(0.));
    baseColor += inputColor;
    outColor=vec4(baseColor,1.0);
}