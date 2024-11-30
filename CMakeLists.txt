cmake_minimum_required(VERSION 3.25)
project(wampy)

set(CMAKE_CXX_STANDARD 11)

set(PROTOBUF_INCLUDE "")
if (DESKTOP EQUAL 1)
    set(CONNECTOR_SOURCE connector/mpd.cpp)
else ()
    set(CONNECTOR_SOURCE connector/hagoromo.cpp server/armv5-unknown-linux-gnueabihf/command_names.cpp)
    set(PROTOBUF_INCLUDE libs/protobuf/build-armv5/include/)
    set(PROTOBUF_SOURCE server/armv5-unknown-linux-gnueabihf/command.pb.cc)
endif ()

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
        libs/mINI/src
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
link_libraries(glfw)

if (DESKTOP EQUAL 1)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install/arm)
else ()
    set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install/x86)

    # https://stackoverflow.com/questions/23583317/c-11-threads-error-pure-virtual-function-called
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_1")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4")
endif ()


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
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
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
        src/util/util.cpp
        src/wstring.cpp
        src/cassette/tape.cpp
        src/${CONNECTOR_SOURCE}
        src/imgui_widgets.cpp
        src/connector/song.cpp
        src/skinVariant.cpp
        src/magick/magick.cpp
        src/winamp/winamp.cpp
        src/cassette/cassette.cpp
        src/config.cpp
        #        libs/imgui/imgui_demo.cpp
        src/shader.cpp)

if (DESKTOP EQUAL 1)
    set(arch x86)
else ()
    set(arch arm)
endif ()

set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install/${arch})

find_library(JPEG jpeg PATHS libs/libjpeg-turbo/build-${arch}/release/lib REQUIRED NO_DEFAULT_PATH)
target_link_libraries(${PROJECT_NAME} "${JPEG}")

target_link_libraries(${PROJECT_NAME} zlib)

# dynamic linking
set(ENV{PKG_CONFIG_LIBDIR} "${CMAKE_SOURCE_DIR}/libs/ImageMagick/build-${arch}/lib/pkgconfig")
find_package(ImageMagick COMPONENTS MagickCore MagickWand Magick++ REQUIRED)
include_directories(${ImageMagick_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${ImageMagick_LIBRARIES})

if (DESKTOP EQUAL 1)
    find_library(GLES GLESv2 PATHS /usr/lib)
    target_link_libraries(${PROJECT_NAME} "${GLES}")
    target_compile_definitions(${PROJECT_NAME} PRIVATE DESKTOP=1)
else ()
    set_target_properties(${PROJECT_NAME} PROPERTIES
            BUILD_WITH_INSTALL_RPATH FALSE
            INSTALL_RPATH_USE_LINK_PATH FALSE
            INSTALL_RPATH "./;../lib/;/system/lib;/system/vendor/unknown321/lib;/lib;/system/vendor/sony/lib")

    find_library(MALI Mali_linux PATHS libs)
    #    find_library(ASOUND asound PATHS libs)
    find_library(PROTOBUF protobuf REQUIRED PATHS libs/protobuf/build-armv5/lib/ REQUIRED)
    target_link_libraries(${PROJECT_NAME} "${MALI}" "${PROTOBUF}")
    #    target_link_libraries(${PROJECT_NAME} "${ASOUND}")
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
endif ()
