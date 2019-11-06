#include "stdio.h"
#include "SkCommon.h"
#include "SkRender.h"

extern "C" _declspec(dllexport) int render(BScene *s)
{
    SkRenderEngine *engine=new SkRenderEngine();
    printf("SkRender\n");
    printf("Light Count:%d\n",s->lightCount);
    // printf("Mesh Nums %d,\n", s->nums);
    // for (uint32_t i = 0; i < s->nums; i++)
    // {
    //     fprintf(stderr, "mesh[%d]...\n", i);

    //     fprintf(stderr, "Vertex Count:%d...\n", s->meshes[i].Vc);
    //     for (uint32_t j = 0; j < s->meshes[i].Vc; j++)
    //     {
    //         printf("%f,",s->meshes[i].V[j]);
    //     }
    //     fprintf(stderr, "\nIndex Count:%d...\n", s->meshes[i].Ic);
    //     for (uint32_t j = 0; j < s->meshes[i].Ic; j++)
    //     {
    //         printf("%d,",s->meshes[i].I[j]);
    //     }
    //     fprintf(stderr, "mesh[%d]..end.\n", i);
    // }
    engine->Render(s);
    delete engine;
    return 0;
}