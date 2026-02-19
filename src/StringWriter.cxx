#include "StringWriter.hxx"
#include <fmt/core.h>

namespace codeclipper {

    void StringWriter::writeHeader(const common::fs::path& relativePath) {
        std::string p = common::pathToString(relativePath);
        std::ranges::replace(p, '\\', '/');
        m_buffer << "```path=" << p << "\n";    }

    void StringWriter::writeContent(std::string_view content) {
        m_buffer << content << "\n```\n";
    }

    void StringWriter::writeLine(std::string_view text) {
        m_buffer << text << "\n";
    }

    void StringWriter::flush() {
    }

    std::string StringWriter::getString() const {
        return m_buffer.str();
    }

    void StringWriter::clear() {
        m_buffer.str("");
        m_buffer.clear();
    }
}