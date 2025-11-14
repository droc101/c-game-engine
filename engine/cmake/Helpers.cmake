macro(enable_options)
    foreach (OPTION_TO_SET IN ITEMS ${ARGN})
        set(${OPTION_TO_SET} ON CACHE BOOL "" FORCE)
    endforeach ()
endmacro()

macro(disable_options)
    foreach (OPTION_TO_SET IN ITEMS ${ARGN})
        set(${OPTION_TO_SET} OFF CACHE BOOL "" FORCE)
    endforeach ()
endmacro()

macro(detect_platform)
    include(CheckCSourceCompiles)
    set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
    check_c_source_compiles("
    #if !((defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64)) && !defined(_M_ARM64EC))
        #error
    #endif" x86_64)
    if (NOT x86_64)
        check_c_source_compiles("
        #if !(defined(__aarch64__) || defined(_M_ARM64))
            #error
        #endif" arm64)
        if (arm64)
            message(STATUS "Detected ARM64 architecture (no support will be provided)")
        else ()
            message(FATAL_ERROR "Unsupported architecture: ${CMAKE_SYSTEM_PROCESSOR}. Supported architectures are x86_64 and aarch64.")
        endif ()
    endif ()
    if (NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
        message(FATAL_ERROR "GAME only supports 64-bit systems (CMAKE_SIZEOF_VOID_P=${CMAKE_SIZEOF_VOID_P})")
    endif ()
endmacro()

function(enable_lto)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT result)
    if (result)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE PARENT_SCOPE)
        message(STATUS "IPO/LTO is supported")
    else ()
        message(WARNING "IPO/LTO is not supported")
    endif ()
endfunction()

macro(create_game_module target_name march sources compile_definitions)
    add_library(${target_name} MODULE EXCLUDE_FROM_ALL)
    target_sources(${target_name} PRIVATE ${sources})

    target_compile_definitions(${target_name} PRIVATE ${compile_definitions})
    target_compile_options(${target_name} PRIVATE -march=${march})
    target_link_libraries(${target_name} PRIVATE engine)
    target_include_directories(${target_name} PRIVATE include)
    set_target_properties(${target_name} PROPERTIES LINKER_LANGUAGE CXX LINK_FLAGS "-Wl,-rpath='$ORIGIN'" PREFIX "")
    if (WIN32)
        add_custom_command(TARGET ${target_name} PRE_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${DISCORD_GAME_SDK_LIBRARY_DIR}/discord_game_sdk.dll"
                "${CMAKE_CURRENT_BINARY_DIR}/discord_game_sdk.dll"
        )
    else ()
        add_custom_command(TARGET ${target_name} PRE_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${DISCORD_GAME_SDK_LIBRARY_DIR}/discord_game_sdk.so"
                "${CMAKE_CURRENT_BINARY_DIR}/discord_game_sdk.so"
        )
    endif ()
endmacro()
