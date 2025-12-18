find_package(Git)
if (GIT_EXECUTABLE)
    execute_process(
            COMMAND ${GIT_EXECUTABLE} status --porcelain
            OUTPUT_VARIABLE DIRTY_STRINGS
    )

    if (NOT DIRTY_STRINGS STREQUAL "")
        string(TIMESTAMP date "%Y-%m-%d %H:%M")
        execute_process(
                COMMAND ${GIT_EXECUTABLE} log -1 --format=%h-dirty
                OUTPUT_VARIABLE WAMPY_VERSION
                RESULT_VARIABLE ERROR_CODE
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        string(CONCAT WAMPY_VERSION ${WAMPY_VERSION} ", " ${date})
    else ()
        execute_process(
                COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=0
                OUTPUT_VARIABLE WAMPY_VERSION
                RESULT_VARIABLE ERROR_CODE
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    endif ()
endif ()


if (WAMPY_VERSION STREQUAL "")
    set(WAMPY_VERSION unknown)
    message(WARNING "Failed to determine version from Git tags. Using default version \"${WAMPY_VERSION}\".")
endif ()

configure_file(${SRC} ${DST} @ONLY)
