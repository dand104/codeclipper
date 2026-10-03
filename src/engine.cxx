export module codeclipper:engine;

import std;
import :config;
import :filter;
import :clipboard;

namespace codeclipper {

    export class ContextEngine {
    public:
        ContextEngine(RuntimeConfig config, std::ostream& out)
            : m_config(std::move(config)), 
              m_out(out), 
              m_filter(m_config) 
        {}

        Result<> run() {
            std::error_code ec;

            if (!fs::exists(m_config.rootPath, ec)) {
                return std::unexpected(ErrorCode::PathNotFound);
            }

            const auto absRoot = fs::canonical(m_config.rootPath, ec);
            if (ec) {
                return std::unexpected(ErrorCode::AccessDenied);
            }

            if (m_config.verbose) {
                std::println(m_out, "[Info] Starting traversal at: {}", absRoot.generic_string());
            }

            auto traverse = [&](auto& self, const fs::path& currentDir) -> void {
                fs::path relDir;
                if (currentDir != absRoot) {
                    relDir = fs::relative(currentDir, absRoot, ec);
                    if (ec) {
                        if (m_config.verbose) {
                            std::println(m_out, "[FS Error] Failed to calculate relative path for: {}", currentDir.generic_string());
                        }
                        return;
                    }
                }

                if (!relDir.empty() && m_filter.isIgnored(relDir, true)) {
                    if (m_config.verbose) {
                        std::println(m_out, "[Skip Dir] {}", relDir.generic_string());
                    }
                    return;
                }

                m_filter.pushDirectory(currentDir);

                try {
                    for (const auto& entry : fs::directory_iterator(
                             currentDir, 
                             fs::directory_options::skip_permission_denied)) 
                    {
                        const auto& entryAbs = entry.path();
                        const auto entryRel = relDir.empty() 
                            ? entryAbs.filename() 
                            : relDir / entryAbs.filename();

                        if (entry.is_directory(ec)) {
                            if (!entry.is_symlink(ec)) {
                                self(self, entryAbs);
                            }
                        } else if (entry.is_regular_file(ec)) {
                            processFile(entryRel, entryAbs);
                        }
                    }
                } catch (const fs::filesystem_error& e) {
                    if (m_config.verbose) {
                        std::println(m_out, "[FS Warning] Error traversing {}: {}", currentDir.generic_string(), e.what());
                    }
                }

                m_filter.popDirectory();
            };
            traverse(traverse, absRoot);
            m_out.flush();

            if (m_config.useClipboard && !m_config.listOnly) {
                if (!m_clipboardBuffer.empty()) {
                    auto clipRes = clipboard::copy(m_clipboardBuffer);
                    if (clipRes) {
                        if (m_config.verbose) {
                            std::println(m_out, "[Info] Copied to clipboard.");
                        }
                    } else {
                        std::println(m_out, "[Error] Failed to copy to clipboard.");
                        return clipRes;
                    }
                } else {
                    if (m_config.verbose) {
                        std::println(m_out, "[Info] Nothing to copy.");
                    }
                }
            }

            return {};
        }

    private:
        RuntimeConfig m_config;
        std::ostream& m_out;
        PathFilter m_filter;
        std::string m_clipboardBuffer;

        void processFile(const fs::path& relPath, const fs::path& absPath) {
            if (m_filter.isIgnored(relPath, false)) {
                if (m_config.verbose) {
                    std::println(m_out, "[Ignore] {}", relPath.generic_string());
                }
                return;
            }

            if (m_config.listOnly) {
                std::println(m_out, "{}", relPath.generic_string());
                return;
            }

            if (auto content = readFile(absPath)) {
                const std::string pathStr = relPath.generic_string();

                std::println(m_out, "```path={}\n{}\n```", pathStr, *content);

                if (m_config.useClipboard) {
                    m_clipboardBuffer += std::format("```path={}\n{}\n```\n", pathStr, *content);
                }
            } else {
                if (m_config.verbose) {
                    std::println(m_out, "[Skip Binary/Empty] {}", relPath.generic_string());
                }
            }
        }

        [[nodiscard]] static bool isBinary(std::string_view content) noexcept {
            const size_t checkLen = std::min(content.size(), static_cast<size_t>(4096));
            return content.substr(0, checkLen).find('\0') != std::string_view::npos;
        }

        [[nodiscard]] static std::optional<std::string> readFile(const fs::path& path) {
            std::ifstream file(path, std::ios::in | std::ios::binary | std::ios::ate);
            if (!file) {
                return std::nullopt;
            }

            const auto size = file.tellg();
            if (size == -1) {
                return std::nullopt;
            }
            if (size == 0) {
                return "";
            }
            if (size > 50 * 1024 * 1024) {
                return std::nullopt;
            }

            std::string buffer(static_cast<size_t>(size), '\0');
            file.seekg(0, std::ios::beg);
            
            if (file.read(buffer.data(), size)) {
                if (isBinary(buffer)) {
                    return std::nullopt;
                }
                return buffer;
            }

            return std::nullopt;
        }
    };

} // namespace codeclipper