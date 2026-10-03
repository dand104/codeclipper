#include <catch2/catch_all.hpp>
import std;
import codeclipper;

namespace fs = std::filesystem;

class TempDirFixture {
public:
    fs::path root;
    TempDirFixture() {
        root = fs::temp_directory_path() / "cc_test_fs";
        fs::create_directories(root / "a/b");
        std::ofstream(root / "file1.txt") << "content1";
        std::ofstream(root / "a/file2.txt") << "content2";
        std::ofstream(root / "a/b/file3.txt") << "content3";
    }
    ~TempDirFixture() {
        std::error_code ec;
        fs::remove_all(root, ec);
    }
};

TEST_CASE("FileSystem: Traversal and Binary Detection", "[fs]") {
    TempDirFixture fix;

    SECTION("Gathers all non-binary files") {
        codeclipper::RuntimeConfig config;
        config.rootPath = fix.root;
        config.useClipboard = false;
        config.listOnly = true;

        std::ostringstream out;
        codeclipper::ContextEngine engine(config, out);
        auto res = engine.run();

        REQUIRE(res.has_value());
        std::string output = out.str();

        CHECK(output.contains("file1.txt"));
        CHECK(output.contains("a/file2.txt"));
        CHECK(output.contains("a/b/file3.txt"));
    }

    SECTION("Skips binary files") {
        fs::path binPath = fix.root / "binary.bin";
        {
            std::ofstream binFile(binPath, std::ios::binary);
            char zero[4] = {'a', '\0', 'b', 'c'};
            binFile.write(zero, sizeof(zero));
        }

        codeclipper::RuntimeConfig config;
        config.rootPath = fix.root;
        config.useClipboard = false;
        config.listOnly = false;

        std::ostringstream out;
        codeclipper::ContextEngine engine(config, out);
        auto res = engine.run();

        REQUIRE(res.has_value());
        CHECK_FALSE(out.str().contains("```path=binary.bin"));
    }
}