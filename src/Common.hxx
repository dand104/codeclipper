#ifndef CODECLIPPER_DEFINITIONS_HXX
#define CODECLIPPER_DEFINITIONS_HXX

#include <string>
#include <filesystem>
#include <vector>
#include <tl/expected.hpp>
#include <optional>

namespace codeclipper::common {

    namespace fs = std::filesystem;

    struct FileEntry {
        fs::path relativePath;
        fs::path absolutePath;
        uintmax_t size;
    };

    enum class ErrorCode {
        Success = 0,
        PathNotFound,
        AccessDenied,
        ClipboardFailure,
        Unknown
    };

    inline std::string pathToString(const fs::path& p) {
        const auto u8str = p.u8string();
        return std::string{reinterpret_cast<const char*>(u8str.c_str())};
    }

    using Result = tl::expected<void, ErrorCode>;
}
#endif //CODECLIPPER_DEFINITIONS_HXX