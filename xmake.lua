set_project("ToyEngine")

set_arch("x64")
add_rules("mode.debug", "mode.release")
set_languages("c++20")

add_requires("glfw", "spdlog")
add_includedirs("$(projectdir)/Source/", os.getenv("VK_SDK_PATH").."/Include/")
add_linkdirs(os.getenv("VK_SDK_PATH").."/Lib/")
add_rpathdirs(os.getenv("VK_SDK_PATH").."/Bin/")
add_links("vulkan-1", "shaderc_shared")

target("Engine")
    set_kind("binary")
    add_headerfiles("$(projectdir)/**/*.h", "$(projectdir)/**/*.hpp")
    add_files("$(projectdir)/**/*.cpp")
    add_packages("glfw", "spdlog")
    --add_defines("GUNGNIR_STATIC")