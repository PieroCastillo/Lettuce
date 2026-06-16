rule("slang")
    set_extensions(".slang")

    on_buildcmd_file(function(target, batchcmds, sourcefile, opt)
        if sourcefile:endswith(".part.slang") then
            return
        end

        local outdir = target:targetdir()
        local relativedir = path.directory(path.relative(sourcefile, os.projectdir()))
        local filename = path.basename(sourcefile)

        local shaderoutdir = path.join(outdir, relativedir)
        local depdir = path.join(target:autogendir(), "rules", "slang", relativedir)

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
            "-capability",
            "SPV_EXT_descriptor_indexing",
            "-capability",
            "SPV_KHR_vulkan_memory_model",

            "-depfile",
            depfile,
            "-o",
            outfile,
            sourcefile
        }

        batchcmds:vrunv("slangc", args)

        batchcmds:add_depfiles(sourcefile, depfile)
        batchcmds:set_depmtime(os.mtime(outfile))
        batchcmds:set_depcache(dependfile)
    end)