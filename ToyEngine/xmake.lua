set_project("ToyEngine")

set_arch("x64")
add_rules("mode.debug", "mode.release")
set_languages("c++20")

add_requires("glfw", "fmt")
add_includedirs("$(projectdir)/Source/", os.getenv("VK_SDK_PATH").."/Include/")
add_linkdirs(os.getenv("VK_SDK_PATH").."/Lib/")
add_links("vulkan-1")

target("Engine")
    set_kind("binary")
    add_headerfiles("$(projectdir)/**/*.h", "$(projectdir)/**/*.hpp")
    add_files("$(projectdir)/**/*.cpp")
    add_packages("glfw", "fmt")
    --add_defines("GUNGNIR_STATIC")
