include(FetchContent)

macro(getLatestPackageVersion gitRepo versionSplat)
    find_package(Git 2.18 REQUIRED)
    if (WIN32)
        execute_process(COMMAND powershell -command "((& '${GIT_EXECUTABLE}' -c 'versionsort.suffix=-' ls-remote --exit-code --refs --sort=version:refname --tags ${gitRepo} '${versionSplat}' | Select-Object -Last 1) -Split '/')[2]" OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE LATEST_RELEASE)
    else ()
        execute_process(COMMAND ${GIT_EXECUTABLE} -c "versionsort.suffix=-" ls-remote --exit-code --refs --sort=version:refname --tags ${gitRepo} "${versionSplat}" COMMAND tail --lines=1 COMMAND cut --delimiter=/ --fields=3 COMMAND tr -d "\n" OUTPUT_VARIABLE LATEST_RELEASE)
    endif ()
endmacro()

macro(fetchPackage gitRepo versionSplat packageName)
    getLatestPackageVersion(${gitRepo} ${versionSplat})

    FetchContent_Declare(
            ${packageName}
            GIT_REPOSITORY ${gitRepo}
            GIT_TAG ${LATEST_RELEASE}
            GIT_SHALLOW TRUE
            GIT_PROGRESS TRUE
            EXCLUDE_FROM_ALL
            SYSTEM
    )
    FetchContent_MakeAvailable(${packageName})
endmacro()

macro(findOrFetchPackage gitRepo versionSplat packageName)
    getLatestPackageVersion(${gitRepo} ${versionSplat})

    FetchContent_Declare(
            ${packageName}
            GIT_REPOSITORY ${gitRepo}
            GIT_TAG ${LATEST_RELEASE}
            GIT_SHALLOW TRUE
            GIT_PROGRESS TRUE
            EXCLUDE_FROM_ALL
            SYSTEM
            FIND_PACKAGE_ARGS ${ARGN}
    )
    FetchContent_MakeAvailable(${packageName})
endmacro()
