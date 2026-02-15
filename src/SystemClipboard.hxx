#ifndef CODECLIPPER_SYSTEMCLIPBOARD_HXX
#define CODECLIPPER_SYSTEMCLIPBOARD_HXX

#include "IClipboard.hxx"

namespace codeclipper {
    class SystemClipboard : public IClipboard {
    public:
        common::Result copy(std::string_view content) override;
        common::Result clear() override;
    };
}

#endif //CODECLIPPER_SYSTEMCLIPBOARD_HXX