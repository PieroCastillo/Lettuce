set_languages("c++23")

includes("rules/*.lua")
add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})

option("san")
    set_default("none")
    set_values("none", "asan", "tsan", "ubsan")
    set_showmenu(true)
option_end()

if is_mode("debug") then
    local san = get_config("san")

    if san == "asan" then
        set_policy("build.sanitizer.address", true)
    elseif san == "tsan" then
        set_policy("build.sanitizer.thread", true)
    elseif san == "ubsan" then
        set_policy("build.sanitizer.undefined", true)
    end
end

add_requires("vulkansdk")
add_requires("vulkan-memory-allocator")
add_requires("volk")
add_requires("glfw")
add_requires("glm")
add_requires("fastgltf")
add_requires("imgui")
add_requires("ktx", { configs={ vulkan=true, ktx2=true, decoder=true}})
add_requires("meshoptimizer")

local v = ("0.0.1.0"):split("%.")
add_defines("VARIANT_VERSION=" .. (v[1] or 0),
            "MAJOR_VERSION="   .. (v[2] or 0),
            "MINOR_VERSION="   .. (v[3] or 0),
            "PATCH_VERSION="   .. (v[4] or 0))

if is_os("windows") then
    add_links("user32", "gdi32", "kernel32")
    add_defines("VK_USE_PLATFORM_WIN32_KHR", "_WIN32")
else
    add_defines("VK_USE_PLATFORM_WAYLAND_KHR")
end

target("Lettuce")
    set_kind("shared")
    add_includedirs("include/")
    add_headerfiles("include/Lettuce/**.hpp")
    add_files("src/**.cpp")
    add_packages("volk", "glfw", "ktx", "glm", "fastgltf", "meshoptimizer", "vulkan-memory-allocator")
    add_rules("utils.symbols.export_all", {export_classes = true})
    local slangFiles = os.files("src/**.slang")
    if #slangFiles > 0 then
        add_files(slangFiles, {rule = "slang"})
    end

local samples = {
    "asyncRecord",
    "helloTriangle",
    "helloTriangleMesh",
    "indirectDrawing",
    "loadModel",
    "movingSquares",
    "sceneTreeDemo",
    "textureLoad",
    "uniform",
}

for _, name in ipairs(samples) do 
    target(name)
        set_kind("binary")
        add_deps("Lettuce")
        add_includedirs("include")
        add_files("samples/" .. name .. "/app.cpp")
        add_packages("volk", "glfw", "glm", "imgui", "fastgltf", "slang", "meshoptimizer")
        local slangFiles = os.files("samples/" .. name .. "/**.slang")
        local hlslFiles = os.files("samples/" .. name .. "/**.hlsl")
        if #slangFiles > 0 then
            add_files(slangFiles, {rule = "slang"})
        end
        if #hlslFiles > 0 then
            add_files(hlslFiles, {rule = "hlsl"})
        end
end