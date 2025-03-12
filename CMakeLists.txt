cmake_minimum_required(VERSION 3.30)

project(LettuceV2 LANGUAGES CXX VERSION 0.0.1.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_SCAN_FOR_MODULES false)

find_package(Vulkan REQUIRED COMPONENTS shaderc_combined dxc)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(volk CONFIG REQUIRED)
find_package(glfw3 REQUIRED)
find_package(fastgltf CONFIG REQUIRED)

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
                        Lettuce/Foundation/*.cpp)
                        # Lettuce/X2D/*.cpp 
                        # Lettuce/X2D/Geometries/*.cpp 
                        # Lettuce/X2D/Lights/*.cpp  
                        # Lettuce/X2D/Materials/*.cpp )

add_library(Lettuce)
target_sources(Lettuce PRIVATE ${LETTUCE_SOURCES})
target_include_directories(Lettuce PUBLIC include external/stb)
# add_executable(ClearScreenSample
#         samples/ClearScreenSample/app.cpp)
add_executable(DonutSample
        samples/DonutSample/app.cpp)
add_executable(SampleX2D
        samples/SampleX2D/app.cpp)
add_executable(tester
                samples/tester/app.cpp)
add_executable(loadModel
                samples/LoadModel/app.cpp)
add_executable(geometry
                samples/GeometryIndirectSample/app.cpp)

message(STATUS "all ok")

# Definir el directorio de origen
set(SRC_DIR "${CMAKE_SOURCE_DIR}/samples/assets")

# Crear un comando personalizado para copiar los archivos
add_custom_command(
    TARGET geometry  # Reemplaza con el nombre de tu target
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${SRC_DIR}
            "$<TARGET_FILE_DIR:geometry>/assets"
    COMMENT "Copiando assets a $<TARGET_FILE_DIR:geometry>/assets"
)

target_include_directories(DonutSample PUBLIC samples/SampleFramework)
target_include_directories(loadModel PUBLIC samples/SampleFramework)
target_include_directories(geometry PUBLIC samples/SampleFramework)
# add_executable(TriangleSample
#         samples/TriangleSample/app.cpp)
# add_executable(VertexSquare
#         samples/VertexSquare/app.cpp)

if(WIN32)
        target_compile_definitions(Lettuce PRIVATE VK_USE_PLATFORM_WIN32_KHR)
        target_compile_definitions(DonutSample PRIVATE VK_USE_PLATFORM_WIN32_KHR)
        target_compile_definitions(SampleX2D PRIVATE VK_USE_PLATFORM_WIN32_KHR)
        target_compile_definitions(tester PRIVATE VK_USE_PLATFORM_WIN32_KHR)
        target_compile_definitions(loadModel PRIVATE VK_USE_PLATFORM_WIN32_KHR)
        target_compile_definitions(geometry PRIVATE VK_USE_PLATFORM_WIN32_KHR)
        set(VOLK_STATIC_DEFINES "VK_USE_PLATFORM_WIN32_KHR")
elseif()
        target_compile_definitions(Lettuce PRIVATE VK_USE_PLATFORM_WAYLAND_KHR)
        target_compile_definitions(DonutSample PRIVATE VK_USE_PLATFORM_WAYLAND_KHR)
        target_compile_definitions(SampleX2D PRIVATE VK_USE_PLATFORM_WAYLAND_KHR)
        target_compile_definitions(tester PRIVATE VK_USE_PLATFORM_WAYLAND_KHR)
        target_compile_definitions(loadModel PRIVATE VK_USE_PLATFORM_WAYLAND_KHR)
        target_compile_definitions(geometry PRIVATE VK_USE_PLATFORM_WAYLAND_KHR)
        set(VOLK_STATIC_DEFINES "VK_USE_PLATFORM_WAYLAND_KHR")
endif()
target_include_directories(Lettuce PRIVATE ${TINYGLTF_INCLUDE_DIRS})
target_link_libraries(Lettuce  PRIVATE volk::volk_headers Vulkan::shaderc_combined Vulkan::dxc_lib GPUOpen::VulkanMemoryAllocator fastgltf::fastgltf)
# target_link_libraries(ClearScreenSample PRIVATE Lettuce volk::volk_headers glfw)
target_link_libraries(DonutSample PRIVATE Lettuce volk::volk_headers glfw)
target_link_libraries(SampleX2D PRIVATE Lettuce volk::volk_headers glfw)
target_link_libraries(tester PRIVATE Lettuce volk::volk_headers glfw)
target_link_libraries(loadModel PRIVATE Lettuce volk::volk_headers glfw)
target_link_libraries(geometry PRIVATE Lettuce volk::volk_headers glfw)
# target_link_libraries(TriangleSample PRIVATE Lettuce volk::volk_headers glfw)
# target_link_libraries(TriangleSample Lettuce glfw)
# target_link_libraries(VertexSquare Lettuce glfw)