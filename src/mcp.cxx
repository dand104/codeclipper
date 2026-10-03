module;
#include <nlohmann/json.hpp>

export module codeclipper:mcp;

import std;
import :config;
import :engine;

namespace codeclipper {

    using json = nlohmann::json;

    export class McpServer {
    public:
        void run(std::istream& in, std::ostream& out) {
            std::string line;
            while (std::getline(in, line)) {
                if (line.empty()) continue;
                try {
                    auto msg = json::parse(line);
                    handleMessage(msg, out);
                } catch (const std::exception& e) {
                    std::println(std::cerr, "[MCP Error] Parse failed: {}", e.what());
                }
            }
        }

    private:
        void handleMessage(const json& msg, std::ostream& out) {
            if (!msg.contains("jsonrpc") || msg["jsonrpc"] != "2.0") return;
            if (!msg.contains("id")) return;

            auto id = msg["id"];
            std::string method = msg.value("method", "");

            try {
                json result;
                if (method == "initialize") {
                    result = {
                        {"protocolVersion", "2024-11-05"},
                        {"capabilities", {{"tools", json::object()}}},
                        {"serverInfo", {{"name", "codeclipper-mcp"}, {"version", "1.0.0"}}}
                    };
                } else if (method == "tools/list") {
                    result = {
                        {"tools", {{
                            {"name", "clip_context"},
                            {"description", "Gathers code context from a local directory respecting .gitignore."},
                            {"inputSchema", {
                                {"type", "object"},
                                {"properties", {
                                    {"path", {
                                        {"type", "string"},
                                        {"description", "Root path to scan"}
                                    }},
                                    {"only_patterns", {
                                        {"type", "array"},
                                        {"items", {{"type", "string"}}}
                                    }},
                                    {"ignore_patterns", {
                                        {"type", "array"},
                                        {"items", {{"type", "string"}}}
                                    }},
                                    {"verbose", {
                                        {"type", "boolean"}
                                    }}
                                }}
                            }}
                        }}}
                    };
                } else if (method == "tools/call") {
                    result = handleToolCall(msg.value("params", json{}));
                } else if (method == "notifications/initialized") {
                    return;
                } else {
                    sendError(out, id, -32601, "Method not found");
                    return;
                }
                sendResponse(out, id, result);
            } catch (const std::exception& e) {
                sendError(out, id, -32000, e.what());
            }
        }

        json handleToolCall(const json& params) {
            std::string name = params.value("name", "");
            if (name != "clip_context") {
                throw std::runtime_error("Unknown tool: " + name);
            }

            auto args = params.value("arguments", json{});
            RuntimeConfig config;

            if (args.contains("path")) {
                config.rootPath = args["path"].get<std::string>();
            } else {
                config.rootPath = fs::current_path();
            }

            if (args.contains("only_patterns")) {
                for (const auto& p : args["only_patterns"]) {
                    config.onlyPatterns.push_back(p.get<std::string>());
                }
            }

            if (args.contains("ignore_patterns")) {
                for (const auto& p : args["ignore_patterns"]) {
                    config.ignorePatterns.push_back(p.get<std::string>());
                }
            }

            config.verbose = args.value("verbose", false);
            config.useClipboard = false;
            config.listOnly = false;

            std::error_code ec;
            if (!fs::exists(config.rootPath, ec)) {
                return {
                    {"content", {{
                        {"type", "text"},
                        {"text", "Error: Path does not exist: " + config.rootPath.generic_string()}
                    }}},
                    {"isError", true}
                };
            }

            std::ostringstream ss;
            ContextEngine engine(config, ss);
            auto res = engine.run();

            return {
                {"content", {{
                    {"type", "text"},
                    {"text", ss.str()}
                }}},
                {"isError", !res.has_value()}
            };
        }

        void sendResponse(std::ostream& out, const json& id, const json& result) {
            json resp = {
                {"jsonrpc", "2.0"},
                {"id", id},
                {"result", result}
            };
            out << resp.dump() << "\n";
        }

        void sendError(std::ostream& out, const json& id, int code, const std::string& message) {
            json resp = {
                {"jsonrpc", "2.0"},
                {"id", id},
                {"error", {
                    {"code", code},
                    {"message", message}
                }}
            };
            out << resp.dump() << "\n";
        }
    };

} // namespace codeclipper