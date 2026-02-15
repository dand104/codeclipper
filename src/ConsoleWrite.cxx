#include "ConsoleWrite.hxx"
#include <fmt/core.h>
#include <iostream>

namespace codeclipper {

    ConsoleWrite::ConsoleWrite() {
        std::ios::sync_with_stdio(false);
        std::cin.tie(nullptr);
    }

    void ConsoleWrite::writeHeader(const common::fs::path& relativePath) {
        fmt::print("```path={}\n", relativePath.string());
    }

    void ConsoleWrite::writeContent(std::string_view content) {
        fmt::print("{}\n```\n", content);
    }

    void ConsoleWrite::writeLine(std::string_view text) {
        fmt::print("{}\n", text);
    }

    void ConsoleWrite::flush() {
        std::cout << std::flush;
    }
}