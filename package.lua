package("lettuce")
    set_sourcedir(os.scriptdir())

    on_install(function(package)
        import("package.tools.xmake").install(package)
    end)
package_end()