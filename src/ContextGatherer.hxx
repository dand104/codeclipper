#ifndef CODECLIPPER_CONTEXTGATHERER_HXX
#define CODECLIPPER_CONTEXTGATHERER_HXX

#include <memory>
#include "RuntimeConfig.hxx"
#include "IFilter.hxx"
#include "IFileSystem.hxx"
#include "IClipboard.hxx"
#include "IWriter.hxx"

namespace codeclipper {

    class ContextGatherer {
    public:
        ContextGatherer(
            RuntimeConfig  config,
            std::shared_ptr<IFileSystem> fs,
            std::shared_ptr<IFilter> filter,
            std::shared_ptr<IClipboard> clipboard,
            std::shared_ptr<IWriter> writer
        );

        common::Result execute();

    private:
        RuntimeConfig m_config;
        std::shared_ptr<IFileSystem> m_fs;
        std::shared_ptr<IFilter> m_filter;
        std::shared_ptr<IClipboard> m_clipboard;
        std::shared_ptr<IWriter> m_writer;

        std::string m_clipboardBuffer;

        void processFile(const common::FileEntry& entry);
    };
}

#endif //CODECLIPPER_CONTEXTGATHERER_HXX