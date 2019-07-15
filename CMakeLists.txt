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
set(INCLUDE ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/base)
include_directories(${INCLUDE})

link_directories(${PROJECT_SOURCE_DIR}/lib)

SET( CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS} "/NODEFAULTLIB:library" )


IF( CMAKE_SIZEOF_VOID_P EQUAL 8)
    #64位静态链接库
    link_directories(${PROJECT_SOURCE_DIR}/Lib)
ELSE( CMAKE_SIZEOF_VOID_P EQUAL 8)
    #32位静态链接库
    link_directories(${PROJECT_SOURCE_DIR}/Lib32)  
ENDIF( CMAKE_SIZEOF_VOID_P EQUAL 8)

link_libraries(glfw3.lib)
link_libraries(vulkan-1.lib)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin)   
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin) 

add_subdirectory(${PROJECT_SOURCE_DIR}/Base)
link_libraries(base)
add_subdirectory(${PROJECT_SOURCE_DIR}/Example/1_Triangle)
add_subdirectory(${PROJECT_SOURCE_DIR}/Example/2_TestBase)
add_subdirectory(${PROJECT_SOURCE_DIR}/Example/3_SkRender)
