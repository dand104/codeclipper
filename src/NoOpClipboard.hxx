#ifndef CODECLIPPER_NOOPCLIPBOARD_HXX
#define CODECLIPPER_NOOPCLIPBOARD_HXX

#include "IClipboard.hxx"

namespace codeclipper {
    class NoOpClipboard : public IClipboard {
    public:
        common::Result copy(std::string_view) override { return {}; }
        common::Result clear() override { return {}; }
    };
}

#endif //CODECLIPPER_NOOPCLIPBOARD_HXX