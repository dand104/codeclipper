#ifndef CODECLIPPER_IFILTER_HXX
#define CODECLIPPER_IFILTER_HXX

#include "Common.hxx"

namespace codeclipper {

    class IFilter {
    public:
        virtual ~IFilter() = default;

        virtual void pushDirectory(const common::fs::path& dir) = 0;

        virtual void popDirectory() = 0;

        [[nodiscard]] virtual bool isIgnored(const common::fs::path& relativePath, bool isDirectory) const = 0;
    };
}

#endif //CODECLIPPER_IFILTER_HXX