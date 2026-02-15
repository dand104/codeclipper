#ifndef CODECLIPPER_FILESYSTEM_HXX
#define CODECLIPPER_FILESYSTEM_HXX

#include "IFileSystem.hxx"

namespace codeclipper {
    class FileSystem : public IFileSystem {
    public:
        common::Result traverse(
            const common::fs::path& root,
            DirectoryCallback onEnterDir,
            DirectoryCallback onExitDir,
            FileCallback onFile
        ) const override;

        std::optional<std::string> readFile(const common::fs::path& path) const override;

    private:
        static void visitDirectory(
            const common::fs::path& currentDir,
            const common::fs::path& rootDir,
            const DirectoryCallback& onEnterDir,
            const DirectoryCallback& onExitDir,
            const FileCallback& onFile
        );

        static bool isBinary(const std::string& content);
    };
}

#endif //CODECLIPPER_FILESYSTEM_HXX