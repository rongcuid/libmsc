add_rules("mode.debug", "mode.release")

set_languages("c17")

add_requires("unity_test ^2")

target("test-msca")
    set_kind("binary")
    add_files("tests/msca/*.c")
    add_includedirs("src")
    add_packages("unity_test")
