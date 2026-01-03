include(FetchContent)

macro(get_latest_package_version git_repo version_glob)
    find_package(Git 2.18 REQUIRED)
    if (WIN32)
        execute_process(COMMAND powershell -command "((& '${GIT_EXECUTABLE}' -c 'versionsort.suffix=-' ls-remote --exit-code --refs --sort=version:refname --tags ${git_repo} '${version_glob}' | Select-Object -Last 1) -Split '/')[2]" OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE LATEST_RELEASE)
    else ()
        execute_process(COMMAND ${GIT_EXECUTABLE} -c "versionsort.suffix=-" ls-remote --exit-code --refs --sort=version:refname --tags ${git_repo} "${version_glob}" COMMAND tail --lines=1 COMMAND cut --delimiter=/ --fields=3 COMMAND tr -d "\n" OUTPUT_VARIABLE LATEST_RELEASE)
    endif ()
endmacro()

function(fetch_package git_repo tag package_name)
    #    message(STATUS "FetchContent_Declare ${package_name}")
    FetchContent_Declare(
            ${package_name}
            GIT_REPOSITORY ${git_repo}
            GIT_TAG ${tag}
            GIT_SHALLOW TRUE
            GIT_PROGRESS TRUE
            EXCLUDE_FROM_ALL
            SYSTEM
    )
    #    message(STATUS "FetchContent_MakeAvailable ${package_name}")
    FetchContent_MakeAvailable(${package_name})
endfunction()

function(fetch_package_tag git_repo version_glob package_name)
    get_latest_package_version(${git_repo} ${version_glob})
    fetch_package(${git_repo} ${LATEST_RELEASE} ${package_name})
endfunction()

function(fetch_glew)
    message(STATUS "Fetching GLEW...")
    set(GLEW_EGL ON)
    set(GLEW_GLX OFF)
    set(BUILD_UTILS OFF)
    set(BUILD_SHARED_LIBS OFF)

    get_latest_package_version(https://github.com/nigels-com/glew glew-2.3.*)

    FetchContent_Declare(
            GLEW
            URL https://github.com/nigels-com/glew/releases/download/${LATEST_RELEASE}/${LATEST_RELEASE}.tgz
            SOURCE_SUBDIR "build/cmake"
            DOWNLOAD_EXTRACT_TIMESTAMP
            EXCLUDE_FROM_ALL
            SYSTEM
    )
    FetchContent_MakeAvailable(GLEW)
endfunction()

function(fetch_dict)
    message(STATUS "Fetching mlib dict...")
    set(DICT_DIR ${CMAKE_CURRENT_BINARY_DIR}/_deps/dict)
    get_latest_package_version(https://github.com/P-p-H-d/mlib.git V0.8.*)
    file(DOWNLOAD https://raw.githubusercontent.com/P-p-H-d/mlib/refs/tags/${LATEST_RELEASE}/m-core.h ${DICT_DIR}/m-core.h)
    file(DOWNLOAD https://raw.githubusercontent.com/P-p-H-d/mlib/refs/tags/${LATEST_RELEASE}/m-array.h ${DICT_DIR}/m-array.h)
    file(DOWNLOAD https://raw.githubusercontent.com/P-p-H-d/mlib/refs/tags/${LATEST_RELEASE}/m-dict.h ${DICT_DIR}/m-dict.h)
    file(DOWNLOAD https://raw.githubusercontent.com/P-p-H-d/mlib/refs/tags/${LATEST_RELEASE}/LICENSE ${DICT_DIR}/LICENSE)

    add_library(dict INTERFACE)
    target_include_directories(dict INTERFACE ${DICT_DIR})
endfunction()

macro(fetch_discord_sdk)
    message(STATUS "Fetching Discord Game SDK...")
    if ((NOT DEFINED DISCORD_GAME_SDK_DIR) OR (DISCORD_GAME_SDK_DIR STREQUAL ""))
        set(DISCORD_GAME_SDK_DIR ${CMAKE_CURRENT_BINARY_DIR}/_deps/discord_game_sdk)
    endif ()
    if ((NOT DEFINED DISCORD_GAME_SDK_LIBRARY_DIR) OR (DISCORD_GAME_SDK_LIBRARY_DIR STREQUAL ""))
        set(DISCORD_GAME_SDK_LIBRARY_DIR ${DISCORD_GAME_SDK_DIR}/lib/x86_64)
    endif ()

    file(DOWNLOAD https://dl-game-sdk.discordapp.net/3.2.1/discord_game_sdk.zip ${DISCORD_GAME_SDK_DIR}/discord_game_sdk.zip)
    file(ARCHIVE_EXTRACT INPUT ${DISCORD_GAME_SDK_DIR}/discord_game_sdk.zip DESTINATION ${DISCORD_GAME_SDK_DIR}/)
    add_library(discord_game_sdk SHARED IMPORTED)
    if (WIN32)
        file(COPY_FILE ${DISCORD_GAME_SDK_LIBRARY_DIR}/discord_game_sdk.dll ${CMAKE_BINARY_DIR}/discord_game_sdk.dll)
        set_target_properties(discord_game_sdk PROPERTIES IMPORTED_LOCATION discord_game_sdk.dll IMPORTED_IMPLIB ${DISCORD_GAME_SDK_LIBRARY_DIR}/discord_game_sdk.dll.lib)
    else ()
        file(COPY_FILE ${DISCORD_GAME_SDK_LIBRARY_DIR}/discord_game_sdk.so ${CMAKE_BINARY_DIR}/discord_game_sdk.so)
        set_target_properties(discord_game_sdk PROPERTIES IMPORTED_LOCATION discord_game_sdk.so)
    endif ()
endmacro()
