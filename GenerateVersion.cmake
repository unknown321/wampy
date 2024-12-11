find_package(Git)
if (GIT_EXECUTABLE)
    execute_process(
            COMMAND ${GIT_EXECUTABLE} status --porcelain
            OUTPUT_VARIABLE DIRTY_STRINGS
    )

    if (NOT DIRTY_STRINGS STREQUAL "")
        execute_process(
                COMMAND ${GIT_EXECUTABLE} log -1 "--format=%h-dirty, %ad" "--date=format:%Y-%m-%d %H:%M"
                OUTPUT_VARIABLE WAMPY_VERSION
                RESULT_VARIABLE ERROR_CODE
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    else ()
        execute_process(
                COMMAND ${GIT_EXECUTABLE} log -1 "--format=%h, %ad" "--date=format:%Y-%m-%d %H:%M"
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
