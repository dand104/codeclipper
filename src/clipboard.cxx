module;
#include <clip.h>

export module codeclipper:clipboard;

import std;
import :config;

namespace codeclipper::clipboard {

    export [[nodiscard]] Result<> copy(std::string_view text) {
        if (clip::set_text(std::string(text))) {
            return {};
        }
        return std::unexpected(ErrorCode::ClipboardFailure);
    }

    export [[nodiscard]] Result<> clear() {
        if (clip::clear()) {
            return {};
        }
        return std::unexpected(ErrorCode::ClipboardFailure);
    }

} // namespace codeclipper::clipboard