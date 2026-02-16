#ifndef CODECLIPPER_STRINGWRITER_HXX
#define CODECLIPPER_STRINGWRITER_HXX

#include "IWriter.hxx"
#include <sstream>

namespace codeclipper {
    class StringWriter : public IWriter {
    public:
        void writeHeader(const common::fs::path& relativePath) override;
        void writeContent(std::string_view content) override;
        void writeLine(std::string_view text) override;
        void flush() override;

        [[nodiscard]] std::string getString() const;
        void clear();

    private:
        std::stringstream m_buffer;
    };
}

#endif //CODECLIPPER_STRINGWRITER_HXX