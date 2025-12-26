cmake_minimum_required(VERSION 3.25)
project(wampy)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_FIND_DEBUG_MODE OFF)
# add -Wl,--verbose=3 for linking
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer")

if (DESKTOP EQUAL 1)
    set(arch x86)
    set(CONNECTOR_SOURCE src/connector/mpd.cpp src/connector/hagoromo.cpp server/armv5-unknown-linux-gnueabihf/command_names.cpp)
else ()
    set(arch arm)
    set(CONNECTOR_SOURCE src/connector/hagoromo.cpp server/armv5-unknown-linux-gnueabihf/command_names.cpp)

    # https://stackoverflow.com/questions/23583317/c-11-threads-error-pure-virtual-function-called
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_1")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4")
endif ()


set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install/${arch})

set(PROTOBUF_INCLUDE libs/protobuf/build-${arch}/include/)
set(PROTOBUF_SOURCE server/armv5-unknown-linux-gnueabihf/command.pb.cc)
find_package(PkgConfig REQUIRED)

include_directories(
        libs/imgui
        libs/imgui/backends
        libs/glfw/include
        libs/GLES
        libs/GLES2
        libs/glad/include/KHR
        libs/glad/include
        libs/GL
        libs/zlib
        libs/ImageMagick
        libs/ImageMagick/Magick++/lib
        libs/glm
        libs/alsa-lib/src/include
        libs/alsa-lib/src/include/alsa
        libs/mINI/src
        libs/sqlite/build-${arch}/install/include
        libs/icu/icu4c/source/build-${arch}/install/include
        libs/implot
        libs/shine/build-${arch}/include
        libs
        server/armv5-unknown-linux-gnueabihf
        ${PROTOBUF_INCLUDE}

        server/qt/qtbase/src/plugins/platforms/eglfs
        server/qt/qtbase/
        server/qt/qtbase/include/
        server/qt/qtbase/include/QtCore
        server/qt/qtbase/include/QtWidgets
        server/qt/qtbase/include/QtGui
        server/qt/qtbase/include/QtGui/5.3.2/QtGui
        server/qt/qtbase/include/QtANGLE
        server/qt/qtdeclarative/include/
        server/qt/qtdeclarative/include/QtQml
        server/qt/qtdeclarative/include/QtQuick

        pstserver/
        sound_service_fw/
        src/util/
        src/
)

set(GLFW_BUILD_WAYLAND false)
set(GLFW_INCLUDE_ES2 true)
set(GLFW_INCLUDE_GLEXT true)
set(GL_GLEXT_PROTOTYPES true)
set(GLFW_EXPOSE_NATIVE_EGL true)
set(GLFW_BUILD_X11 OFF)
if (DESKTOP EQUAL 1)
    set(GLFW_BUILD_X11 ON)
endif ()

add_subdirectory(libs/glfw)

set(ZLIB_BUILD_EXAMPLES FALSE)
add_subdirectory(libs/zlib)
set_property(TARGET zlib PROPERTY VERSION)
set_property(TARGET zlib PROPERTY SOVERSION)

set(BUILD_DOCS OFF)
#set(BUILD_SHARED_LIBS OFF)
add_subdirectory(libs/mINI)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DMAGICKCORE_HDRI_ENABLE=1 ")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMAGICKCORE_HDRI_ENABLE=1 ")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMAGICKCORE_QUANTUM_DEPTH=32")

# profiling
# exec; kill -10 <pid>
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --verbose")

add_compile_definitions(IMGUI_USER_CONFIG="../../src/imgui.config.h")
set(IMGUI_DIR libs/imgui)

