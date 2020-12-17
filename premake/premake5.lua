workspace "Sender"

architecture "x86"
location "../"
staticruntime "off"

configurations{
	"Debug",
	"Release",
}

platforms{
	"x86",
}

os.execute("mklink /J ..\\android\\app\\jni\\SDL 				..\\vendor\\SDL")
os.execute("mklink /J ..\\android\\app\\jni\\SDL_image 			..\\vendor\\SDL_image")
os.execute("mklink /J ..\\android\\app\\jni\\SDL_ttf 			..\\vendor\\SDL_ttf")
os.execute("mklink /J ..\\android\\app\\jni\\SDL_mixer 			..\\vendor\\SDL_mixer")
os.execute("mklink /J ..\\android\\app\\jni\\SDL_net 			..\\vendor\\SDL_net")
os.execute("mkdir ..\\android\\app\\src\\main\\assets")
os.execute("mklink /J ..\\android\\app\\src\\main\\assets\\res 	..\\res")

project "Sender"
	location "../"
	language "C++"
	cppdialect "C++17"
	kind "ConsoleApp"

	targetdir "../bin/%{cfg.buildcfg}_%{cfg.platform}"
	objdir "../bin/obj/%{cfg.buildcfg}_%{cfg.platform}"

    includedirs{
        "../vendor/SDL/include",
        "../vendor/SDL_image/include",
        "../vendor/SDL_ttf/include",
        "../vendor/SDL_mixer/include",
        "../vendor/SDL_net/include",
        "../vendor/SDL_gpu/include",
        "../vendor/GLEW/include",
        "../vendor/GLM/include",
        "../vendor/SFML/include",
		"../vendor/PUGIXML/src",
		"../vendor/CURL/include",
		"../vendor/GTK/include",
		"../vendor/GTK/include/atk-1.0",
		"../vendor/GTK/include/cairo",
		"../vendor/GTK/include/epoxy",
		"../vendor/GTK/include/fontconfig",
		"../vendor/GTK/include/freetype2",
		"../vendor/GTK/include/fribidi",
		"../vendor/GTK/include/gail-3.0",
		"../vendor/GTK/include/gdk-pixbuf-2.0",
		"../vendor/GTK/include/gio-win32-2.0",
		"../vendor/GTK/include/glib-2.0",
		"../vendor/GTK/include/gtk-3.0",
		"../vendor/GTK/include/harfbuzz",
		"../vendor/GTK/include/jasper",
		"../vendor/GTK/include/libpkgconf",
		"../vendor/GTK/include/libpng16",
		"../vendor/GTK/include/libxml2",
		"../vendor/GTK/include/pango-1.0",
		"../vendor/GTK/include/pixman-1",
		"../vendor/GTK/lib/glib-2.0/include",
    }
   
    libdirs{
        "../vendor/SDL/lib/x86",
        "../vendor/SDL_image/lib/x86",
        "../vendor/SDL_ttf/lib/x86",
        "../vendor/SDL_mixer/lib/x86",
        "../vendor/SDL_net/lib/x86",
        "../vendor/SDL_gpu/lib/x86",
        "../vendor/GLEW/lib/x86",
        "../vendor/SFML/lib",
        "../vendor/CURL/lib",
        "../vendor/GTK/lib",
    }

    links{
		"SDL2.lib",
		"SDL2main.lib",
		"SDL2_image.lib",
		"SDL2_ttf.lib",
		"SDL2_mixer.lib",
		"SDL2_net.lib",
		"SDL2_gpu.lib",
		"glew32.lib",
		"opengl32.lib",
		"sfml-system-s-d.lib",
		"sfml-window-s-d.lib",
		"sfml-graphics-s-d.lib",
		"sfml-main-d.lib",
		"sfml-audio-s-d.lib",
		"sfml-network-s-d.lib",
		"openal32.lib",
		"flac.lib",
		"vorbisenc.lib",
		"vorbisfile.lib",
		"vorbis.lib",
		"ogg.lib",
		"gdi32.lib",
		"freetype.lib",
		"winmm.lib",
		"ws2_32.lib",

		"asprintf.lib",
		"atk-1.0.lib",
		"cairo-gobject.lib",
		"cairo.lib",
		"epoxy.lib",
		"ffi.lib",
		"fontconfig.lib",
		"freetype.lib",
		"fribidi.lib",
		"gailutil-3.0.lib",
		"gailutil-3.lib",
		"gdk-3.0.lib",
		"gdk-3.lib",
		"gdk_pixbuf-2.0.lib",
		"gettextpo.lib",
		"gio-2.0.lib",
		"glib-2.0.lib",
		"gmodule-2.0.lib",
		"gobject-2.0.lib",
		"gthread-2.0.lib",
		"gtk-3.0.lib",
		"gtk-3.lib",
		"harfbuzz-gobject.lib",
		"harfbuzz.lib",
		"iconv.lib",
		"intl.lib",
		"jasper.lib",
		"jpeg-static.lib",
		"jpeg.lib",
		"libpng16.lib",
		"libpng16_static.lib",
		"libxml2.lib",
		"pango-1.0.lib",
		"pangocairo-1.0.lib",
		"pangoft2-1.0.lib",
		"pangowin32-1.0.lib",
		"pixman-1.lib",
		"pkgconf.lib",
		"tiff.lib",
		"turbojpeg-static.lib",
		"turbojpeg.lib",
		"zlib1.lib",
    }

	files{
		"../main.cpp",
		"../vendor/PUGIXML/src/pugixml.cpp",
	}

	defines "SFML_STATIC"

	filter "configurations:Debug"
		symbols "On"

	filter "configurations:Release"
		optimize "On"
					