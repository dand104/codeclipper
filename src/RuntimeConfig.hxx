#ifndef CODECLIPPER_RUNTIMECONFIG_HXX
#define CODECLIPPER_RUNTIMECONFIG_HXX

#include <string>
#include <vector>
#include <filesystem>
#include "Common.hxx"

namespace codeclipper {

    struct RuntimeConfig {
        common::fs::path rootPath;

        std::vector<std::string> onlyPatterns;
        std::vector<std::string> ignorePatterns;

        bool useClipboard = true;
        bool listOnly = false;
        bool verbose = false;

        bool ignoreGitIgnore = false;
    };
}

#endif //CODECLIPPER_RUNTIMECONFIG_HXX