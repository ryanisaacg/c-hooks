cmake_minimum_required(VERSION 2.4.0)
project (arcade-physics)
set(CMAKE_BUILD_TYPE Debug)
cmake_policy(SET CMP0003 NEW)
cmake_policy(SET CMP0015 NEW)

add_subdirectory(arcade-engine)
add_subdirectory(json)

include_directories(include arcade-physics/include arcade-physics/data-structures/include json $ENV{SDL2_INCLUDE})
link_directories($ENV{SDL2_LIB})
file(GLOB SOURCES src/*.c)

add_executable(hooks ${SOURCES})
set_property(TARGET hooks PROPERTY C_STANDARD 99)
set_target_properties(hooks PROPERTIES LINKER_LANGUAGE C)
target_link_libraries(hooks m arcade-engine pthread parson)