add_executable(${PROJECT_NAME}
        src/main.cpp
        src/imgui.config.h
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        libs/implot/implot.cpp
        libs/implot/implot_items.cpp
        libs/glad/src/glad.c
        libs/GLES2/gl2platform.h
        libs/glad/include/KHR/khrplatform.h
        libs/GL/glcorearb.h
        libs/glfw/deps/glad/gl.h
        libs/glm/glm/ext/matrix_transform.inl
        libs/zlib/contrib/minizip/unzip.c
        libs/zlib/contrib/minizip/ioapi.c
        libs/ImageMagick/Magick++/lib/Color.cpp
        ${PROTOBUF_SOURCE}
        src/implot_widgets.cpp
        src/skin/fm.cpp
        src/skin/volumeTables.cpp
        src/skin/dac.cpp
        src/skin/cassette.cpp
        src/skin/digiclock.cpp
        src/skin/winamp.cpp
        src/skin/misc.cpp
        src/skin/website.cpp
        src/skin/w1.cpp
        src/skin/eq.cpp
        src/skin/eqTabs.cpp
        src/skin/license.cpp
        src/skin/eqPerSong.cpp
        src/skin/debug.cpp
        src/skin.cpp
        src/langToString/langToString.cpp
        src/dac/cxd3778gf_table.cpp
        src/util/util_string.cpp
        src/util/mkpath.cpp
        src/util/util.cpp
        src/wstring.cpp
        src/w1/w1.cpp
        src/cassette/tape.cpp
        src/cassette/config.cpp
        src/digital_clock/digital_clock.cpp
        ${CONNECTOR_SOURCE}
        src/connector/hagoromoToString.cpp
        src/imgui_widgets.cpp
        src/connector/song.cpp
        src/skinVariant.cpp
        src/magick/magick.cpp
        src/winamp/winamp.cpp
        src/cassette/cassette.cpp
        src/config.cpp
        src/rec/wav.cpp
        src/rec/mp3.cpp
        src/rec/rec.cpp
        #        libs/imgui/imgui_demo.cpp
        #        libs/implot/implot_demo.cpp
        src/shader.cpp
        src/sound_settings/sound_settings.cpp
        src/sound_settings_fw/sound_settings_fw.cpp
        src/dac/dac.cpp)

#add_executable(dac
#        src/dac/cxd3778gf_table.cpp
#        src/dac/dac.cpp
#        src/dac/main.cpp
#        )


set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install/${arch})

target_link_libraries(${PROJECT_NAME} glfw)

find_library(JPEG jpeg PATHS libs/libjpeg-turbo/build-${arch}/release/lib REQUIRED NO_DEFAULT_PATH)
target_link_libraries(${PROJECT_NAME} "${JPEG}")

target_link_libraries(${PROJECT_NAME} zlib)

