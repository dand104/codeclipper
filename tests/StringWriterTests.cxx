#include <catch2/catch_all.hpp>
#include "StringWriter.hxx"

using namespace codeclipper;

TEST_CASE("StringWriter: Accumulation", "[adapter]") {
    StringWriter writer;

    SECTION("Writes formatted header and content") {
        writer.writeHeader("src/main.cpp");
        writer.writeContent("int main() {}");

        std::string output = writer.getString();

        CHECK(output.find("```path=src/main.cpp") != std::string::npos);
        CHECK(output.find("int main() {}") != std::string::npos);
        CHECK(output.find("```\n") != std::string::npos);
    }

    SECTION("Clear buffer") {
        writer.writeLine("Test");
        CHECK_FALSE(writer.getString().empty());

        writer.clear();
        CHECK(writer.getString().empty());
    }
}