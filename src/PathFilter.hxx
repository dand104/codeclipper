#ifndef CODECLIPPER_GITIGNOREFILTER_HXX
#define CODECLIPPER_GITIGNOREFILTER_HXX

#include <vector>
#include <stack>
#include "IFilter.hxx"
#include "RuntimeConfig.hxx"

namespace codeclipper {

    class PathFilter : public IFilter {
    public:
        explicit PathFilter(const RuntimeConfig& config);

        void pushDirectory(const common::fs::path& dir) override;
        void popDirectory() override;
        [[nodiscard]] bool isIgnored(const common::fs::path& relativePath, bool isDirectory) const override;

    private:
        const RuntimeConfig& m_config;
        std::vector<std::string> m_normalizedOnlyPatterns;
        std::vector<std::string> m_normalizedIgnorePatterns;
        struct IgnoreRule {
            std::string pattern;
            bool isNegation;
            bool isDirectoryOnly;
            common::fs::path baseDir;
        };

        std::vector<std::vector<IgnoreRule>> m_rulesStack;
        void loadGitIgnore(const common::fs::path& dir);

        static std::string normalizePattern(std::string_view rawPattern);
        [[nodiscard]] bool checkOnlyPatterns(const std::string& pathStr) const;
        [[nodiscard]] bool checkManualIgnorePatterns(const std::string& pathStr) const;
        [[nodiscard]] static bool checkGitDirectory(const std::string& pathStr) ;
        [[nodiscard]] static bool matchRule(const common::fs::path& path, const std::string& pathStr, bool isDirectory, const IgnoreRule& rule) ;
    };
}

#endif //CODECLIPPER_GITIGNOREFILTER_HXX