#ifndef CODECLIPPER_IFILESYSTEM_HXX
#define CODECLIPPER_IFILESYSTEM_HXX

#include <functional>
#include "Common.hxx"

namespace codeclipper {

    using FileCallback = std::function<bool(const common::FileEntry&)>;
    using DirectoryCallback = std::function<bool(const common::fs::path&)>;

    class IFileSystem {
    public:
        virtual ~IFileSystem() = default;

        virtual common::Result traverse(
            const common::fs::path& root,
            DirectoryCallback onEnterDir,
            DirectoryCallback onExitDir,
            FileCallback onFile
        ) const = 0;

        virtual std::optional<std::string> readFile(const common::fs::path& path) const = 0;
    };
}

#endif //CODECLIPPER_IFILESYSTEM_HXX