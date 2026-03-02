find_package(Git 2.18 REQUIRED QUIET)

set(GIT_SHORT_HASH "unknown")
set(GIT_HASH "unknown")

if (${GIT_FOUND})
    execute_process(
            COMMAND ${GIT_EXECUTABLE} status --porcelain=1 --untracked-files=no
            WORKING_DIRECTORY ${REPO_DIR}
            OUTPUT_VARIABLE GIT_STATUS
            RESULT_VARIABLE GIT_STATUS_RETURN_CODE
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if (${GIT_STATUS_RETURN_CODE} EQUAL 0)
        execute_process(
                COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
                WORKING_DIRECTORY ${REPO_DIR}
                OUTPUT_VARIABLE GIT_SHORT_HASH
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        execute_process(
                COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
                WORKING_DIRECTORY ${REPO_DIR}
                OUTPUT_VARIABLE GIT_HASH
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        if (NOT "${GIT_STATUS}" STREQUAL "")
            set(GIT_SHORT_HASH "based on ${GIT_SHORT_HASH}")
            set(GIT_HASH "uncommitted changes based on ${GIT_HASH}")
        endif ()
    endif ()
endif ()

message(STATUS "Git hash: ${GIT_HASH}")

configure_file(${IN_FILE} ${OUT_FILE} @ONLY)