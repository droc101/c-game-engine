set(GIT_SHORT_HASH "unknown")
set(GIT_HASH "unknown")
set(GIT_EXECUTABLE "git") # find_package(Git) fails here for some reason

execute_process(
        COMMAND ${GIT_EXECUTABLE} status --short --porcelain=1 --untracked-files=no
        WORKING_DIRECTORY ${REPO_DIR}
        OUTPUT_VARIABLE GIT_STATUS
        RESULT_VARIABLE GIT_STATUS_RETURN_CODE
        ERROR_VARIABLE GIT_STATUS_ERROR
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
else ()
    message(WARNING "git status failed with code ${GIT_STATUS_RETURN_CODE}! stdout: ${GIT_STATUS} stderr: ${GIT_STATUS_ERROR}")
endif ()

message(STATUS "Git hash: ${GIT_HASH}")

configure_file(${IN_FILE} ${OUT_FILE} @ONLY)