#define LOC_UNIFORM 100
#define LOC_DIFFUSE 101
#define LOC_STRIDE 5
#define LOC_VERTEX 102
#define LOC_INDEX 103
#define LOC_LIGHT 50
#define MACTEST(x)  (x+10)
struct Mat
{
    vec3 baseColor;
    float metallic;
    float subsurface;
    float specular;
    float roughness;
    float specularTint;
    float anisotropic;
    float sheen;
    float sheenTint;
    float clearcoat;
    float clearcoatGloss;
    float emission;
};
struct Light
{
    float type;
    vec3 pos;
    vec3 dir;
    vec3 color;
    float radius;
    float atten;
};
layout(binding = 2, set = 0) uniform CameraProperties
{
	mat4 viewInverse;
	mat4 projInverse;
	// vec4 lightPos;
	float primNums[];
} cam;
layout(set = 0, binding = LOC_LIGHT  ) buffer Lights{ vec4 l[]; } lights;

#define MESH_INFO(_set,_offset,_i) \
    layout(set = _set, binding = LOC_UNIFORM + _offset*LOC_STRIDE) uniform Material##_i { Mat m; } mat_##_i; \
    layout(set = _set, binding = LOC_DIFFUSE + _offset*LOC_STRIDE) uniform sampler2D tex##_i; \
    layout(set = _set, binding = LOC_VERTEX  + _offset*LOC_STRIDE) buffer Vertices##_i { vec4 v[]; } vertices##_i; \
    layout(set = _set, binding = LOC_INDEX   + _offset*LOC_STRIDE) buffer Indices##_i { uint i[]; } indices##_i; \

MESH_INFO(1,0,0)
MESH_INFO(2,0,1)
MESH_INFO(3,0,2)
#undef MESH_INFO


Light GetLight(uint index)
{
    vec4 l0=lights.l[index*3];
    vec4 l1=lights.l[index*3+1];
    vec4 l2=lights.l[index*3+2];
    Light l;
    l.type=l0.x; 
    l.pos=l0.yzw;
    l.dir=l1.xyz;
    l.color=vec3(l1.w,l2.xy);
    l.radius=l2.z;
    l.atten=l2.w;
    return l;
}
ivec2 GetMeshID(uint id)
{
    float f=float(id);
    if(f<cam.primNums[0])
        return ivec2(0,int(f));
    else
        f-=cam.primNums[0];

    if(f<cam.primNums[1])
        return ivec2(1,int(f));
    else
        f-=cam.primNums[1];

    return ivec2(2,int(f));
}
struct RP
{
    vec3 color;
    vec3 position;
    vec3 direction;
};
const float PI = 3.14159265358979323846;

float sqr(float x) { return x * x; }

float SchlickFresnel(float u)
{
    float m = clamp(1 - u, 0, 1);
    float m2 = m * m;
    return m2 * m2 * m; // pow(m,5)
}

float GTR1(float NdotH, float a)
{
    if (a >= 1)
        return 1 / PI;
    float a2 = a * a;
    float t = 1 + (a2 - 1) * NdotH * NdotH;
    return (a2 - 1) / (PI * log(a2) * t);
}

float GTR2(float NdotH, float a)
{
    float a2 = a * a;
    float t = 1 + (a2 - 1) * NdotH * NdotH;
    return a2 / (PI * t * t);
}

float GTR2_aniso(float NdotH, float HdotX, float HdotY, float ax, float ay)
{
    return 1 / (PI * ax * ay * sqr(sqr(HdotX / ax) + sqr(HdotY / ay) + NdotH * NdotH));
}

float smithG_GGX(float NdotV, float alphaG)
{
    float a = alphaG * alphaG;
    float b = NdotV * NdotV;
    return 1 / (NdotV + sqrt(a + b - a * b));
}

float smithG_GGX_aniso(float NdotV, float VdotX, float VdotY, float ax, float ay)
{
    return 1 / (NdotV + sqrt(sqr(VdotX * ax) + sqr(VdotY * ay) + sqr(NdotV)));
}

vec3 mon2lin(vec3 x)
{
    return vec3(pow(x[0], 2.2), pow(x[1], 2.2), pow(x[2], 2.2));
}

vec3 BRDF(Mat mat, vec3 v_color, vec3 L, vec3 V, vec3 N, vec3 X, vec3 Y)
{
    float NdotL = dot(N, L);
    float NdotV = dot(N, V);
    if (NdotL < 0 || NdotV < 0)
        return vec3(0);

    vec3 H = normalize(L + V);
    float NdotH = dot(N, H);
    float LdotH = dot(L, H);

    vec3 Cdlin = mon2lin(v_color);
    float Cdlum = .3 * Cdlin[0] + .6 * Cdlin[1] + .1 * Cdlin[2]; // luminance approx.

    vec3 Ctint = Cdlum > 0 ? Cdlin / Cdlum : vec3(1); // normalize lum. to isolate hue+sat
    vec3 Cspec0 = mix(mat.specular * .08 * mix(vec3(1), Ctint, mat.specularTint), Cdlin, mat.metallic);
    vec3 Csheen = mix(vec3(1), Ctint, mat.sheenTint);

    // Diffuse fresnel - go from 1 at normal incidence to .5 at grazing
    // and mix in diffuse retro-reflection based on roughness
    float FL = SchlickFresnel(NdotL), FV = SchlickFresnel(NdotV);
    float Fd90 = 0.5 + 2 * LdotH * LdotH * mat.roughness;
    float Fd = mix(1.0, Fd90, FL) * mix(1.0, Fd90, FV);

    // Based on Hanrahan-Krueger brdf approximation of isotropic bssrdf
    // 1.25 scale is used to (roughly) preserve albedo
    // Fss90 used to "flatten" retroreflection based on mat.roughness
    float Fss90 = LdotH * LdotH * mat.roughness;
    float Fss = mix(1.0, Fss90, FL) * mix(1.0, Fss90, FV);
    float ss = 1.25 * (Fss * (1 / (NdotL + NdotV) - .5) + .5);

    // specular
    float aspect = sqrt(1 - mat.anisotropic * .9);
    float ax = max(.3, sqr(mat.roughness) / aspect);
    float ay = max(.3, sqr(mat.roughness) * aspect);
    float Ds = GTR2_aniso(NdotH, dot(H, X), dot(H, Y), ax, ay);
    float FH = SchlickFresnel(LdotH);
    vec3 Fs = mix(Cspec0, vec3(1), FH);
    float Gs;
    Gs = smithG_GGX_aniso(NdotL, dot(L, X), dot(L, Y), ax, ay);
    Gs *= smithG_GGX_aniso(NdotV, dot(V, X), dot(V, Y), ax, ay);

    // sheen
    vec3 Fsheen = FH * mat.sheen * Csheen;

    // clearcoat (ior = 1.5 -> F0 = 0.04)
    float Dr = GTR1(NdotH, mix(.1, .001, mat.clearcoatGloss));
    float Fr = mix(.04, 1.0, FH);
    float Gr = smithG_GGX(NdotL, .25) * smithG_GGX(NdotV, .25);

    return ((1 / PI) * mix(Fd, ss, mat.subsurface) * Cdlin + Fsheen) * (1 - mat.metallic) + Gs * Fs * Ds + .25 * mat.clearcoat * Gr * Fr * Dr;
}
