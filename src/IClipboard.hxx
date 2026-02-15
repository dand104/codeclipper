#ifndef CODECLIPPER_ICLIPBOARD_HXX
#define CODECLIPPER_ICLIPBOARD_HXX

#include <string_view>
#include "Common.hxx"

namespace codeclipper {

    class IClipboard {
    public:
        virtual ~IClipboard() = default;

        virtual common::Result copy(std::string_view content) = 0;
        virtual common::Result clear() = 0;
    };
}

#endif //CODECLIPPER_ICLIPBOARD_HXX