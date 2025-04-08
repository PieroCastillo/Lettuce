set_languages("c++20")

add_requires("vulkansdk")
add_requires("shaderc")
add_requires("volk")
add_requires("vulkan-memory-allocator")
add_requires("glfw")
add_requires("fastgltf")
add_requires("directxshadercompiler")

local v = ("0.0.1.0"):split("%.")
add_defines("VARIANT_VERSION=" .. (v[1] or 0),
            "MAJOR_VERSION="   .. (v[2] or 0),
            "MINOR_VERSION="   .. (v[3] or 0),
            "PATCH_VERSION="   .. (v[4] or 0))

if is_os("windows") then
    add_links("user32", "gdi32") 
    add_defines("VOLK_STATIC_DEFINES", "VK_USE_PLATFORM_WIN32_KHR", "_WIN32")
else
    add_defines("VOLK_STATIC_DEFINES", "VK_USE_PLATFORM_WAYLAND_KHR")
end

target("Lettuce")
    set_kind("static")
    add_includedirs("include")
    add_headerfiles("include/Lettuce/Core/*.hpp")
    add_headerfiles("include/Lettuce/Core/Compilers/*.hpp")
    add_headerfiles("include/Lettuce/Foundation/*.hpp")
    add_headerfiles("include/Lettuce/X2D/*.hpp")
    add_headerfiles("include/Lettuce/X3D/*.hpp")
    add_headerfiles("include/Lettuce/*.hpp")
    add_files("Lettuce/Core/*.cpp")
    add_files("Lettuce/Foundation/*.cpp")
    add_files("Lettuce/X3D/*.cpp")
    add_files("Lettuce/Core/Compilers/*.cpp")
    add_files("Lettuce/X3D/Geometries/*.cpp")
    -- add_files("Lettuce/X3D/Materials/*.cpp")
    add_packages("vulkansdk", "volk", "vulkan-memory-allocator", "glfw", "fastgltf", "shaderc", "directxshadercompiler")