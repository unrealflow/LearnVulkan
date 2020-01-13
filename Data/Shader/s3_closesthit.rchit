#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_NV_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#include "BRDF2.glsl"
#include "RayCommon.glsl"

layout(location = 0) rayPayloadInNV RP hitValue;
layout(location = 2) rayPayloadNV vec3 shadowed;
hitAttributeNV vec3 attribs;

layout(binding = 0, set = 0) uniform accelerationStructureNV topLevelAS;

vec3 rotX(vec3 inPos, float angle)
{
    return vec3(inPos.x,
        inPos.y * cos(angle) + inPos.z * sin(angle),
        inPos.z * cos(angle) - inPos.y * sin(angle));
}
vec3 rotY(vec3 inPos, float angle)
{
    return vec3(inPos.x * cos(angle) + inPos.z * sin(angle),
        inPos.y,
        inPos.z * cos(angle) - inPos.x * sin(angle));
}

float noise(float a)
{
    float k = fract(sin(13133.33 * a + 2333.123) * 13331.133);
    return k;
}
// float noise(float a)
// {
//     float k = fract(fract(13721.33 * a + 23.123) * 134531.133);
//     return k;
// }

vec3 norm_noise(vec2 uv)
{
    float t1 = PI * noise(uv.x) + PI;
    float t2 = PI * noise(uv.y) + PI;
    float t3 = 2.0 * PI * noise(t2 * uv.x - t1 * uv.y);
    float t4 = 2.0 * noise(t1 * uv.x + t2 * uv.y) - 1.0;
    float t5 = t4;
    float r = sqrt(1.0 - t5 * t5);
    vec3 p = vec3(r * sin(t3), r * cos(t3), t5);
    return p;
}
float pw5(float x)
{
    float k = x * x;
    return k * k * x;
}
vec3 noise_light(vec2 uv, float a)
{
    vec3 t = norm_noise(uv);
    float l = noise(t.y) * a + (1.0 - a);
    return l * t;
}
vec3 noise_normal(vec3 normal, vec2 uv, float a)
{
    vec3 p = norm_noise(uv);
    p = normalize(cross(p, normal));
    float t = noise(uv.x + uv.y + a);
    t = t * t;
    t = t / ((1.0 - a) * t + a);
    float peak = a;
    t = t * (1.0 - peak) + peak;
    return normalize(mix(p, normal, t));
}
void shader(Mat _mat, sampler2D _tex, Vertex v0, Vertex v1, Vertex v2)
{
    // Interpolate normal
    const vec3 barycentricCoords = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);
    vec3 normal = normalize(v0.normal * barycentricCoords.x + v1.normal * barycentricCoords.y + v2.normal * barycentricCoords.z);
    vec2 uv = v0.uv * barycentricCoords.x + v1.uv * barycentricCoords.y + v2.uv * barycentricCoords.z;
    vec3 origin = gl_WorldRayOriginNV + gl_WorldRayDirectionNV * gl_HitTNV;
    // Basic lighting
    hitValue.color = vec3(0.0);
    vec3 baseColor = _mat.baseColor;
    if (_mat.useTex > 0) {
        baseColor *= texture(_tex, uv).xyz;
    }
    vec3 view = gl_WorldRayDirectionNV;
    if (hitValue.kS.x < 0) {
        view = -view;
    }
    for (int l = 0; l < cam.lightCount; l++) {
        Light light = GetLight(l);
        vec3 lightVector;
        vec3 lightColor;
        int lightType = int(light.type);
        if (lightType == 2) {
            hitValue.color += baseColor * light.color;
            continue;
        }
        switch (lightType) {
        case 1:
            lightVector = normalize(-light.dir);
            lightColor = light.color;
            break;
        case 0:
        default:
            lightVector = normalize(light.pos + light.radius * noise_light(cam.iTime + origin.xy, 0.5) - origin);
            float d = max(distance(origin, light.pos), light.radius);
            lightColor = light.color / (1.0 + d * 13.0 * light.atten + pow(d, light.atten) * 3.0);
            break;
        }
        float intensity = 1.0 + 1.0 * (lightColor.x * 0.299 + lightColor.y * 0.587 + lightColor.z * 0.114);
        lightColor = lightColor / intensity;
        vec3 signalColor = intensity * BRDF(_mat, baseColor * lightColor, lightVector, -view, normal);
        // Shadow casting
        signalColor = clamp(signalColor, vec3(0.0), vec3(1.0));
        float tmin = 0.001;
        float tmax = 100.0;
        shadowed = vec3(1.0);
        // Offset indices to match shadow hit/miss index
        //| gl_RayFlagsSkipClosestHitShaderNV
        traceNV(topLevelAS, gl_RayFlagsTerminateOnFirstHitNV | gl_RayFlagsOpaqueNV, 0xFF, 1, 0, 1, origin, tmin, lightVector, tmax, 2);
        //TODO: real raytracing shadow is difficult
        signalColor *= shadowed;
        hitValue.color += signalColor;
    }
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, baseColor, 1.0 - _mat.metallic);
    hitValue.kS = fresnelSchlick(abs(dot(view, normal)), F0);

    hitValue.color += _mat.emission * baseColor;
    // hitValue.color=vec3(uv,0.0);
    hitValue.position = origin;
    vec3 d_normal = noise_normal(normal, cam.iTime + uv + hitValue.bias, _mat.roughness);
    // normal =normalize(normal+_mat.roughness*noise_light(cam.iTime + origin.xy + hitValue.bias,2.0));
    if (noise(cam.iTime + origin.x + origin.y + hitValue.bias) > _mat.transmission) {
        hitValue.direction = reflect(gl_WorldRayDirectionNV, d_normal);
        if (dot(hitValue.direction, normal) < 0) {
            hitValue.direction = -hitValue.direction;
        }
    } else {
        hitValue.color *= (1.0 - _mat.transmission * 1.0);
        hitValue.kS = (0.5 + 0.5 * baseColor) * _mat.transmission * 2.0;
        float theta = dot(gl_WorldRayDirectionNV, normal);
        float r = _mat.IOR;
        if (theta > 0) {
            // r = 1.0 / r;
            hitValue.direction = refract(gl_WorldRayDirectionNV, -normal, r);
            if (length(hitValue.direction) < 1e-5) {
                hitValue.direction = reflect(gl_WorldRayDirectionNV, -normal);
                hitValue.kS = -hitValue.kS;
                // hitValue.direction = gl_WorldRayDirectionNV;
            }
        } else {
            r = 1.0 / r;
            hitValue.kS = -hitValue.kS;
            hitValue.direction = refract(gl_WorldRayDirectionNV, normal, r);
        }
    }
}

void main()
{
    // ivec3 index = ivec3(indices0.i[3 * gl_PrimitiveID], indices0.i[3 * gl_PrimitiveID + 1], indices0.i[3 * gl_PrimitiveID + 2]);
    uint meshID = GetMeshID(gl_PrimitiveID);

    ivec3 index = GetIndices(gl_PrimitiveID);
    Vertex v0 = Unpack(index.x, meshID);
    Vertex v1 = Unpack(index.y, meshID);
    Vertex v2 = Unpack(index.z, meshID);
    meshID = clamp(meshID, 0, MAX_MESH);
    shader(GetMat(meshID), t_tex[meshID], v0, v1, v2);
}
