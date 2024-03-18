cmake_minimum_required(VERSION 3.25)
project(LettuceV2 VERSION 0.0.1.0)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "2182bf5c-ef0d-489a-91da-49dbc3090d2a")
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)
set(CMAKE_CXX_EXTENSIONS Off)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
#modules usage
#add_module_library(Hello moduleF.ixx)
#add_executable(Main main.cpp)
#target_link_libraries(Main Hello)

#include(modules.cmake)

include("C:/dev/vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake")

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
#if(GLFW_FOUND) # Works with some other methods
#    message(STATUS "GLFW3_INCLUDE_PATH=${GLFW3_INCLUDE_PATH}")
#    message(STATUS "GLFW_LIBRARIES=${GLFW_LIBRARIES}")
#else( GLFW_FOUND )
#    message(FATAL_ERROR "GLFW3 not found")
#endif( GLFW_FOUND )

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
file(GLOB LETTUCE_SOURCES Lettuce/*.ixx)

add_library(Lettuce)
target_sources(Lettuce
        PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES FILES
        ${LETTUCE_SOURCES}
)
add_executable(ClearScreenSample
        ClearScreenSample/app.cpp)
target_link_libraries(Lettuce Vulkan::Vulkan)
target_link_libraries(ClearScreenSample Lettuce glfw)