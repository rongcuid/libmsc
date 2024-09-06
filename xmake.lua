set_project("libmsc")
set_xmakever("2.8.2")
set_version("0.0.1", {build = "%Y%m%d", soname = true})

set_warnings("all", "error")

set_languages("c17")

if is_plat("wasm") then
    add_requires("emscripten")
    set_toolchains("emcc@emscripten")
end

add_rules("mode.release", "mode.debug", "mode.profile", "mode.coverage", "mode.valgrind", "mode.asan", "mode.tsan", "mode.ubsan")

target("msc")
    set_kind("static")
    add_files("src/msc/**/*.c")