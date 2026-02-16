#include <catch2/catch_all.hpp>
#include "ContextGatherer.hxx"
#include "IFilter.hxx"
#include "IFileSystem.hxx"
#include "IWriter.hxx"
#include "IClipboard.hxx"

using namespace codeclipper;

class MockWriter : public IWriter {
public:
    std::vector<std::string> lines;
    void writeHeader(const common::fs::path& p) override { lines.push_back("H:" + p.string()); }
    void writeContent(std::string_view c) override { lines.push_back("C:" + std::string(c)); }
    void writeLine(std::string_view t) override { lines.push_back(std::string(t)); }
    void flush() override {}
};

class MockClipboard : public IClipboard {
public:
    std::string captured;
    common::Result copy(std::string_view c) override { captured = c; return {}; }
    common::Result clear() override { return {}; }
};

class PassThroughFilter : public IFilter {
public:
    void pushDirectory(const common::fs::path&) override {}
    void popDirectory() override {}
    bool isIgnored(const common::fs::path&, bool) const override { return false; }
};

class SingleFileFS : public IFileSystem {
public:
    common::Result traverse(const common::fs::path& root, DirectoryCallback onEnter, 
                           DirectoryCallback onExit, FileCallback onFile) const override {
        onEnter("");
        onFile({.relativePath = "test.txt", .absolutePath = root / "test.txt", .size = 4});
        onExit("");
        return {};
    }
    std::optional<std::string> readFile(const common::fs::path&) const override {
        return "data";
    }
};

TEST_CASE("ContextGatherer: Clipboard Formatting", "[integration]") {
    RuntimeConfig config;
    config.useClipboard = true;
    
    auto writer = std::make_shared<MockWriter>();
    auto clip = std::make_shared<MockClipboard>();
    auto fs = std::make_shared<SingleFileFS>();
    auto filter = std::make_shared<PassThroughFilter>();

    ContextGatherer gatherer(config, fs, filter, clip, writer);
    auto res = gatherer.execute();

    REQUIRE(res.has_value());
    
    CHECK(clip->captured.find("```path=test.txt") != std::string::npos);
    CHECK(clip->captured.find("data") != std::string::npos);
}

TEST_CASE("ContextGatherer: List-only Mode", "[integration]") {
    RuntimeConfig config;
    config.listOnly = true;
    
    auto writer = std::make_shared<MockWriter>();
    auto clip = std::make_shared<MockClipboard>();
    auto fs = std::make_shared<SingleFileFS>();
    auto filter = std::make_shared<PassThroughFilter>();

    ContextGatherer gatherer(config, fs, filter, clip, writer);
    gatherer.execute();

    CHECK(writer->lines.size() == 1);
    CHECK(writer->lines[0] == "test.txt");
    CHECK(clip->captured.empty());
}