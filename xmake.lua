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
add_requires("spirv-reflect")

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
    "cubes",
    "grass",
    "helloTriangle",
    "helloTriangleMesh",
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

            local slang_files = os.files("samples/" .. name .. "/*.slang")
            if #slang_files == 0 then
                return
            end

            local outdir = path.absolute(target:targetdir())
            os.mkdir(outdir)

            for _, f in ipairs(slang_files) do
                local outfile = path.join(outdir, path.basename(f) .. ".spv")

                local args = { 
                    '-matrix-layout-column-major',
                    -- '-fspv-reflect', 
                    -- '-fvk-use-scalar-layout', 
                    -- '-preserve-params', 
                    "-profile", 
                    "spirv_1_4",
                    -- "-capability", 
                    -- "SPV_EXT_descriptor_indexing",
                    -- "-capability", 
                    -- "SPV_KHR_fragment_shader_barycentric",
                    "-capability", 
                    "meshshading",
                    -- "-capability", 
                    -- "vk_mem_model", 
                    "-target", "spirv",
                    "-o", outfile, f}

                local proc = process.openv("slangc", args)
                local ok, status = proc:wait()
                proc:close()

                if ok < 0 then
                    print(string.format("warning: failed to compile %s", f))
                else
                    print(string.format("compiled %s -> %s", f, path.relative(outfile, outdir)))
                end
            end
        end)
end