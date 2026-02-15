#ifndef CODECLIPPER_DEFINITIONS_HXX
#define CODECLIPPER_DEFINITIONS_HXX

#include <string>
#include <filesystem>
#include <vector>
#include <expected>
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

    using Result = std::expected<void, ErrorCode>;
}
#endif //CODECLIPPER_DEFINITIONS_HXX