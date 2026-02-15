#include "FileSystem.hxx"
#include <fstream>
#include <algorithm>
#include <iostream>
#include <system_error>

namespace codeclipper {

    common::Result FileSystem::traverse(
        const common::fs::path& root,
        const DirectoryCallback onEnterDir,
        const DirectoryCallback onExitDir,
        const FileCallback onFile
    ) const {
        std::error_code ec;
        if (!common::fs::exists(root, ec)) return std::unexpected(common::ErrorCode::PathNotFound);

        const auto absRoot = common::fs::canonical(root, ec);
        if (ec) return std::unexpected(common::ErrorCode::AccessDenied);

        visitDirectory(absRoot, absRoot, onEnterDir, onExitDir, onFile);

        return {};
    }

    void FileSystem::visitDirectory(
        const common::fs::path& currentDir,
        const common::fs::path& rootDir,
        const DirectoryCallback& onEnterDir,
        const DirectoryCallback& onExitDir,
        const FileCallback& onFile
    ) {
        common::fs::path relPath;
        if (currentDir != rootDir) {
            relPath = common::fs::relative(currentDir, rootDir);
        }

        if (!onEnterDir(relPath)) return;

        try {
            for (const auto& entry : common::fs::directory_iterator(currentDir)) {

                const auto& entryAbs = entry.path();
                const auto entryRel = relPath.empty() ? entryAbs.filename() : relPath / entryAbs.filename();

                if (entry.is_directory()) {
                    visitDirectory(entryAbs, rootDir, onEnterDir, onExitDir, onFile);
                } else if (entry.is_regular_file()) {
                    common::FileEntry fEntry{
                        .relativePath = entryRel,
                        .absolutePath = entryAbs,
                        .size = entry.file_size()
                    };
                    if (!onFile(fEntry)) break;
                }
            }
        } catch (const common::fs::filesystem_error& e)
        {
            if (e.code() != std::errc::permission_denied) {
                std::cerr << "[FS Warning] " << e.what() << std::endl;
            }
        }

        (void)onExitDir(relPath);
    }

    std::optional<std::string> FileSystem::readFile(const common::fs::path& path) const {
        std::ifstream file(path, std::ios::in | std::ios::binary);
        if (!file) return std::nullopt;

        file.seekg(0, std::ios::end);
        const auto size = file.tellg();
        file.seekg(0, std::ios::beg);

        if (size <= 0) return "";
        if (size > 50 * 1024 * 1024) return std::nullopt; // Skip > 50 MB

        std::string buffer(static_cast<size_t>(size), '\0');
        if (file.read(buffer.data(), size)) {
            if (isBinary(buffer)) {
                return std::nullopt;
            }
            return buffer;
        }
        return std::nullopt;
    }

    bool FileSystem::isBinary(const std::string& content) {
        const size_t checkLen = std::min(content.size(), static_cast<size_t>(4096));
        for (size_t i = 0; i < checkLen; ++i) {
            if (content[i] == '\0') return true;
        }
        return false;
    }
}