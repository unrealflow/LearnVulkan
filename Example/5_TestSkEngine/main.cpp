﻿#include "stdio.h"
#include "SkCommon.h"
#include "SkRender.h"
std::vector<float> vertices={-1.000000f,-1.000000f,-1.000000f,-0.577349f,-0.577349f,-0.577349f,0.375000f,0.000000f,-1.000000f,-1.000000f,1.000000f,-0.577349f,-0.577349f,0.577349f,0.62500f,0.000000f,-1.000000f,1.000000f,1.000000f,-0.577349f,0.577349f,0.577349f,0.625000f,0.250000f,-1.000000f,1.000000f,-1.000000f,-0.577349f,0.577349f,-0.577349f,0.375000f,0.250000f,-1.00000f,1.000000f,-1.000000f,-0.577349f,0.577349f,-0.577349f,0.375000f,0.250000f,-1.000000f,1.000000f,1.000000f,-0.577349f,0.577349f,0.577349f,0.625000f,0.250000f,1.000000f,1.000000f,1.000000f,0.57734f,0.577349f,0.577349f,0.625000f,0.500000f,1.000000f,1.000000f,-1.000000f,0.577349f,0.577349f,-0.577349f,0.375000f,0.500000f,1.000000f,1.000000f,-1.000000f,0.577349f,0.577349f,-0.577349f,0.37500f,0.500000f,1.000000f,1.000000f,1.000000f,0.577349f,0.577349f,0.577349f,0.625000f,0.500000f,1.000000f,-1.000000f,1.000000f,0.577349f,-0.577349f,0.577349f,0.625000f,0.750000f,1.000000f,-1.00000f,-1.000000f,0.577349f,-0.577349f,-0.577349f,0.375000f,0.750000f,1.000000f,-1.000000f,-1.000000f,0.577349f,-0.577349f,-0.577349f,0.375000f,0.750000f,1.000000f,-1.000000f,1.000000f,0.57734f,-0.577349f,0.577349f,0.625000f,0.750000f,-1.000000f,-1.000000f,1.000000f,-0.577349f,-0.577349f,0.577349f,0.625000f,1.000000f,-1.000000f,-1.000000f,-1.000000f,-0.577349f,-0.577349f,-0.57734f,0.375000f,1.000000f,-1.000000f,1.000000f,-1.000000f,-0.577349f,0.577349f,-0.577349f,0.125000f,0.500000f,1.000000f,1.000000f,-1.000000f,0.577349f,0.577349f,-0.577349f,0.375000f,0.50000f,1.000000f,-1.000000f,-1.000000f,0.577349f,-0.577349f,-0.577349f,0.375000f,0.750000f,-1.000000f,-1.000000f,-1.000000f,-0.577349f,-0.577349f,-0.577349f,0.125000f,0.750000f,1.000000f,1.00000f,1.000000f,0.577349f,0.577349f,0.577349f,0.625000f,0.500000f,-1.000000f,1.000000f,1.000000f,-0.577349f,0.577349f,0.577349f,0.875000f,0.500000f,-1.000000f,-1.000000f,1.000000f,-0.57734f,-0.577349f,0.577349f,0.875000f,0.750000f,1.000000f,-1.000000f,1.000000f,0.577349f,-0.577349f,0.577349f,0.625000f,0.750000f,};
std::vector<uint32_t> indices={0,1,2,0,2,3,4,5,6,4,6,7,8,9,10,8,10,11,12,13,14,12,14,15,16,17,18,16,18,19,20,21,22,20,22,23};



int main()
{
    BMaterial mat={};
    mat.baseColor={0.4f,0.8f,0.3f,1.0f};
    mat.metallic=0.1f;
    mat.roughness=0.3f;

    BTransform trans={};
    trans.Position={0.0f,0.0f,0.0f};
    trans.Rotation={0.0f,0.0f,0.0f};
    trans.Scale={1.0f,1.0f,1.0f};

    BMesh mesh={};
    mesh.I=indices.data();
    mesh.Ic=(uint32_t)indices.size();
    mesh.V=vertices.data();
    mesh.Vc=(uint32_t)vertices.size();
    mesh.M=&mat;
    mesh.T=&trans;

    BLight light={};
    light.type=0.0;
    light.pos=glm::vec3(0.0f,-90.0f,30.0f);
    light.color=glm::vec3(10.0f);
    printf("Test....\n");
    BScene scene={};
    scene.nums=1;
    scene.meshes=&mesh;
    scene.lightCount=0;
    scene.lights=&light;
    SkRenderEngine engine;
    return engine.Render(&scene);
}