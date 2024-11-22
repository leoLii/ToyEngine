set_project("ToyEngine")

set_arch("x64")
add_rules("mode.debug", "mode.release")
set_languages("c++20")

add_requires("glfw", "spdlog", "boost")
add_includedirs("$(projectdir)/Source/", 
                "$(projectdir)/ThirdParty/", 
                os.getenv("VK_SDK_PATH").."/Include/", 
                os.getenv("ASSIMP_PATH").."/include/",
                os.getenv("KTX_PATH").."/include")
add_linkdirs(os.getenv("VK_SDK_PATH").."/Lib/", os.getenv("ASSIMP_PATH").."/lib/x64", os.getenv("KTX_PATH").."/lib")
add_rpathdirs(os.getenv("VK_SDK_PATH").."/Bin/", os.getenv("ASSIMP_PATH").."/bin/x64", os.getenv("KTX_PATH").."/bin")
add_links("vulkan-1", "shaderc_shared", "assimp-vc143-mt", "ktx.lib")

target("Engine")
    set_kind("binary")
    add_headerfiles("$(projectdir)/**/*.h", "$(projectdir)/**/*.hpp")
    add_files("$(projectdir)/**/*.cpp")
    add_packages("glfw", "spdlog", "boost")
    after_build(function (target)
        os.cp(os.getenv("ASSIMP_PATH").."/bin/x64/assimp-vc143-mt.dll", path.join(target:targetdir(), "assimp-vc143-mt.dll"))
        os.cp(os.getenv("KTX_PATH").."/bin/ktx.dll", path.join(target:targetdir(), "ktx.dll"))
    end)
    --add_defines("GUNGNIR_STATIC")