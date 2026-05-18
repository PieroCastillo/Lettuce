rule("hlsl")
    set_extensions(".hlsl")

    on_buildcmd_file(function(target, batchcmds, sourcefile, opt)

        local outfile = path.join(
            target:targetdir(),
            path.basename(sourcefile) .. ".spv"
        )

        local profile = nil
        local extra = {}

        if sourcefile:find("%.vert%.") then
            profile = "vs_6_1"

        elseif sourcefile:find("%.frag%.") then
            profile = "ps_6_4"

        elseif sourcefile:find("%.comp%.") then
            profile = "cs_6_1"

        elseif sourcefile:find("%.geom%.") then
            profile = "gs_6_1"

        elseif sourcefile:find("%.tesc%.") then
            profile = "hs_6_1"

        elseif sourcefile:find("%.tese%.") then
            profile = "ds_6_1"

        elseif sourcefile:find("%.mesh%.") then
            profile = "ms_6_6"

            table.join2(extra, {
                "-fspv-target-env=vulkan1.2",
                "-fspv-extension=SPV_EXT_mesh_shader"
            })

        elseif sourcefile:find("%.task%.") then
            profile = "as_6_6"

            table.join2(extra, {
                "-fspv-target-env=vulkan1.2",
                "-fspv-extension=SPV_EXT_mesh_shader"
            })

        elseif sourcefile:find("%.rgen%.")
            or sourcefile:find("%.rchit%.")
            or sourcefile:find("%.rmiss%.") then

            profile = "lib_6_3"

            table.insert(extra,
                "-fspv-target-env=vulkan1.2")
        end

        if not profile then
            raise("Unknown shader type: %s", sourcefile)
        end

        batchcmds:show_progress(opt.progress,
            "${color.build.object}compiling.hlsl %s",
            sourcefile)

        batchcmds:mkdir(target:targetdir())

        local args = {
            "-spirv",
            "-T", profile,
            "-E", "main",

            "-fspv-extension=SPV_KHR_ray_tracing",
            "-fspv-extension=SPV_KHR_multiview",
            "-fspv-extension=SPV_KHR_shader_draw_parameters",
            "-fspv-extension=SPV_EXT_descriptor_indexing",
            "-fspv-extension=SPV_KHR_ray_query",
            "-fspv-extension=SPV_KHR_fragment_shading_rate",
        }

        table.join2(args, extra)

        table.join2(args, {
            sourcefile,
            "-Fo", outfile
        })

        batchcmds:vrunv("dxc", args)

        -- incremental tracking
        batchcmds:add_depfiles(sourcefile)
        batchcmds:set_depmtime(os.mtime(outfile))
        batchcmds:set_depcache(target:dependfile(outfile))
    end)