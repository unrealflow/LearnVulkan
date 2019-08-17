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
set(INCLUDE ${LearnVulkanProject}/include ${LearnVulkanProject}/base)

add_definitions(-DVK_EXAMPLE_DATA_DIR=\"${CMAKE_SOURCE_DIR}/bin/\")
link_directories(${LearnVulkanProject}/lib)

SET( CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS} "/NODEFAULTLIB:library" )


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

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${LearnVulkanProject}/bin)   
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${LearnVulkanProject}/bin)   
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${LearnVulkanProject}/bin)   
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${LearnVulkanProject}/bin) 
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${LearnVulkanProject}/bin)   
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${LearnVulkanProject}/bin) 

add_subdirectory(${LearnVulkanProject}/Base)
include_directories(${INCLUDE})
link_libraries(base)
add_subdirectory(${LearnVulkanProject}/Example/1_Triangle)
add_subdirectory(${LearnVulkanProject}/Example/3_SkRender)
