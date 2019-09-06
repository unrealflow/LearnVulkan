set(SK_ENGINE_CORE "SkRender.cpp")
set(SK_ENGINE_MAIN "main.cpp")
include_directories(${LearnVulkanProject}/include)
add_library(Core STATIC ${SK_ENGINE_CORE})

link_libraries(Core)
add_library(SkEngine SHARED ${SK_ENGINE_MAIN})