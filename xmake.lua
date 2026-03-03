add_rules("plugin.compile_commands.autoupdate", {outputdir = "."})
add_rules("mode.debug", "mode.release")
set_languages("c++23")

add_requires("zig")
set_toolchains("@zig")

if is_mode("release") then
    set_optimize("aggressive")                 
    set_strip("all")                           
    set_policy("build.optimization.lto", true) 

    add_cxflags(
        "-march=native",
        "-Ofast",
        "-fno-unsafe-math-optimizations", 
        "-fno-finite-math-only",
        "-mllvm", 
        "-inline-threshold=1000", 
        "-mllvm",
        "-fno-stack-protector",                 
        "-fno-rtti",                            
        {force = true}
    )

    add_ldflags("-static", {force = true})
end

function apply_zig_system_patch()
    add_cxflags("-std=c++2b", "-fexperimental-library", "-target x86_64-windows-gnu", {force = true}) 
    add_ldflags("-target x86_64-windows-gnu", {force = true})

    on_load(function (target)
        import("core.project.project")
        local zig_pkg = target:pkg("zig") or project.required_package("zig")

        if zig_pkg then
            local zig_dir = zig_pkg:installdir()
            local lib_dir = path.join(zig_dir, "lib")

            target:add("sysincludedirs", path.join(lib_dir, "libcxx/include"))
            target:add("sysincludedirs", path.join(lib_dir, "libcxxabi/include"))
            
            local clang_res = path.join(lib_dir, "clang")
            if os.isdir(clang_res) then
                for _, dir in ipairs(os.dirs(path.join(clang_res, "*"))) do
                    target:add("sysincludedirs", path.join(dir, "include"))
                    break 
                end
            end

            target:add("sysincludedirs", path.join(lib_dir, "libc/include/x86_64-windows-gnu"))
            target:add("sysincludedirs", path.join(lib_dir, "libc/include/generic-mingw"))
            target:add("sysincludedirs", path.join(lib_dir, "libc/include/any-windows-any"))
        end

        target:add("defines", 
            "_LIBCPP_HAS_THREADS=1",
            "_LIBCPP_HAS_WIDE_CHARACTERS=1"
        )
    end)
end

target("pvz_emulator")
    set_kind("static")
    apply_zig_system_patch()
    add_files("object/*.cpp")
    add_files("system/*.cpp")
    add_files("system/plant/*.cpp")
    add_files("system/zombie/*.cpp")
    add_files("system/projectile/*.cpp")
    add_files("learning/*.cpp")
    add_files("world.cpp")
    add_includedirs(".", "lib", "system", "object", "learning", {public = true})

target("example")
    set_kind("binary")
    apply_zig_system_patch()
    add_files("example/main.cpp")
    add_deps("pvz_emulator")

target("catch2")
    set_kind("static")
    apply_zig_system_patch()
    add_files("lib/catch2/catch_amalgamated.cpp")
    add_includedirs("lib/catch2", {public = true})
    add_cxflags("-fno-fast-math", {force = true})

target("tests")
    set_kind("binary")
    apply_zig_system_patch()
    add_files("test/*.cpp")
    add_includedirs("lib/catch2")
    add_deps("pvz_emulator", "catch2")
    set_runargs("-r", "compact", "--durations", "yes")