#include "session/ksud_cli_compat.hpp"

namespace ghostlock::session {
    namespace {
        bool has_option(std::string_view help, std::string_view option) noexcept {
            size_t pos = 0;
            while ((pos = help.find(option, pos)) != std::string_view::npos) {
                const size_t end = pos + option.size();
                const bool left_ok = pos == 0 || help[pos - 1] == ' ' ||
                                     help[pos - 1] == '\t' || help[pos - 1] == '\n';
                const bool right_ok = end == help.size() || help[end] == ' ' ||
                                      help[end] == '\t' || help[end] == '\n' ||
                                      help[end] == '=';
                if (left_ok && right_ok) return true;
                pos = end;
            }
            return false;
        }
    }

    KsudLateLoadInterface classify_ksud_late_load_help(
            std::string_view help) noexcept {
        if (help.empty() || help.find("late-load") == std::string_view::npos)
            return KsudLateLoadInterface::Unsupported;
        const bool has_kmi = has_option(help, "--kmi");
        const bool has_allow_shell = has_option(help, "--allow-shell");
        if (has_kmi && has_allow_shell)
            return KsudLateLoadInterface::ExplicitKmiAllowShell;
        if (!has_kmi && !has_allow_shell)
            return KsudLateLoadInterface::AutoKmi;
        return KsudLateLoadInterface::Unsupported;
    }
}
