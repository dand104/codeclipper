#include <catch2/catch_all.hpp>
#include "FileSystem.hxx"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

class TempDirFixture {
public:
    fs::path root;
    TempDirFixture() {
        root = fs::temp_directory_path() / "cc_test_env";
        fs::create_directories(root / "a/b");
        std::ofstream(root / "file1.txt") << "content1";
        std::ofstream(root / "a/file2.txt") << "content2";
        std::ofstream(root / "a/b/file3.txt") << "content3";
    }
    ~TempDirFixture() { fs::remove_all(root); }
};

TEST_CASE("FileSystem: Traversal and Relative Paths", "[fs]") {
    TempDirFixture fix;
    codeclipper::FileSystem system;

    std::vector<std::string> seenRelPaths;
    auto onFile = [&](const codeclipper::common::FileEntry& e) {
        seenRelPaths.push_back(e.relativePath.generic_string());
        return true;
    };

    auto res = system.traverse(fix.root,
        [](auto){return true;}, [](auto){return true;}, onFile);

    REQUIRE(res.has_value());
    CHECK(seenRelPaths.size() == 3);

    CHECK_THAT(seenRelPaths, Catch::Matchers::VectorContains(std::string("file1.txt")));
    CHECK_THAT(seenRelPaths, Catch::Matchers::VectorContains(std::string("a/file2.txt")));
    CHECK_THAT(seenRelPaths, Catch::Matchers::VectorContains(std::string("a/b/file3.txt")));
}

TEST_CASE("FileSystem: Binary detection", "[fs]") {
    using namespace std::string_literals;
    codeclipper::FileSystem system;

    std::string binaryData = "Hello\0World"s;

    fs::path p = fs::temp_directory_path() / "bin_test.dat";
    {
        std::ofstream ofs(p, std::ios::binary);
        ofs.write(binaryData.data(), binaryData.size());
    }

    auto content = system.readFile(p);
    CHECK_FALSE(content.has_value());

    fs::remove(p);
}