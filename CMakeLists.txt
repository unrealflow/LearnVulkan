cmake_minimum_required(VERSION 2.8)

project(LearnVulkan)

if (MSVC_VERSION GREATER_EQUAL "1900")
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
    if (_cpp_latest_flag_supported)
        add_compile_options("/std:c++latest")
    endif()
endif()

set(LearnVulkanProject ${PROJECT_SOURCE_DIR})
set(SK_INCLUDE ${LearnVulkanProject}/Include ${LearnVulkanProject}/Base)

add_definitions(-DVK_EXAMPLE_DATA_DIR=\"${CMAKE_SOURCE_DIR}/Bin/\")
link_directories(${LearnVulkanProject}/Lib)

# SET( CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS} "/NODEFAULTLIB:library" )


IF( CMAKE_SIZEOF_VOID_P EQUAL 8)
    #64位静态链接库
    link_directories(${LearnVulkanProject}/Lib)
ELSE( CMAKE_SIZEOF_VOID_P EQUAL 8)
    #32位静态链接库
    link_directories(${LearnVulkanProject}/Lib32)  
ENDIF( CMAKE_SIZEOF_VOID_P EQUAL 8)

link_libraries(glfw3.lib)
link_libraries(vulkan-1.lib)
link_libraries(assimp.lib)

#exe可执行文件输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${LearnVulkanProject}/Bin)   
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${LearnVulkanProject}/Bin)   
#dll动态链接库输出目录
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${LearnVulkanProject}/Bin)   
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${LearnVulkanProject}/Bin) 
#lib静态链接库输出目录
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${LearnVulkanProject}/Bin)   
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${LearnVulkanProject}/Bin) 

add_subdirectory(${LearnVulkanProject}/Base)
include_directories(${SK_INCLUDE})
link_libraries(base)
add_subdirectory(${LearnVulkanProject}/Example/1_Triangle)
add_subdirectory(${LearnVulkanProject}/Example/3_SkRender)
add_subdirectory(${LearnVulkanProject}/Example/4_SkEngine)
add_subdirectory(${LearnVulkanProject}/Example/5_TestSkEngine)
