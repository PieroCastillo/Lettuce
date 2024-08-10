cmake_minimum_required(VERSION 3.30)

#required to import std feature
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")

project(LettuceV2 LANGUAGES CXX VERSION 0.0.1.0)
#enables C++ modules feature
# make sure c++ 20 is set
set(CMAKE_CXX_STANDARD 23)# turn on the dynamic depends for ninja
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)
# turn on the experimental API
set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API  "2182bf5c-ef0d-489a-91da-49dbc3090d2a")
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  include(gcc_modules.cmake)
endif()

#enanles import std support
#set(CMAKE_CXX_MODULE_STD 1)


# find_package(Vulkan REQUIRED)
find_package(Vulkan REQUIRED COMPONENTS shaderc_combined)
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

# Buscar todos los archivos con extensión .ixx dentro de la carpeta Lettuce
file(GLOB LETTUCE_SOURCES Lettuce/*.ixx Lettuce/Compilers/*.ixx)
file(GLOB LETTUCE3D_SOURCES Lettuce3D/*.ixx)

add_library(Lettuce SHARED)
target_sources(Lettuce
        PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES FILES
        ${LETTUCE_SOURCES}
)
add_library(Lettuce3D SHARED)
target_sources(Lettuce3D
        PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES FILES
        ${LETTUCE3D_SOURCES}
)

target_include_directories(Lettuce PRIVATE Lettuce)
# target_compile_features(Lettuce PRIVATE cxx_std_23
#                                 INTERFACE cxx_std_20)
add_executable(ClearScreenSample
        samples/ClearScreenSample/app.cpp)
# add_executable(TriangleSample
#         samples/TriangleSample/app.cpp)
# add_executable(VertexSquare
#         samples/VertexSquare/app.cpp)
target_link_libraries(Lettuce  PRIVATE volk::volk Vulkan::shaderc_combined GPUOpen::VulkanMemoryAllocator)
# target_link_libraries(Lettuce3D  PRIVATE Lettuce volk::volk_headers Vulkan::shaderc_combined)
target_link_libraries(ClearScreenSample PRIVATE Lettuce glfw)
# target_link_libraries(TriangleSample Lettuce glfw)
# target_link_libraries(VertexSquare Lettuce glfw)