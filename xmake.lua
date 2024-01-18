add_rules("mode.debug", "mode.release")

set_languages("c17")

add_requires("unity_test ^2")

target("test-mscarr")
    set_default(false)
    set_kind("binary")
    add_files("tests/msc_arr/*.c")
    add_tests("default")
    add_includedirs("src")
    add_packages("unity_test")
