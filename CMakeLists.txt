cmake_minimum_required(VERSION 3.20)
project(codeclipper LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CODECLIP_EXECUTABLE_NAME cclip)
include(FetchContent)

if(NOT FORCE_NOT_LTO)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON) # LTO by default for release
endif()

option(BUILD_TESTS "Build unit-tests for codeclipper" ON )
option(ENABLE_COVERAGE "Enable code coverage generation" OFF)
option(ENABLE_SANITIZER "Enable Address and Undefined sanitizers" OFF)

if(ENABLE_SANITIZER AND (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU"))
        message(STATUS "Sanitizers enabled (Address, Undefined)")
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
endif()

# Coverage for gcov

    if(ENABLE_COVERAGE)
        if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            message(FATAL_ERROR "Code coverage requires Clang or GCC")
        endif()

        find_program(LCOV_PATH lcov REQUIRED)
        find_program(GENHTML_PATH genhtml REQUIRED)
        set(BUILD_TESTS ON)
        add_compile_options(--coverage -g -O0)
        add_link_options(--coverage)

        add_custom_target(coverage
                COMMAND ${LCOV_PATH} --directory . --zerocounters
                COMMAND ./codeclipperTest
                COMMAND ${LCOV_PATH} --directory . --capture --output-file coverage.info
                COMMAND ${LCOV_PATH} --remove coverage.info
                '/usr/*'
                '*/_deps/*'
                '*/tests/*'
                --output-file coverage_cleaned.info
                COMMAND ${GENHTML_PATH} -o coverage_report coverage_cleaned.info
                COMMAND ${CMAKE_COMMAND} -E echo "Coverage report generated in: ${CMAKE_BINARY_DIR}/coverage_report/index.html"
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "Generating code coverage report..."
        )

        add_dependencies(coverage ${PROJECT_NAME}Test)
    endif()

# Dependencies

    # CLI11
    FetchContent_Declare(
            cli11
            GIT_REPOSITORY https://github.com/CLIUtils/CLI11
            GIT_TAG v2.6.1
            GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(cli11)

    # tl/expected
    FetchContent_Declare(
            tlexpected
            GIT_REPOSITORY https://github.com/TartanLlama/expected.git
            GIT_TAG v1.3.1
            GIT_SHALLOW    TRUE
    )
    set(EXPECTED_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(EXPECTED_BUILD_PACKAGE_DEB OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(tlexpected)

    # {fmt}
    FetchContent_Declare(
            fmt
            GIT_REPOSITORY https://github.com/fmtlib/fmt.git
            GIT_TAG 12.1.0
            GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(fmt)

    # clip
    FetchContent_Declare(
            clip
            GIT_REPOSITORY https://github.com/dacap/clip.git
            GIT_TAG v1.13
            GIT_SHALLOW    TRUE
    )
    set(CLIP_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(CLIP_TESTS OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(clip)
    if(TARGET clip)
        target_include_directories(clip PUBLIC
                $<BUILD_INTERFACE:${clip_SOURCE_DIR}>
        )
    endif()

    # nlohmann_json
    FetchContent_Declare(
            json
            GIT_REPOSITORY https://github.com/nlohmann/json.git
            GIT_TAG v3.12.0
            GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(json)

    # Catch2
    if(BUILD_TESTS)
        FetchContent_Declare(
                Catch2
                GIT_REPOSITORY https://github.com/catchorg/Catch2.git
                GIT_TAG v3.12.0
                GIT_SHALLOW    TRUE
        )
        FetchContent_MakeAvailable(Catch2)
    endif()

add_library(codeclipper-main OBJECT
        src/CliParser.cxx
        src/ConsoleWrite.cxx
        src/SystemClipboard.cxx
        src/PathFilter.cxx
        src/FileSystem.cxx
        src/ContextGatherer.cxx
        src/StringWriter.cxx
        src/StringWriter.hxx
        src/McpServer.cxx
        src/McpServer.hxx
)

target_include_directories(codeclipper-main PUBLIC src)

target_link_libraries(codeclipper-main PUBLIC
        CLI11::CLI11
        tl::expected
        fmt::fmt
        nlohmann_json::nlohmann_json
        clip
)

add_executable(${CODECLIP_EXECUTABLE_NAME} src/main.cxx)
target_link_libraries(${CODECLIP_EXECUTABLE_NAME} PRIVATE codeclipper-main)

if(BUILD_TESTS)
    enable_testing()
    include(Catch)

    add_executable(${PROJECT_NAME}Test
            tests/FilterTests.cxx
            tests/FileSystemTests.cxx
            tests/GathererTests.cxx
            tests/StringWriterTests.cxx
            tests/McpTests.cxx
    )
    target_link_libraries(${PROJECT_NAME}Test PRIVATE codeclipper-main Catch2::Catch2WithMain)
    catch_discover_tests(${PROJECT_NAME}Test)
endif ()

if(MSVC)
    target_compile_options(${CODECLIP_EXECUTABLE_NAME} PRIVATE /W4 /WX /permissive-)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${CODECLIP_EXECUTABLE_NAME})
elseif(CLANG OR GCC)
    target_compile_options(${CODECLIP_EXECUTABLE_NAME} PRIVATE -Wall -Wextra -Werror -pedantic)
endif()