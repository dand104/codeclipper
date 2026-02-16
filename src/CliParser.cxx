#include "CliParser.hxx"
#include <iostream>

namespace codeclipper {

    CliParser::CliParser(int argc, char** argv) {
        setup();
        try {
            m_app.parse(argc, argv);
        } catch (const CLI::ParseError& e) {
            m_exitCode = m_app.exit(e);
            m_shouldExit = true;
        }
    }

    void CliParser::setup() {
        m_app.description("CodeClipper: Copy project context to clipboard.");

        // Root directory argument
        m_app.add_option("path", m_config.rootPath, "Path to the project root directory")
             ->default_val(common::fs::current_path().string());

        // -i / --ignore
        m_app.add_option("-i,--ignore", m_config.ignorePatterns, "Ignore file patterns (glob)");

        // -o / --only (Keep only)
        m_app.add_option("-o,--only", m_config.onlyPatterns, "Process ONLY these file patterns");

        // Flags
        m_app.add_flag("--no-copy", [&](auto count){ m_config.useClipboard = (count == 0); })
             ->description("Do not copy to clipboard (output to console only)");
        
        m_app.add_flag("-l,--list", m_config.listOnly, "List files only, do not copy content");
        
        m_app.add_flag("-v,--verbose", m_config.verbose, "Enable verbose logging");
        
        m_app.add_flag("--no-gitignore", m_config.ignoreGitIgnore, "Do not read .gitignore files");
    }

    RuntimeConfig CliParser::getConfig() const {
        return m_config;
    }

    bool CliParser::shouldExit() const {
        return m_shouldExit;
    }

    int CliParser::getExitCode() const {
        return m_exitCode;
    }
}