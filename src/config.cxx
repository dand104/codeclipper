export module codeclipper:config;
import std;

namespace codeclipper {
    export namespace fs = std::filesystem;

    export enum class ErrorCode {
        Success = 0,
        PathNotFound,
        AccessDenied,
        ClipboardFailure,
        Unknown
    };

    export template <typename T = void>
    using Result = std::expected<T, ErrorCode>;

    export struct RuntimeConfig {
        fs::path rootPath = fs::current_path();
        std::vector<std::string> onlyPatterns;
        std::vector<std::string> ignorePatterns;
        bool useClipboard = true;
        bool listOnly = false;
        bool verbose = false;
        bool ignoreGitIgnore = false;
        bool runMcpServer = false;
    };
}