cd ..\..\bin\Shader & glslangValidator -V ..\..\Data\Shader\s3_gbuffer.vert -o vert_3_gbuffer.spv  &  glslangValidator -V ..\..\Data\Shader\s3_gbuffer.frag  -o frag_3_gbuffer.spv& glslangValidator -V ..\..\Data\Shader\s3_denoise.vert -o vert_3_denoise.spv  &  glslangValidator -V ..\..\Data\Shader\s3_denoise.frag  -o frag_3_denoise.spv & glslangValidator -V ..\..\Data\Shader\s3_closesthit.rchit -o s3_closesthit.spv & glslangValidator -V ..\..\Data\Shader\s3_miss.rmiss -o s3_miss.spv & glslangValidator -V ..\..\Data\Shader\s3_raygen.rgen -o s3_raygen.spv & glslangValidator -V ..\..\Data\Shader\s3_shadow.rmiss -o s3_shadow.spv &  cd ..\..\Example\3_SkRender



