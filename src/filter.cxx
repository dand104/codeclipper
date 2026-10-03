export module codeclipper:filter;
import std;
import :config;

namespace codeclipper {

    bool globMatch(std::string_view str, std::string_view pattern) {
        size_t s = 0, p = 0, lastWildcardIdx = std::string_view::npos, sBacktrackIdx = 0;

        while (s < str.size()) {
            if (p < pattern.size() && (pattern[p] == '?' || pattern[p] == str[s])) {
                ++s; ++p;
            } else if (p < pattern.size() && pattern[p] == '*') {
                lastWildcardIdx = p++;
                sBacktrackIdx = s;
            } else if (lastWildcardIdx != std::string_view::npos) {
                p = lastWildcardIdx + 1;
                s = ++sBacktrackIdx;
            } else {
                return false;
            }
        }
        while (p < pattern.size() && pattern[p] == '*') ++p;
        return p == pattern.size();
    }

    export class PathFilter {
    public:
        explicit PathFilter(const RuntimeConfig& config) : m_config(config) {
            m_normalizedOnlyPatterns.reserve(config.onlyPatterns.size());
            for (const auto& pat : config.onlyPatterns) {
                m_normalizedOnlyPatterns.push_back(normalizePattern(pat));
            }
            m_normalizedIgnorePatterns.reserve(config.ignorePatterns.size());
            for (const auto& pat : config.ignorePatterns) {
                m_normalizedIgnorePatterns.push_back(normalizePattern(pat));
            }
        }

        void pushDirectory(const fs::path& dir) {
            if (m_config.ignoreGitIgnore) {
                m_rulesStack.emplace_back();
                return;
            }
            loadGitIgnore(dir);
        }

        void popDirectory() {
            if (!m_rulesStack.empty()) {
                m_rulesStack.pop_back();
            }
        }

        [[nodiscard]] bool isIgnored(const fs::path& relativePath, bool isDirectory) const {
            if (relativePath.empty() || relativePath == ".") return false;

            const std::string pathStr = relativePath.generic_string();

            if (pathStr == ".git" || pathStr.starts_with(".git/")) {
                return true;
            }

            if (m_config.onlyPatterns.empty()) {
                for (const auto& part : relativePath) {
                    const std::string s = part.generic_string();
                    if (s.starts_with('.') && s != "." && s != ".." && s != ".gitignore") {
                        return true;
                    }
                }
            }

            if (!checkOnlyPatterns(pathStr)) return true;
            if (checkManualIgnorePatterns(pathStr)) return true;

            bool ignored = false;
            for (const auto& level : m_rulesStack) {
                for (const auto& rule : level) {
                    if (matchRule(relativePath, pathStr, isDirectory, rule)) {
                        ignored = !rule.isNegation;
                    }
                }
            }
            return ignored;
        }

    private:
        struct IgnoreRule {
            std::string pattern;
            bool isNegation{false};
            bool isDirectoryOnly{false};
        };

        const RuntimeConfig& m_config;
        std::vector<std::string> m_normalizedOnlyPatterns;
        std::vector<std::string> m_normalizedIgnorePatterns;
        std::vector<std::vector<IgnoreRule>> m_rulesStack;

        static std::string normalizePattern(std::string_view rawPattern) {
            std::string p(rawPattern);
            std::ranges::replace(p, '\\', '/');

            auto [first, last] = std::ranges::unique(p, [](char a, char b) {
                return a == '/' && b == '/';
            });
            p.erase(first, last);

            if (p.starts_with("./")) {
                p = p.substr(2);
            }
            return p;
        }

        void loadGitIgnore(const fs::path& dir) {
            std::vector<IgnoreRule> currentRules;
            const auto gitignorePath = dir / ".gitignore";

            if (std::ifstream file(gitignorePath); file.is_open()) {
                std::string line;
                while (std::getline(file, line)) {
                    const auto start = line.find_first_not_of(" \t");
                    if (start == std::string::npos || line[start] == '#') continue;
                    const auto end = line.find_last_not_of(" \t\r\n");
                    line = line.substr(start, end - start + 1);

                    if (line.empty()) continue;

                    IgnoreRule rule;
                    rule.isNegation = (line[0] == '!');
                    if (rule.isNegation) {
                        line = line.substr(1);
                    }

                    rule.isDirectoryOnly = (line.back() == '/');
                    if (rule.isDirectoryOnly) {
                        line.pop_back();
                    }

                    rule.pattern = line;
                    currentRules.push_back(rule);
                }
            }
            m_rulesStack.push_back(std::move(currentRules));
        }

        [[nodiscard]] bool checkOnlyPatterns(const std::string& pathStr) const {
            if (m_config.onlyPatterns.empty()) return true;
            return std::ranges::any_of(m_normalizedOnlyPatterns, [&](const auto& pat) {
                if (pat == "." || pat == "./") return true;
                if (globMatch(pathStr, pat)) return true;
                if (pathStr.starts_with(pat) && (pat.back() == '/' || pathStr[pat.size()] == '/')) return true;
                if (pat.starts_with(pathStr + "/")) return true;
                return false;
            });
        }

        [[nodiscard]] bool checkManualIgnorePatterns(const std::string& pathStr) const {
            return std::ranges::any_of(m_normalizedIgnorePatterns, [&](const auto& pat) {
                if (pat.empty()) return false;
                if (pat.back() == '/') {
                    if (pathStr.starts_with(pat)) return true;
                    std::string folder = pat;
                    folder.pop_back();
                    if (pathStr == folder) return true;
                }
                return globMatch(pathStr, pat);
            });
        }

        [[nodiscard]] static bool matchRule(const fs::path& path, const std::string& pathStr, bool isDirectory, const IgnoreRule& rule) {
            if (rule.pattern.empty()) return false;

            if (rule.pattern.contains('/')) {
                std::string p = rule.pattern;
                if (p.starts_with('/')) p = p.substr(1);

                if (globMatch(pathStr, p)) return true;
                if (rule.isDirectoryOnly && pathStr.starts_with(p + "/")) return true;
                return false;
            }

            return std::ranges::any_of(path, [&](const auto& part) {
                return globMatch(part.generic_string(), rule.pattern);
            });
        }
    };
}