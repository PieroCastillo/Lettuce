# CMakeList.txt: archivo del proyecto de CMake de nivel superior, establezca la configuración global
# e incluya los subproyectos aquí.
cmake_minimum_required (VERSION 3.26)

# Enable Hot Reload for MSVC compilers if supported.
# if (POLICY CMP0141)
#   cmake_policy(SET CMP0141 NEW)
#   set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
# endif()

project (Lettuce VERSION 0.1.0 DESCRIPTION "Vulkan Render Library" LANGUAGES CXX)

message(STATUS $ENV{VCPKG_CMAKE})
include(C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake)

find_package(Vulkan REQUIRED)
find_package(SDL2 CONFIG REQUIRED)

add_subdirectory (LettuceCore)
add_subdirectory (TriangleSample)