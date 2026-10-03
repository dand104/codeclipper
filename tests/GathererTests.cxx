#include <catch2/catch_all.hpp>
import std;
import codeclipper;

namespace fs = std::filesystem;

class GathererEnvFixture {
public:
    fs::path root;
    GathererEnvFixture() {
        root = fs::temp_directory_path() / "cc_gather_env";
        std::error_code ec;
        fs::remove_all(root, ec);
        fs::create_directories(root);
        std::ofstream(root / "hello.txt") << "Hello World Content";
    }
    ~GathererEnvFixture() {
        std::error_code ec;
        fs::remove_all(root, ec);
    }
};

TEST_CASE("ContextEngine: Formatting Output", "[engine]") {
    GathererEnvFixture fix;
    codeclipper::RuntimeConfig config;
    config.rootPath = fix.root;
    config.useClipboard = false;

    SECTION("Full Markdown Format") {
        std::ostringstream out;
        codeclipper::ContextEngine engine(config, out);
        auto res = engine.run();

        REQUIRE(res.has_value());
        std::string expected = "```path=hello.txt\nHello World Content\n```\n";
        CHECK(out.str() == expected);
    }

    SECTION("List-only Mode") {
        config.listOnly = true;
        std::ostringstream out;
        codeclipper::ContextEngine engine(config, out);
        auto res = engine.run();

        REQUIRE(res.has_value());
        CHECK(out.str() == "hello.txt\n");
    }
}