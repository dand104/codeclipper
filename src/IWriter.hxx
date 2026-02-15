#ifndef CODECLIPPER_IWRITER_HXX
#define CODECLIPPER_IWRITER_HXX

#include <string_view>
#include "Common.hxx"

namespace codeclipper {

    class IWriter {
    public:
        virtual ~IWriter() = default;

        virtual void writeHeader(const common::fs::path& relativePath) = 0;

        virtual void writeContent(std::string_view content) = 0;

        virtual void writeLine(std::string_view text) = 0;

        virtual void flush() = 0;
    };
}

#endif //CODECLIPPER_IWRITER_HXX