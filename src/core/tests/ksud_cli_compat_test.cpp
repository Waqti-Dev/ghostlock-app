#include "session/ksud_cli_compat.hpp"

#include <cassert>
#include <cstdio>

using ghostlock::session::KsudLateLoadInterface;
using ghostlock::session::classify_ksud_late_load_help;

int main() {
    const char *current =
            "Usage: ksud late-load [OPTIONS]\n"
            "      --kmi <KMI>\n"
            "      --allow-shell\n";
    assert(classify_ksud_late_load_help(current) ==
           KsudLateLoadInterface::ExplicitKmiAllowShell);

    /* KernelSU v3.2.1 removed both explicit options and auto-detects KMI. */
    const char *auto_kmi =
            "Usage: ksud late-load [OPTIONS]\n"
            "      --magica [<MAGICA>]\n"
            "      --post-magica\n";
    assert(classify_ksud_late_load_help(auto_kmi) == KsudLateLoadInterface::AutoKmi);

    assert(classify_ksud_late_load_help(
                   "error: unexpected argument '--kmi' found\n") ==
           KsudLateLoadInterface::Unsupported);
    assert(classify_ksud_late_load_help(
                   "Usage: ksud late-load [OPTIONS]\n --kmi <KMI>\n") ==
           KsudLateLoadInterface::Unsupported);
    assert(classify_ksud_late_load_help(
                   "Usage: ksud late-load [OPTIONS]\n --allow-shell\n") ==
           KsudLateLoadInterface::Unsupported);
    assert(classify_ksud_late_load_help(
                   "Usage: ksud late-load [OPTIONS]\n --kmi-extra <value>\n") ==
           KsudLateLoadInterface::AutoKmi);
    assert(classify_ksud_late_load_help("") == KsudLateLoadInterface::Unsupported);

    std::puts("ksud_cli_compat_test: ok");
    return 0;
}
