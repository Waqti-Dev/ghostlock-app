#ifndef GHOSTLOCK_KSUD_CLI_COMPAT_HPP
#define GHOSTLOCK_KSUD_CLI_COMPAT_HPP

#include <string_view>

namespace ghostlock::session {
    enum class KsudLateLoadInterface {
        Unsupported,
        AutoKmi,
        ExplicitKmiAllowShell,
    };

    /* Classify only the documented late-load options exposed by ksud --help.
     * Mixed or malformed interfaces are intentionally rejected. */
    [[nodiscard]] KsudLateLoadInterface classify_ksud_late_load_help(
            std::string_view help) noexcept;
}

#endif
