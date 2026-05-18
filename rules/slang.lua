rule("slang")

    set_extensions(".slang")

    on_buildcmd_file(function(target, batchcmds, sourcefile, opt)
        if sourcefile:endswith(".part.slang") then
            return
        end

        local outdir = target:targetdir()
        local filename = path.basename(sourcefile)
        local outfile = path.join(outdir, filename .. ".spv")
        local depdir = path.join(target:autogendir(), "rules", "slang")
        local depfile = path.join(depdir, filename .. ".d")

        batchcmds:show_progress(
            opt.progress,
            "${color.build.object}compiling.slang %s",
            sourcefile
        )

        batchcmds:mkdir(outdir)
        batchcmds:mkdir(depdir)

        local args = {
            "-matrix-layout-column-major",
            "-target", "spirv",
            "-profile", "spirv_1_6",
            "-fvk-use-entrypoint-name",
            "-fvk-use-scalar-layout",
            "-capability",
            "SPV_EXT_descriptor_indexing",

            "-depfile",
            depfile,
            "-o",
            outfile,
            sourcefile
        }

        batchcmds:vrunv("slangc", args)

        batchcmds:add_depfiles(depfile)
        batchcmds:set_depmtime(os.mtime(outfile))
        batchcmds:set_depcache(
            target:dependfile(outfile)
        )
    end)