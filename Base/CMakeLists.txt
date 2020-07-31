file(GLOB BASE_SRC "*.cpp" "*.c")
file(GLOB BASE_HEADERS "*.hpp" "*.h")

include_directories(${SK_Project}/include)
add_library(base STATIC ${BASE_SRC})
