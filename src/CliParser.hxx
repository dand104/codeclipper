#ifndef CODECLIPPER_CLIPARSER_HXX
#define CODECLIPPER_CLIPARSER_HXX

#include <CLI/CLI.hpp>
#include "RuntimeConfig.hxx"

namespace codeclipper {

    class CliParser {
    public:
        explicit CliParser(int argc, char** argv);

        RuntimeConfig getConfig() const;
        bool shouldExit() const;
        int getExitCode() const;

    private:
        CLI::App m_app{"CodeClipper"};
        RuntimeConfig m_config;
        int m_exitCode = 0;
        bool m_shouldExit = false;

        void setup();
    };
}

#endif //CODECLIPPER_CLIPARSER_HXX