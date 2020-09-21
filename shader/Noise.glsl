#ifndef USE_NOISE
#define USE_NOISE

#define MOD2 vec2(3.07965, 7.4235)
#ifndef PI
  #define PI 3.1415926535897932384626433f
#endif

float acosFast4(float inX)
{
    float x1 = abs(inX);
    float x2 = x1 * x1;
    float x3 = x2 * x1;
    float s;

    s = -0.2121144f * x1 + 1.5707288f;
    s = 0.0742610f * x2 + s;
    s = -0.0187293f * x3 + s;
    s = sqrt(1.0f - x1) * s;

    return inX >= 0.0f ? s : 3.1415926535897932384626433f - s;
}
float asinFast4(float inX)
{
    float x = inX;

    // asin is offset of acos
    return 1.5707963267948966192313217f - acosFast4(x);
}
// float noise11(float a)
// {
//   return fract(sin(a * 930.1 + 4929.7)+23.3280);
// }
// float noise11(float a)
// {
//   return fract(sin(a * 930.1 + 4929.7) * (a+23.3280));
// }
float noise11( float a )
{
  vec2 p=vec2(a,sin(a * 930.1 + 4929.7) * (a+23.3280));
	vec2 p2 = fract(vec2(p) / MOD2);
    p2 += dot(p2.yx, p2.xy+19.19);
	return fract(p2.x * p2.y);
}

vec3 norm_noise(vec2 uv)
{
    float t1 = PI * noise11(uv.x) + PI;
    float t2 = PI * noise11(uv.y) + PI;
    float t3 = 2.0 * PI * noise11(t2 * uv.x - t1 * uv.y);
    float t4 = 2.0 * noise11(t1 * uv.x + t2 * uv.y) - 1.0;
    float t5 = t4;
    float r = sqrt(1.0 - t5 * t5);
    vec3 p = vec3(r * sin(t3), r * cos(t3), t5);
    return p;
}

vec3 noise_light(vec2 uv, float a)
{
    vec3 t = norm_noise(uv);
    float l = noise11(t.y) * a + (1.0 - a);
    return l * t;
}


vec3 noise_normal2(vec3 normal, vec2 uv, float a)
{
    vec3 p = norm_noise(uv);
    p = normalize(cross(p, normal));
    float t = noise11(uv.x + uv.y + a);
    t = sqrt(t);
    t = t / ((1.0 - a) * t + a);
    float peak = a;
    t = t * (1.0 - peak) + peak;
    return normalize(mix(p, normal, t));
}
vec3 noise_normal(vec3 normal, vec2 uv, float a)
{
    vec3 p = vec3(0.0,0.0,1.0)+a;
    p = normalize(cross(p, normal));
    vec3 kp=normalize(cross(p,normal));
    float rad=2.0*PI*noise11(dot(uv,uv)+a);
    p=p*cos(rad)+kp*sin(rad);
    float t = noise11(uv.x + uv.y + a);
    a=a*a;
    float s=2.0/PI*asinFast4(sqrt(t*a/(1+t*(a-1))));
    return normalize(mix(normal,p,s));
}
#endif