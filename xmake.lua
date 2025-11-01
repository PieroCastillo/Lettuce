set_languages("c++23")

add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})

add_requires("vulkansdk")
add_requires("volk")
add_requires("glfw")
add_requires("glm")
add_requires("fastgltf")
add_requires("imgui")
add_requires("ktx", { configs={ vulkan=true, ktx2=true, decoder=true} })
add_requires("slang", { configs={ slangc=true }})
add_requires("spirv-reflect", { configs = { shared=true }})

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
    add_headerfiles("include/Lettuce/**.hpp", "include/Lettuce/**.inl")
    add_files("src/Core/**.cpp")
    add_packages("volk", "glfw", "ktx", "glm", "fastgltf", "slang", "spirv-reflect")
    add_rules("utils.symbols.export_all", {export_classes = true})
    
local samples = {
    "helloTriangle",
}

for _, name in ipairs(samples) do 
    target(name)
        set_kind("binary")
        add_deps("Lettuce")
        add_includedirs("include")
        add_files("samples/" .. name .. "/app.cpp")
        add_packages("volk", "glfw", "glm", "imgui", "fastgltf", "slang")
        
        after_build(function (target)
            import("core.base.process")
            -- local slangc = find_tool("slangc") or find_tool("slangc.exe")
            -- if not slangc then
            --     print("warning: 'slangc' not found in PATH; skipping .slang compilation for target '" .. target:name() .. "'")
            --     return
            -- end

            -- search for .slang files under the project tree
            local slang_files = os.files("**/*.slang")
            if #slang_files == 0 then
                return
            end

            -- Use target's binary directory as output
            local outdir = path.absolute(target:targetdir())
            os.mkdir(outdir)

            -- Compile each .slang file to SPIR-V
            for _, f in ipairs(slang_files) do
                local outfile = path.join(outdir, path.basename(f) .. ".spv")
                -- Try compiling to SPIR-V with Vulkan target
                local ok = process.set_runargs({ 
                    "slangc",
                    f,
                    "-profile", "glsl_450",
                    "-target", "spirv",      -- Output format
                    "-o", outfile,           -- Output file
                })
                if ok ~= 0 then
                    print(string.format("warning: failed to compile %s", f))
                else
                    print(string.format("compiled %s -> %s", f, outfile))
                end
            end
        end)
end