#include <catch2/catch_all.hpp>
#include <nlohmann/json.hpp>
#include <sstream>
#include <filesystem>
#include <fstream>
#include "McpServer.hxx"

using namespace codeclipper;
using json = nlohmann::json;
namespace fs = std::filesystem;

class McpEnvFixture {
public:
    fs::path root;
    McpEnvFixture() {
        root = fs::temp_directory_path() / "mcp_test_env";
        if (fs::exists(root)) fs::remove_all(root);
        fs::create_directories(root);
        
        std::ofstream(root / "hello.txt") << "Hello World";
        std::ofstream(root / "ignore.me") << "Secret";
    }
    ~McpEnvFixture() { fs::remove_all(root); }
};

TEST_CASE("MCP Server Protocol", "[mcp]") {
    McpServer server;
    std::stringstream input;
    std::stringstream output;

    SECTION("Initialize Handshake") {
        json req = {
            {"jsonrpc", "2.0"},
            {"method", "initialize"},
            {"id", 1},
            {"params", {}}
        };
        input << req.dump() << "\n";

        server.run(input, output);

        std::string rawStr = output.str();
        auto resp = json::parse(std::string_view(rawStr));

        CHECK(resp["id"] == 1);
        CHECK(resp["result"]["serverInfo"]["name"] == "codeclipper-mcp");
    }

    SECTION("List Tools") {
        json req = {
            {"jsonrpc", "2.0"},
            {"method", "tools/list"},
            {"id", 2}
        };
        input << req.dump() << "\n";

        server.run(input, output);

        auto resp = json::parse(output.str());
        CHECK(resp["id"] == 2);
        
        auto tools = resp["result"]["tools"];
        CHECK(tools.is_array());
        CHECK(tools[0]["name"] == "clip_context");
    }
}

TEST_CASE("MCP Server: Call Tool Integration", "[mcp]") {
    McpEnvFixture fix;
    McpServer server;
    std::stringstream input;
    std::stringstream output;

    SECTION("Successfull Context Clip") {
        json req = {
            {"jsonrpc", "2.0"},
            {"method", "tools/call"},
            {"id", 10},
            {"params", {
                {"name", "clip_context"},
                {"arguments", {
                    {"path", fix.root.string()},
                    {"ignore_patterns", {"*.me"}}
                }}
            }}
        };
        input << req.dump() << "\n";

        server.run(input, output);

        std::string rawOutput = output.str();
        INFO("Raw Output: " << rawOutput);
        auto resp = json::parse(rawOutput);

        CHECK(resp["id"] == 10);
        CHECK_FALSE(resp["result"]["isError"]);
        
        std::string content = resp["result"]["content"][0]["text"];
        
        CHECK(content.find("hello.txt") != std::string::npos);
        CHECK(content.find("Hello World") != std::string::npos);
        
        CHECK(content.find("ignore.me") == std::string::npos);
    }

    SECTION("Error on Invalid Path") {
        json req = {
            {"jsonrpc", "2.0"},
            {"method", "tools/call"},
            {"id", 11},
            {"params", {
                {"name", "clip_context"},
                {"arguments", {
                    {"path", "/path/to/nowhere/ghost_dir"}
                }}
            }}
        };
        input << req.dump() << "\n";

        server.run(input, output);
        
        auto resp = json::parse(output.str());
        CHECK(resp["result"]["isError"] == true);
        
        std::string content = resp["result"]["content"][0]["text"];
        CHECK(content.find("Error: Path does not exist") != std::string::npos);
    }
}