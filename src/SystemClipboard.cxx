#include "SystemClipboard.hxx"
#include <clip.h>
#include <iostream>

namespace codeclipper {

    common::Result SystemClipboard::copy(const std::string_view content) {
        const std::string str(content);
        if (clip::set_text(str)) {
            return {};
        }
        return tl::unexpected(common::ErrorCode::ClipboardFailure);
    }

    common::Result SystemClipboard::clear() {
        if (clip::clear()) {
            return {};
        }
        return tl::unexpected(common::ErrorCode::ClipboardFailure);
    }
}