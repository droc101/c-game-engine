include(FetchContent)

macro(get_latest_package_version git_repo version_splat)
    find_package(Git 2.18 REQUIRED)
    if (WIN32)
        execute_process(COMMAND powershell -command "((& '${GIT_EXECUTABLE}' -c 'versionsort.suffix=-' ls-remote --exit-code --refs --sort=version:refname --tags ${git_repo} '${version_splat}' | Select-Object -Last 1) -Split '/')[2]" OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE LATEST_RELEASE)
    else ()
        execute_process(COMMAND ${GIT_EXECUTABLE} -c "versionsort.suffix=-" ls-remote --exit-code --refs --sort=version:refname --tags ${git_repo} "${version_splat}" COMMAND tail --lines=1 COMMAND cut --delimiter=/ --fields=3 COMMAND tr -d "\n" OUTPUT_VARIABLE LATEST_RELEASE)
    endif ()
endmacro()

function(fetch_package git_repo version_splat package_name)
    get_latest_package_version(${git_repo} ${version_splat})

    FetchContent_Declare(
            ${package_name}
            GIT_REPOSITORY ${git_repo}
            GIT_TAG ${LATEST_RELEASE}
            GIT_SHALLOW TRUE
            GIT_PROGRESS TRUE
            EXCLUDE_FROM_ALL
            SYSTEM
    )
    FetchContent_MakeAvailable(${package_name})
endfunction()

function(find_or_fetch_package git_repo version_splat package_name)
    get_latest_package_version(${git_repo} ${version_splat})

    FetchContent_Declare(
            ${package_name}
            GIT_REPOSITORY ${git_repo}
            GIT_TAG ${LATEST_RELEASE}
            GIT_SHALLOW TRUE
            GIT_PROGRESS TRUE
            EXCLUDE_FROM_ALL
            SYSTEM
            FIND_PACKAGE_ARGS ${ARGN}
    )
    FetchContent_MakeAvailable(${package_name})
endfunction()

function(fetch_dict)
    get_latest_package_version(https://github.com/P-p-H-d/mlib.git V0.8.*)
    file(DOWNLOAD https://raw.githubusercontent.com/P-p-H-d/mlib/refs/tags/${LATEST_RELEASE}/m-core.h ${CMAKE_BINARY_DIR}/_deps/dict/m-core.h)
    file(DOWNLOAD https://raw.githubusercontent.com/P-p-H-d/mlib/refs/tags/${LATEST_RELEASE}/m-array.h ${CMAKE_BINARY_DIR}/_deps/dict/m-array.h)
    file(DOWNLOAD https://raw.githubusercontent.com/P-p-H-d/mlib/refs/tags/${LATEST_RELEASE}/m-dict.h ${CMAKE_BINARY_DIR}/_deps/dict/m-dict.h)
    file(DOWNLOAD https://raw.githubusercontent.com/P-p-H-d/mlib/refs/tags/${LATEST_RELEASE}/LICENSE ${CMAKE_BINARY_DIR}/_deps/dict/LICENSE)

    add_library(dict INTERFACE)
    target_include_directories(dict INTERFACE ${CMAKE_BINARY_DIR}/_deps/dict)
endfunction()

function(fetch_discord_sdk)
    file(DOWNLOAD https://dl-game-sdk.discordapp.net/3.2.1/discord_game_sdk.zip ${CMAKE_BINARY_DIR}/_deps/discord_game_sdk/discord_game_sdk.zip)
    file(ARCHIVE_EXTRACT INPUT ${CMAKE_BINARY_DIR}/_deps/discord_game_sdk/discord_game_sdk.zip DESTINATION ${CMAKE_BINARY_DIR}/_deps/discord_game_sdk/)
    add_library(discord_game_sdk SHARED IMPORTED)
    if (WIN32)
        file(COPY_FILE ${CMAKE_BINARY_DIR}/_deps/discord_game_sdk/lib/x86_64/discord_game_sdk.dll ${CMAKE_BINARY_DIR}/discord_game_sdk.dll)
        file(COPY_FILE ${CMAKE_BINARY_DIR}/discord_game_sdk.dll ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/discord_game_sdk.dll)
        set_target_properties(discord_game_sdk PROPERTIES IMPORTED_LOCATION discord_game_sdk.dll IMPORTED_IMPLIB ${CMAKE_BINARY_DIR}/_deps/discord_game_sdk/lib/x86_64/discord_game_sdk.dll.lib)
    else ()
        file(COPY_FILE ${CMAKE_BINARY_DIR}/_deps/discord_game_sdk/lib/x86_64/discord_game_sdk.so ${CMAKE_BINARY_DIR}/discord_game_sdk.so)
        file(COPY_FILE ${CMAKE_BINARY_DIR}/discord_game_sdk.so ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/discord_game_sdk.so)
        set_target_properties(discord_game_sdk PROPERTIES IMPORTED_LOCATION discord_game_sdk.so)
    endif ()
    target_link_libraries(engine PRIVATE discord_game_sdk)
    target_include_directories(engine PRIVATE ${CMAKE_BINARY_DIR}/_deps/discord_game_sdk/c/)
endfunction()
