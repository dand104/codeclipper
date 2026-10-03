module;
#include <CLI/CLI.hpp>

export module codeclipper.cli;

import std;
import codeclipper;

namespace codeclipper {

    export class CliParser {
    public:
        CliParser(int argc, char** argv) {
            CLI::App app{"CodeClipper: Copy project context to clipboard."};

            app.add_flag("-m,--mcp", m_config.runMcpServer, "Run as MCP Server");

            std::string pathStr = fs::current_path().string();
            app.add_option("path", pathStr, "Path to root directory");

            app.add_option("-i,--ignore", m_config.ignorePatterns, "Ignore patterns (glob)");
            app.add_option("-o,--only", m_config.onlyPatterns, "Whitelist patterns");

            app.add_flag("--no-copy", [&](auto count) {
                m_config.useClipboard = (count == 0);
            })->description("Do not copy to clipboard (output to console only)");

            app.add_flag("-l,--list", m_config.listOnly, "List files only");
            app.add_flag("-v,--verbose", m_config.verbose, "Verbose logging");
            app.add_flag("--no-gitignore", m_config.ignoreGitIgnore, "Do not read .gitignore");

            try {
                app.parse(argc, argv);
                m_config.rootPath = pathStr;
            } catch (const CLI::ParseError& e) {
                m_exitCode = app.exit(e);
                m_shouldExit = true;
            }
        }

        [[nodiscard]] RuntimeConfig config() const noexcept { return m_config; }
        [[nodiscard]] bool shouldExit() const noexcept { return m_shouldExit; }
        [[nodiscard]] int exitCode() const noexcept { return m_exitCode; }

    private:
        RuntimeConfig m_config;
        int m_exitCode{0};
        bool m_shouldExit{false};
    };

} // namespace codeclipper