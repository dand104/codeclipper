cmake_minimum_required(VERSION 3.20)
project(codeclipper LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(FetchContent)

option(BUILD_TESTS "Build unit-tests for codeclipper" on)

# Dependencies

    # CLI11
    FetchContent_Declare(
            cli11
            GIT_REPOSITORY https://github.com/CLIUtils/CLI11
            GIT_TAG v2.6.1
    )
    FetchContent_MakeAvailable(cli11)

    # {fmt}
    FetchContent_Declare(
            fmt
            GIT_REPOSITORY https://github.com/fmtlib/fmt.git
            GIT_TAG 12.1.0
    )
    FetchContent_MakeAvailable(fmt)

    # clip
    FetchContent_Declare(
            clip
            GIT_REPOSITORY https://github.com/dacap/clip.git
            GIT_TAG v1.13
    )
    set(CLIP_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(CLIP_TESTS OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(clip)
    if(TARGET clip)
        target_include_directories(clip PUBLIC
                $<BUILD_INTERFACE:${clip_SOURCE_DIR}>
        )
    endif()

    # Catch2
    if(BUILD_TESTS)
        FetchContent_Declare(
                Catch2
                GIT_REPOSITORY https://github.com/catchorg/Catch2.git
                GIT_TAG v3.12.0
        )
        FetchContent_MakeAvailable(Catch2)
    endif()

add_library(codeclipper-main OBJECT
        src/CliParser.cxx
        src/ConsoleWrite.cxx
        src/SystemClipboard.cxx
        src/GitIgnoreFilter.cxx
        src/FileSystem.cxx
        src/ContextGatherer.cxx
)
target_include_directories(codeclipper-main PUBLIC src)

target_link_libraries(codeclipper-main PUBLIC
        CLI11::CLI11
        fmt::fmt
        clip
)

add_executable(${PROJECT_NAME} src/main.cxx)
target_link_libraries(${PROJECT_NAME} PRIVATE codeclipper-main)

if(BUILD_TESTS)
    enable_testing()
    add_executable(${PROJECT_NAME}Test
            tests/FilterTests.cxx
            tests/FileSystemTests.cxx
    )
    target_link_libraries(${PROJECT_NAME}Test PRIVATE codeclipper-main Catch2::Catch2WithMain)
    add_test(NAME AllTests COMMAND ${PROJECT_NAME}Test)
endif ()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX /permissive-)
elseif(CLANG OR GCC)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Werror -pedantic)
endif()