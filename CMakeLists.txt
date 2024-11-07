cmake_minimum_required(VERSION 3.30)

project(LettuceV2 LANGUAGES CXX VERSION 0.0.1.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_SCAN_FOR_MODULES false)

find_package(Vulkan REQUIRED COMPONENTS shaderc_combined )
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(volk CONFIG REQUIRED)
find_package(glfw3 REQUIRED)
find_path(TINYGLTF_INCLUDE_DIRS "tiny_gltf.h")

# Dividir la versión en sus componentes
string(REPLACE "." ";" VERSION_LIST ${PROJECT_VERSION})

# Obtener los componentes de la versión
list(GET VERSION_LIST 0 VARIANT_VERSION)
list(GET VERSION_LIST 1 MAJOR_VERSION)
list(GET VERSION_LIST 2 MINOR_VERSION)
list(GET VERSION_LIST 3 PATCH_VERSION)

# Agregar definición de macros
add_compile_definitions(VARIANT_VERSION=${VARIANT_VERSION})
add_compile_definitions(MAJOR_VERSION=${MAJOR_VERSION})
add_compile_definitions(MINOR_VERSION=${MINOR_VERSION})
add_compile_definitions(PATCH_VERSION=${PATCH_VERSION})

file(GLOB LETTUCE_SOURCES Lettuce/Core/*.cpp 
                        Lettuce/Core/Compilers/*.cpp 
                        Lettuce/X3D/*.cpp 
                        Lettuce/X3D/Materials/*.cpp
                        Lettuce/X2D/*.cpp 
                        Lettuce/X2D/Geometries/*.cpp 
                        Lettuce/X2D/Lights/*.cpp  
                        Lettuce/X2D/Materials/*.cpp )

add_library(Lettuce)
target_sources(Lettuce PRIVATE ${LETTUCE_SOURCES})
target_include_directories(Lettuce PUBLIC include)
# add_executable(ClearScreenSample
#         samples/ClearScreenSample/app.cpp)
add_executable(DonutSample
        samples/DonutSample/app.cpp)
add_executable(SampleX2D
        samples/SampleX2D/app.cpp)
add_executable(tester
                samples/tester/app.cpp)
# add_executable(TriangleSample
#         samples/TriangleSample/app.cpp)
# add_executable(VertexSquare
#         samples/VertexSquare/app.cpp)

if(WIN32)
        target_compile_definitions(Lettuce PRIVATE VK_USE_PLATFORM_WIN32_KHR)
        target_compile_definitions(DonutSample PRIVATE VK_USE_PLATFORM_WIN32_KHR)
        target_compile_definitions(SampleX2D PRIVATE VK_USE_PLATFORM_WIN32_KHR)
        target_compile_definitions(tester PRIVATE VK_USE_PLATFORM_WIN32_KHR)
        set(VOLK_STATIC_DEFINES "VK_USE_PLATFORM_WIN32_KHR")
elseif()
        target_compile_definitions(Lettuce PRIVATE VK_USE_PLATFORM_WAYLAND_KHR)
        target_compile_definitions(DonutSample PRIVATE VK_USE_PLATFORM_WAYLAND_KHR)
        target_compile_definitions(SampleX2D PRIVATE VK_USE_PLATFORM_WAYLAND_KHR)
        target_compile_definitions(tester PRIVATE VK_USE_PLATFORM_WAYLAND_KHR)
        set(VOLK_STATIC_DEFINES "VK_USE_PLATFORM_WAYLAND_KHR")
endif()
target_include_directories(Lettuce PRIVATE ${TINYGLTF_INCLUDE_DIRS})
target_link_libraries(Lettuce  PRIVATE volk::volk_headers Vulkan::shaderc_combined GPUOpen::VulkanMemoryAllocator)
# target_link_libraries(ClearScreenSample PRIVATE Lettuce volk::volk_headers glfw)
target_link_libraries(DonutSample PRIVATE Lettuce volk::volk_headers glfw)
target_link_libraries(SampleX2D PRIVATE Lettuce volk::volk_headers glfw)
target_link_libraries(tester PRIVATE Lettuce volk::volk_headers glfw)
# target_link_libraries(TriangleSample PRIVATE Lettuce volk::volk_headers glfw)
# target_link_libraries(TriangleSample Lettuce glfw)
# target_link_libraries(VertexSquare Lettuce glfw)