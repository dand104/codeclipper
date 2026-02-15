#include <iostream>
#include "CliParser.hxx"
#include "ContextGatherer.hxx"
#include "GitIgnoreFilter.hxx"
#include "FileSystem.hxx"
#include "ConsoleWrite.hxx"
#include "SystemClipboard.hxx"

int main(int argc, char** argv) {
    using namespace codeclipper;

    const CliParser parser(argc, argv);
    if (parser.shouldExit()) {
        return parser.getExitCode();
    }

    auto config = parser.getConfig();

    if (!common::fs::exists(config.rootPath)) {
        std::cerr << "Error: Path does not exist: " << config.rootPath.string() << std::endl;
        return 1;
    }
    config.rootPath = common::fs::canonical(config.rootPath);

    auto fs = std::make_shared<FileSystem>();
    auto writer = std::make_shared<ConsoleWrite>();
    auto clipboard = std::make_shared<SystemClipboard>();
    auto filter = std::make_shared<GitIgnoreFilter>(config);

    const auto app = std::make_shared<ContextGatherer>(
        config,
        fs,
        filter,
        clipboard,
        writer
    );

    auto result = app->execute();

    if (!result) {
        std::cerr << "Runtime Error Code: " << static_cast<int>(result.error()) << std::endl;
        return 1;
    }

    return 0;
}