cmake_minimum_required(VERSION 3.30)

project(LettuceV2 LANGUAGES CXX VERSION 0.0.1.0)
set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED COMPONENTS shaderc_combined )
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(volk CONFIG REQUIRED)
find_package(glfw3 REQUIRED)

# Dividir la versión en sus componentes
string(REPLACE "." ";" VERSION_LIST ${PROJECT_VERSION})

# Obtener los componentes de la versión
list(GET VERSION_LIST 0 VARIANT_VERSION)
list(GET VERSION_LIST 1 MAJOR_VERSION)
list(GET VERSION_LIST 2 MINOR_VERSION)
list(GET VERSION_LIST 3 PATCH_VERSION)

# Agregar definición de macros
add_definitions(-DVARIANT_VERSION=${VARIANT_VERSION})
add_definitions(-DMAJOR_VERSION=${MAJOR_VERSION})
add_definitions(-DMINOR_VERSION=${MINOR_VERSION})
add_definitions(-DPATCH_VERSION=${PATCH_VERSION})

file(GLOB LETTUCE_SOURCES Lettuce/Core/*.cpp Lettuce/core/Compilers/*.cpp)

add_library(Lettuce)
target_sources(Lettuce PRIVATE ${LETTUCE_SOURCES})
target_include_directories(Lettuce PUBLIC include)
# add_executable(ClearScreenSample
#         samples/ClearScreenSample/app.cpp)
# add_executable(TriangleSample
#         samples/TriangleSample/app.cpp)
# add_executable(VertexSquare
#         samples/VertexSquare/app.cpp)
target_link_libraries(Lettuce  PRIVATE volk::volk Vulkan::shaderc_combined GPUOpen::VulkanMemoryAllocator)
# target_link_libraries(ClearScreenSample PRIVATE Lettuce glfw)
# target_link_libraries(TriangleSample Lettuce glfw)
# target_link_libraries(VertexSquare Lettuce glfw)