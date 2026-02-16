#include "McpServer.hxx"
#include "ContextGatherer.hxx"
#include "FileSystem.hxx"
#include "PathFilter.hxx"
#include "StringWriter.hxx"
#include "NoOpClipboard.hxx"

namespace codeclipper {

    void McpServer::run(std::istream& input, std::ostream& output) {
        std::string line;
        while (std::getline(input, line)) {
            if (line.empty()) continue;
            try {
                auto msg = json::parse(line);
                handleMessage(msg, output);
            } catch (const std::exception& e) {
                std::cerr << "[MCP Error] Parse failed: " << e.what() << std::endl;
            }
        }
    }

    void McpServer::handleMessage(const json& msg, std::ostream& output) {
        if (!msg.contains("jsonrpc") || msg["jsonrpc"] != "2.0") return;

        if (!msg.contains("id")) return;

        auto id = msg["id"];
        std::string method = msg["method"];

        try {
            json result;
            if (method == "initialize") {
                result = handleInitialize(msg.value("params", json{}));
            } else if (method == "tools/list") {
                result = handleListTools();
            } else if (method == "tools/call") {
                result = handleCallTool(msg.value("params", json{}));
            } else if (method == "notifications/initialized") {
                 return;
            } else {
                sendError(output, id, -32601, "Method not found");
                return;
            }
            sendResponse(output, id, result);
        } catch (const std::exception& e) {
            sendError(output, id, -32000, e.what());
        }
    }

    void McpServer::sendResponse(std::ostream& output, const json& id, const json& result) {
        json resp = {
            {"jsonrpc", "2.0"},
            {"id", id},
            {"result", result}
        };
        output << resp.dump() << std::endl;
    }

    void McpServer::sendError(std::ostream& output, const json& id, int code, const std::string& message) {
        json resp = {
            {"jsonrpc", "2.0"},
            {"id", id},
            {"error", {
                {"code", code},
                {"message", message}
            }}
        };
        output << resp.dump() << std::endl;
    }

    // ... (handleInitialize, handleListTools, handleCallTool остаются БЕЗ ИЗМЕНЕНИЙ) ...
    // Скопируйте реализацию этих методов из предыдущего ответа

    nlohmann::json McpServer::handleInitialize(const json&) {
        return {
            {"protocolVersion", "2024-11-05"},
            {"capabilities", {
                {"tools", json::object()}
            }},
            {"serverInfo", {
                {"name", "codeclipper-mcp"},
                {"version", "1.0.0"}
            }}
        };
    }

    nlohmann::json McpServer::handleListTools() {
        return {
            {"tools", {
                {
                    {"name", "clip_context"},
                    {"description", "Gathers code context from a local directory respecting .gitignore. Returns formatted content."},
                    {"inputSchema", {
                        {"type", "object"},
                        {"properties", {
                            {"path", {
                                {"type", "string"},
                                {"description", "Root path to scan (absolute or relative to CWD). Default: current dir."}
                            }},
                            {"only_patterns", {
                                {"type", "array"},
                                {"items", {{"type", "string"}}},
                                {"description", "List of glob patterns to include (whitelist)."}
                            }},
                            {"ignore_patterns", {
                                {"type", "array"},
                                {"items", {{"type", "string"}}},
                                {"description", "List of extra glob patterns to ignore."}
                            }},
                            {"verbose", {
                                {"type", "boolean"},
                                {"description", "Include logs in output."}
                            }}
                        }}
                    }}
                }
            }}
        };
    }

    nlohmann::json McpServer::handleCallTool(const json& params) {
        std::string name = params.value("name", "");
        auto args = params.value("arguments", json{});

        if (name != "clip_context") {
            throw std::runtime_error("Unknown tool: " + name);
        }

        RuntimeConfig config;

        if (args.contains("path")) {
            config.rootPath = args["path"].get<std::string>();
        } else {
            config.rootPath = common::fs::current_path();
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
        config.useClipboard = true;
        config.listOnly = false; 

        if (!common::fs::exists(config.rootPath)) {
             return {
                {"content", {
                    {
                        {"type", "text"},
                        {"text", "Error: Path does not exist: " + config.rootPath.string()}
                    }
                }},
                {"isError", true}
            };
        }
        config.rootPath = common::fs::canonical(config.rootPath);

        auto fs = std::make_shared<FileSystem>();
        auto writer = std::make_shared<StringWriter>();
        auto clipboard = std::make_shared<NoOpClipboard>();
        auto filter = std::make_shared<PathFilter>(config);

        ContextGatherer gatherer(config, fs, filter, clipboard, writer);

        auto result = gatherer.execute();

        bool isError = !result.has_value();
        std::string outputText = writer->getString();

        if (isError) {
            outputText += "\n\nError Code: " + std::to_string(static_cast<int>(result.error()));
        }

        return {
            {"content", {
                {
                    {"type", "text"},
                    {"text", outputText}
                }
            }},
            {"isError", isError}
        };
    }
}