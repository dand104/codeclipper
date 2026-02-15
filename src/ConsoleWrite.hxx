#ifndef CODECLIPPER_CONSOLEWRITE_HXX
#define CODECLIPPER_CONSOLEWRITE_HXX

#include "IWriter.hxx"

namespace codeclipper {
    class ConsoleWrite : public IWriter {
    public:
        ConsoleWrite();
        void writeHeader(const common::fs::path& relativePath) override;
        void writeContent(std::string_view content) override;
        void writeLine(std::string_view text) override;
        void flush() override;
    };
}

#endif //CODECLIPPER_CONSOLEWRITE_HXX