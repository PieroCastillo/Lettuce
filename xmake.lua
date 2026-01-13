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
    add_headerfiles("include/Lettuce/**.hpp")
    add_files("src/**.cpp")
    add_packages("volk", "glfw", "ktx", "glm", "fastgltf", "slang", "spirv-reflect")
    add_rules("utils.symbols.export_all", {export_classes = true})

local samples = {
    "grass",
    "helloTriangle",
    "helloTriangleMesh",
    "textureLoad",
    "uniform",
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

            local outdir = path.absolute(target:targetdir())
            os.mkdir(outdir)

            if #slang_files ~= 0 then
                for _, f in ipairs(slang_files) do
                    local outfile = path.join(outdir, path.basename(f) .. ".spv")

                    local args = { 
                        '-matrix-layout-column-major',
                        -- '-fspv-reflect', 
                        -- '-fvk-use-scalar-layout', 
                        -- '-preserve-params', 
                        "-verbose-paths",
                        "-profile", 
                        "spirv_1_6",
                        "-capability", 
                        "SPV_EXT_descriptor_indexing",
                        "-capability", 
                        "SPV_KHR_fragment_shader_barycentric",
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
            end

            local hlsl_files = os.files("samples/" .. name .. "/*.hlsl")

            local outdir = path.absolute(target:targetdir())
            os.mkdir(outdir)

            local dxc_path = "dxc"

            if #hlsl_files ~= 0 then
                for _, f in ipairs(hlsl_files) do
                    local outfile = path.join(outdir, path.basename(f) .. ".spv")
                    
                    local profile = ''
                    local target = ''
                    local additional_exts = ''
                    
                    if f:find('%.vert%.') then
                        profile = 'vs_6_1'
                    elseif f:find('%.frag%.') then
                        profile = 'ps_6_4'
                    elseif f:find('%.comp%.') then
                        profile = 'cs_6_1'
                    elseif f:find('%.geom%.') then
                        profile = 'gs_6_1'
                    elseif f:find('%.tesc%.') then
                        profile = 'hs_6_1'
                    elseif f:find('%.tese%.') then
                        profile = 'ds_6_1'
                    elseif f:find('%.rgen%.') or f:find('%.rchit%.') or f:find('%.rmiss%.') then
                        target = '-fspv-target-env=vulkan1.2'
                        profile = 'lib_6_3'
                    elseif f:find('%.mesh%.') then
                        target = '-fspv-target-env=vulkan1.2'
                        additional_exts = '-fspv-extension=SPV_EXT_mesh_shader'
                        profile = 'ms_6_6'
                    elseif f:find('%.task%.') then
                        target = '-fspv-target-env=vulkan1.2'
                        additional_exts = '-fspv-extension=SPV_EXT_mesh_shader'
                        profile = 'as_6_6'
                    end
                    
                    -- Extensión adicional para debugprintf
                    if f:find('debugprintf') then
                        additional_exts = '-fspv-extension=SPV_KHR_non_semantic_info'
                    end
                    
                    -- Construir argumentos
                    local args = {
                        '-spirv',
                        '-T', profile,
                        '-E', 'main',
                        '-fspv-extension=SPV_KHR_ray_tracing',
                        '-fspv-extension=SPV_KHR_multiview',
                        '-fspv-extension=SPV_KHR_shader_draw_parameters',
                        '-fspv-extension=SPV_EXT_descriptor_indexing',
                        '-fspv-extension=SPV_KHR_ray_query',
                        '-fspv-extension=SPV_KHR_fragment_shading_rate',
                    }
                    
                    if additional_exts ~= '' then
                        table.insert(args, additional_exts)
                    end
                    if target ~= '' then
                        table.insert(args, target)
                    end
                    
                    table.insert(args, f)
                    table.insert(args, '-Fo')
                    table.insert(args, outfile)
                    
                    print(string.format("Compiling %s", f))
                    
                    local proc = process.openv(dxc_path, args)
                    local ok, status = proc:wait()
                    proc:close()
                    
                    if ok < 0 then
                        print(string.format("warning: failed to compile %s", f))
                    else
                        print(string.format("compiled %s -> %s", f, path.relative(outfile, outdir)))
                    end
                end       
            end
        end)
end