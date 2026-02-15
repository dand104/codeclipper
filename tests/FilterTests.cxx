#include <catch2/catch_all.hpp>
#include "GitIgnoreFilter.hxx"
#include "RuntimeConfig.hxx"

using namespace codeclipper;

TEST_CASE("GitIgnoreFilter OnlyPatterns Logic", "[filter]") {
    RuntimeConfig config;
    config.onlyPatterns = {"src"};
    GitIgnoreFilter filter(config);

    SECTION("Should allow 'src' directory itself") {
        REQUIRE_FALSE(filter.isIgnored("src", true));
    }

    SECTION("Should allow files inside 'src'") {
        REQUIRE_FALSE(filter.isIgnored("src/main.cpp", false));
    }

    SECTION("Should ignore files outside 'src'") {
        REQUIRE(filter.isIgnored("README.md", false));
        REQUIRE(filter.isIgnored("build/main.o", false));
    }
}

TEST_CASE("GitIgnoreFilter Dotfiles Logic", "[filter]") {
    RuntimeConfig config;
    GitIgnoreFilter filter(config);

    SECTION("Should ignore .idea by default") {
        REQUIRE(filter.isIgnored(".idea", true));
        REQUIRE(filter.isIgnored(".idea/workspace.xml", false));
    }

    SECTION("Should allow normal files") {
        REQUIRE_FALSE(filter.isIgnored("src/main.cpp", false));
    }
}

TEST_CASE("GitIgnoreFilter GitIgnore Logic", "[filter]") {
    RuntimeConfig config;
    GitIgnoreFilter filter(config);

    SECTION("Git directory should always be ignored") {
        REQUIRE(filter.isIgnored(".git/config", false));
        REQUIRE(filter.isIgnored("sub/.git/config", false));
    }
}