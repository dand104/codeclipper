#include "GitIgnoreFilter.hxx"
#include <fstream>
#include <algorithm>

namespace codeclipper {

    static bool globMatch(const std::string_view str, const std::string_view pattern) {
        if (pattern.empty()) return str.empty();
        if (pattern == "*") return true;

        if (pattern[0] == '*') {
            return globMatch(str, pattern.substr(1)) ||
                   (!str.empty() && globMatch(str.substr(1), pattern));
        }
        if (!str.empty() && (pattern[0] == '?' || pattern[0] == str[0])) {
            return globMatch(str.substr(1), pattern.substr(1));
        }
        return false;
    }

    GitIgnoreFilter::GitIgnoreFilter(const RuntimeConfig& config)
        : m_config(config) {}

    void GitIgnoreFilter::pushDirectory(const common::fs::path& dir) {
        std::vector<IgnoreRule> rules;
        if (!m_config.ignoreGitIgnore) {
            loadGitIgnore(dir);
        } else {
            m_rulesStack.emplace_back();
        }
    }

    void GitIgnoreFilter::popDirectory() {
        if (!m_rulesStack.empty()) {
            m_rulesStack.pop_back();
        }
    }

    void GitIgnoreFilter::loadGitIgnore(const common::fs::path& dir) {
        std::vector<IgnoreRule> currentRules;
        const auto gitignorePath = dir / ".gitignore";

        if (common::fs::exists(gitignorePath)) {
            std::ifstream file(gitignorePath);
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

    bool GitIgnoreFilter::checkOnlyPatterns(const std::string& pathStr) const {
        if (m_config.onlyPatterns.empty()) return true;

        return std::ranges::any_of(m_config.onlyPatterns, [&](const auto& pat) {
            if (globMatch(pathStr, pat)) return true;
            if (pathStr.starts_with(pat + "/")) return true;
            if (pat.starts_with(pathStr + "/")) return true;

            return false;
        });
    }


    bool GitIgnoreFilter::checkManualIgnorePatterns(const std::string& pathStr) const {
        return std::ranges::any_of(m_config.ignorePatterns, [&](const auto& pat) {
            return globMatch(pathStr, pat);
        });
    }

    bool GitIgnoreFilter::checkGitDirectory(const std::string& pathStr) {
        return pathStr.starts_with(".git") || pathStr.find("/.git") != std::string::npos;
    }

    bool GitIgnoreFilter::isIgnored(const common::fs::path& relativePath, const bool isDirectory) const {
        if (relativePath.empty() || relativePath == ".") return false;

        const std::string pathStr = relativePath.generic_string();

        if (m_config.onlyPatterns.empty()) {
            for (const auto& part : relativePath) {
                std::string s = part.string();
                if (s.starts_with('.') && s != "." && s != ".." && s != ".gitignore") {
                    return true;
                }
            }
        }
        if (!checkOnlyPatterns(pathStr)) return true;
        if (checkManualIgnorePatterns(pathStr)) return true;
        if (pathStr.starts_with(".git") || pathStr.find("/.git") != std::string::npos) {
            return true;
        }

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

    bool GitIgnoreFilter::matchRule(const common::fs::path& path, const std::string& pathStr, bool isDirectory, const IgnoreRule& rule) {
        if (rule.pattern.empty()) return false;

        if (rule.pattern.find('/') != std::string::npos) {
            std::string p = rule.pattern;
            if (p.starts_with('/')) p = p.substr(1);

            if (globMatch(pathStr, p)) return true;
            if (rule.isDirectoryOnly && pathStr.starts_with(p + "/")) return true;
            return false;
        }
        return std::ranges::any_of(path, [&](const auto& part) {
            return globMatch(part.string(), rule.pattern);
        });
    }
}
