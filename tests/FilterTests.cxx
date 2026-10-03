#include <catch2/catch_all.hpp>
import std;
import codeclipper;

using namespace codeclipper;

TEST_CASE("PathFilter Logic", "[filter]") {
    RuntimeConfig config;

    SECTION("Manual Ignore patterns with slash") {
        config.ignorePatterns = {"temp/"};
        PathFilter filter(config);

        CHECK(filter.isIgnored("temp", true));
        CHECK(filter.isIgnored("temp/data.txt", false));
        CHECK_FALSE(filter.isIgnored("src/main.cpp", false));
    }

    SECTION("Default Hidden Rules and .gitignore") {
        PathFilter filter(config);

        CHECK(filter.isIgnored(".git", true));
        CHECK(filter.isIgnored(".git/config", false));

        CHECK_FALSE(filter.isIgnored(".gitignore", false));
        CHECK(filter.isIgnored(".idea", true));
    }

    SECTION("Only patterns filtering") {
        config.onlyPatterns = {"src/*", "include/"};
        PathFilter filter(config);

        CHECK_FALSE(filter.isIgnored("src/app.cpp", false));
        CHECK_FALSE(filter.isIgnored("include/header.h", false));
        CHECK(filter.isIgnored("tests/test.cpp", false));
    }
}