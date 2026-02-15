#include <catch2/catch_all.hpp>
#include "FileSystem.hxx"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

TEST_CASE("FileSystem Traversal Paths", "[fs]") {
    fs::path temp = fs::temp_directory_path() / "codeclipper_test";
    fs::create_directories(temp / "src/inner");
    
    std::ofstream(temp / "src/main.cpp") << "test";
    std::ofstream(temp / "src/inner/util.h") << "test";
    std::ofstream(temp / "README.md") << "test";

    codeclipper::FileSystem system;
    std::vector<std::string> foundPaths;

    auto onEnter = [](const fs::path&) { return true; };
    auto onExit = [](const fs::path&) { return true; };
    auto onFile = [&](const codeclipper::common::FileEntry& entry) {
        foundPaths.push_back(entry.relativePath.generic_string());
        return true;
    };

    auto result = system.traverse(temp, onEnter, onExit, onFile);
    
    REQUIRE(result.has_value());
    
    CHECK_THAT(foundPaths, Catch::Matchers::VectorContains(std::string("src/main.cpp")));
    CHECK_THAT(foundPaths, Catch::Matchers::VectorContains(std::string("src/inner/util.h")));
    CHECK_THAT(foundPaths, Catch::Matchers::VectorContains(std::string("README.md")));

    fs::remove_all(temp);
}