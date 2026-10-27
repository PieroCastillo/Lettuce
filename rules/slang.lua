rule("slang")
    set_extensions(".slang")

    on_buildcmd_file(function(target, batchcmds, sourcefile, opt)
        if sourcefile:endswith(".part.slang") then
            return
        end

        local filename = path.basename(sourcefile)
        local relativedir = path.directory(sourcefile)

        local shaderoutdir = path.join(target:targetdir(), "shaders", path.basename(relativedir))
        local depdir = path.join(target:autogendir(), "rules", "slang", "deps")

        local outfile = path.join(shaderoutdir, filename .. ".spv")
        local depfile = path.join(depdir, filename .. ".d")
        local dependfile = target:dependfile(outfile)

        batchcmds:show_progress(
            opt.progress,
            "${color.build.object}compiling.slang %s",
            sourcefile
        )

        batchcmds:mkdir(shaderoutdir)
        batchcmds:mkdir(depdir)

        local args = {
            "-matrix-layout-column-major",
            "-target", "spirv",
            "-profile", "spirv_1_6",
            "-fvk-use-entrypoint-name",
            "-fvk-use-scalar-layout",
            "-fvk-use-c-layout",
            "-capability", "SPV_EXT_descriptor_indexing",
            "-capability", "SPV_KHR_non_semantic_info",
            "-capability", "SPV_KHR_vulkan_memory_model",
            "-capability", "SPV_GOOGLE_user_type",
            "-depfile",
            depfile,
            "-o",
            outfile,
            sourcefile
        }

        if is_mode("debug") then
            table.join2(args, {
                "-fspv-reflect",
            })
        else
            table.join2(args, {
                "-O3"
            })
        end

        batchcmds:vrunv("slangc", args)

        batchcmds:add_depfiles(sourcefile, depfile)
        batchcmds:set_depmtime(os.mtime(outfile))
        batchcmds:set_depcache(dependfile)
    end)