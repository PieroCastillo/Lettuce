set_languages("c++23")
set_runtimes("MD") 

add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})

add_requires("vulkansdk")
add_requires("shaderc", {
    configs = {
        binaryonly = false,
        shared = false     
    }
})
add_requires("volk")
add_requires("vulkan-memory-allocator")
add_requires("glfw")
add_requires("glm")
add_requires("fastgltf")
add_requires("directxshadercompiler")
add_requires("imgui")
add_requires("taskflow")

local v = ("0.0.1.0"):split("%.")
add_defines("VARIANT_VERSION=" .. (v[1] or 0),
            "MAJOR_VERSION="   .. (v[2] or 0),
            "MINOR_VERSION="   .. (v[3] or 0),
            "PATCH_VERSION="   .. (v[4] or 0),
            "VOLK_STATIC_DEFINES")

if is_os("windows") then
    add_links("user32", "gdi32") 
    add_defines("VK_USE_PLATFORM_WIN32_KHR", "_WIN32")
else
    add_defines("VK_USE_PLATFORM_WAYLAND_KHR")
end

target("Lettuce")
    set_kind("static")
    add_includedirs("include", "external/stb" )
    add_headerfiles("include/Lettuce/**.hpp")
    add_files("src/Core/**.cpp")
    add_packages("vulkansdk", "volk", "vulkan-memory-allocator", "glfw", "glm", "fastgltf", "shaderc", "directxshadercompiler", {public = true})
    
local samples = {
        "helloTriangle",
--     "ClearScreenSample",
--     "DonutSample",
--     "GeometryIndirectSample",
--     "LoadModel",
--     "SampleX2D",
--     "ShadowMaps",
--     "SkyBox",
}

for _, name in ipairs(samples) do 
    target(name)
        set_kind("binary")
        -- add_includedirs("samples/SampleFramework", "external/stb","include")
        add_files("samples/" .. name .. "/app.cpp")
        add_deps("Lettuce")
        add_packages("volk", "glfw", "glm", "imgui")
end