#include <catch2/catch_all.hpp>
#include <nlohmann/json.hpp>
import std;
import codeclipper;

using json = nlohmann::json;
namespace fs = std::filesystem;

class McpEnvFixture {
public:
    fs::path root;
    McpEnvFixture() {
        root = fs::temp_directory_path() / "mcp_test_env";
        std::error_code ec;
        fs::remove_all(root, ec);
        fs::create_directories(root);

        std::ofstream(root / "hello.txt") << "Hello World";
        std::ofstream(root / "ignore.me") << "Secret";
    }
    ~McpEnvFixture() {
        std::error_code ec;
        fs::remove_all(root, ec);
    }
};

TEST_CASE("MCP Server Protocol", "[mcp]") {
    codeclipper::McpServer server;
    std::stringstream input;
    std::stringstream output;

    SECTION("Initialize Handshake") {
        json req = {{"jsonrpc", "2.0"}, {"method", "initialize"}, {"id", 1}, {"params", json::object()}};
        input << req.dump() << "\n";
        server.run(input, output);

        auto resp = json::parse(output.str());
        CHECK(resp["id"] == 1);
        CHECK(resp["result"]["serverInfo"]["name"] == "codeclipper-mcp");
    }

    SECTION("List Tools") {
        json req = {{"jsonrpc", "2.0"}, {"method", "tools/list"}, {"id", 2}};
        input << req.dump() << "\n";
        server.run(input, output);

        auto resp = json::parse(output.str());
        CHECK(resp["id"] == 2);
        CHECK(resp["result"]["tools"][0]["name"] == "clip_context");
    }
}

TEST_CASE("MCP Server: Call Tool Integration", "[mcp]") {
    McpEnvFixture fix;
    codeclipper::McpServer server;
    std::stringstream input;
    std::stringstream output;

    SECTION("Successful Context Clip with Ignore Pattern") {
        json req = {
            {"jsonrpc", "2.0"},
            {"method", "tools/call"},
            {"id", 10},
            {"params", {
                {"name", "clip_context"},
                {"arguments", {
                    {"path", fix.root.string()},
                    {"ignore_patterns", json::array({"*.me"})}
                }}
            }}
        };
        input << req.dump() << "\n";
        server.run(input, output);

        auto resp = json::parse(output.str());
        CHECK(resp["id"] == 10);
        CHECK_FALSE(resp["result"]["isError"]);

        std::string content = resp["result"]["content"][0]["text"];
        CHECK(content.contains("hello.txt"));
        CHECK(content.contains("Hello World"));
        CHECK_FALSE(content.contains("ignore.me"));
    }

    SECTION("Error on Invalid Path") {
        json req = {
            {"jsonrpc", "2.0"},
            {"method", "tools/call"},
            {"id", 11},
            {"params", {
                {"name", "clip_context"},
                {"arguments", {{"path", "/path/to/invalid/dir"}}}
            }}
        };
        input << req.dump() << "\n";
        server.run(input, output);

        auto resp = json::parse(output.str());
        CHECK(resp["result"]["isError"] == true);
    }
}