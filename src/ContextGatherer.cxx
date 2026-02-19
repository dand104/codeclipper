#include "ContextGatherer.hxx"
#include <iostream>
#include <utility>

namespace codeclipper {

    ContextGatherer::ContextGatherer(
        RuntimeConfig config,
        std::shared_ptr<IFileSystem> fs,
        std::shared_ptr<IFilter> filter,
        std::shared_ptr<IClipboard> clipboard,
        std::shared_ptr<IWriter> writer
    ) : m_config(std::move(config)),
        m_fs(std::move(fs)), 
        m_filter(std::move(filter)), 
        m_clipboard(std::move(clipboard)), 
        m_writer(std::move(writer)) 
    {}

    common::Result ContextGatherer::execute() {
        if (m_config.verbose) {
            m_writer->writeLine("[Info] Starting traversal at: " + m_config.rootPath.string());
        }

        auto onEnter = [&](const common::fs::path& relPath) {
            if (!relPath.empty() && m_filter->isIgnored(relPath, true)) {
                if (m_config.verbose) m_writer->writeLine("[Skip Dir] " + relPath.string());
                return false;
            }

            m_filter->pushDirectory(m_config.rootPath / relPath);
            return true;
        };

        auto onExit = [&](const common::fs::path&) {
            m_filter->popDirectory();
            return true;
        };

        auto onFile = [&](const common::FileEntry& entry) {
            processFile(entry);
            return true;
        };

        auto res = m_fs->traverse(m_config.rootPath, onEnter, onExit, onFile);

        if (!res) return res;

        m_writer->flush();

        if (m_config.useClipboard && !m_config.listOnly) {
            if (!m_clipboardBuffer.empty()) {
                auto clipRes = m_clipboard->copy(m_clipboardBuffer);
                if (clipRes) {
                    if (m_config.verbose) m_writer->writeLine("[Info] Copied to clipboard.");
                } else {
                    m_writer->writeLine("[Error] Failed to copy to clipboard.");
                }
            } else {
                 if (m_config.verbose) m_writer->writeLine("[Info] Nothing to copy.");
            }
        }

        return {};
    }

    void ContextGatherer::processFile(const common::FileEntry& entry) {
        if (m_filter->isIgnored(entry.relativePath, false)) {
            if (m_config.verbose) {
                  m_writer->writeLine("[Ignore] " + entry.relativePath.string());
            }
            return;
        }

        if (m_config.listOnly) {
            m_writer->writeLine(entry.relativePath.string());
            return;
        }

        auto content = m_fs->readFile(entry.absolutePath);
        if (content) {
            m_writer->writeHeader(entry.relativePath);
            m_writer->writeContent(*content);

            if (m_config.useClipboard) {
                std::string p = common::pathToString(entry.relativePath);
                std::ranges::replace(p, '\\', '/');
                m_clipboardBuffer += "```path=" + p + "\n";

                m_clipboardBuffer += *content + "\n```\n";
            }
        } else {
            if (m_config.verbose) {
                m_writer->writeLine("[Skip Binary/Empty] " + common::pathToString(entry.relativePath));
            }
        }
    }
}