# dynamic linking
set(ENV{PKG_CONFIG_LIBDIR} "${CMAKE_SOURCE_DIR}/libs/ImageMagick/build-${arch}/lib/pkgconfig")
find_package(ImageMagick COMPONENTS MagickCore MagickWand Magick++ REQUIRED)
include_directories(${ImageMagick_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${ImageMagick_LIBRARIES})

find_library(PROTOBUF protobuf REQUIRED PATHS libs/protobuf/build-${arch}/lib/ REQUIRED)
target_link_libraries(${PROJECT_NAME} "${PROTOBUF}")

set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/libs/sqlite/build-${arch}/install")
find_package(SQLite3 REQUIRED)
target_link_libraries(${PROJECT_NAME} "${SQLite3_LIBRARIES}")

set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/libs/icu/icu4c/source/build-${arch}/install")
find_package(ICU REQUIRED COMPONENTS data i18n uc)
target_link_libraries(${PROJECT_NAME} "${ICU_DATA_LIBRARIES}" "${ICU_LIBRARIES}")

target_link_libraries(${PROJECT_NAME} m)

add_custom_target(version
        ${CMAKE_COMMAND} -D SRC=${CMAKE_SOURCE_DIR}/src/Version.h.in
        -D DST=${CMAKE_SOURCE_DIR}/src/Version.h
        -P ${CMAKE_SOURCE_DIR}/GenerateVersion.cmake
)

add_dependencies(${PROJECT_NAME} version)

if (DESKTOP EQUAL 1)
    find_library(GLES GLESv2 PATHS /usr/lib)
    target_link_libraries(${PROJECT_NAME} ${GLES})
    target_compile_definitions(${PROJECT_NAME} PRIVATE DESKTOP=1)
    find_package(ALSA REQUIRED)
    target_link_libraries(${PROJECT_NAME} ${ALSA_LIBRARIES})
else ()
    set_target_properties(${PROJECT_NAME} PROPERTIES
            BUILD_WITH_INSTALL_RPATH FALSE
            INSTALL_RPATH_USE_LINK_PATH FALSE
            INSTALL_RPATH "./;../lib/;/system/lib;/system/vendor/unknown321/lib;/lib;/system/vendor/sony/lib")

    find_library(MALI Mali_linux PATHS libs/sysroot/lib/)
    target_link_libraries(${PROJECT_NAME} ${MALI})
    find_library(ASOUND asound PATHS libs/sysroot/lib/)
    target_link_libraries(${PROJECT_NAME} ${ASOUND})
    #    target_link_libraries(${PROJECT_NAME} "${ASOUND}")
endif ()

install(TARGETS ${PROJECT_NAME})

# dynamic linking
foreach (Library ${ImageMagick_LIBRARIES})
    install(FILES ${Library} DESTINATION lib)
endforeach ()

# https://gitlab.kitware.com/cmake/cmake/-/issues/23249
get_filename_component(JPEG_NAME ${JPEG} REALPATH)
install(FILES ${JPEG_NAME} DESTINATION lib)

get_filename_component(PROTOBUF_NAME ${PROTOBUF} REALPATH)
install(FILES ${PROTOBUF_NAME} DESTINATION lib)

foreach (Library ${ICU_LIBRARIES})
    get_filename_component(Libname ${Library} REALPATH)
    get_filename_component(Libdir ${Libname} DIRECTORY)
    get_filename_component(LibnameWLE ${Libname} NAME_WLE)
    install(FILES ${Libname} DESTINATION lib RENAME ${LibnameWLE})
endforeach ()

set(ENV{PKG_CONFIG_PATH} "${CMAKE_SOURCE_DIR}/libs/shine/build-${arch}/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
pkg_check_modules(SHINE REQUIRED IMPORTED_TARGET shine)
target_link_libraries(${PROJECT_NAME} PkgConfig::SHINE)

foreach (Library ${SHINE_LINK_LIBRARIES})
    get_filename_component(Libname ${Library} REALPATH)
    get_filename_component(Libdir ${Libname} DIRECTORY)
    get_filename_component(LibnameWLE ${Libname} NAME_WLE)
    install(FILES ${Libname} DESTINATION lib RENAME ${LibnameWLE})
endforeach ()

add_executable(rec
        src/util/mkpath.cpp
        src/rec/wav.cpp
        src/rec/mp3.cpp
        src/rec/rec.cpp
        src/rec/main.cpp
)
target_link_libraries(rec PkgConfig::SHINE)
target_link_libraries(rec "${ICU_DATA_LIBRARIES}" "${ICU_LIBRARIES}")

if (DESKTOP EQUAL 1)
    target_compile_definitions(rec PRIVATE DESKTOP=1)
    find_package(ALSA REQUIRED)
    target_link_libraries(rec ${ALSA_LIBRARIES})
else ()
    target_link_libraries(rec ${ASOUND})
endif ()

set_target_properties(rec PROPERTIES
        BUILD_WITH_INSTALL_RPATH FALSE
        INSTALL_RPATH_USE_LINK_PATH FALSE
        INSTALL_RPATH " ./;../lib/;/system/lib;/system/vendor/unknown321/lib;/lib;/system/vendor/sony/lib")


add_executable(tl
        src/langToString/langToString.cpp
        src/langToString/main.cpp
)

if (DESKTOP EQUAL 1)
    target_compile_definitions(tl PRIVATE DESKTOP=1)
endif ()

set_target_properties(tl PROPERTIES
        BUILD_WITH_INSTALL_RPATH FALSE
        INSTALL_RPATH_USE_LINK_PATH FALSE
        INSTALL_RPATH " ./;../lib/;/system/lib;/system/vendor/unknown321/lib;/lib;/system/vendor/sony/lib")