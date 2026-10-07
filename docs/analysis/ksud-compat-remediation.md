# KernelSU CLI compatibility remediation

## Root cause

The observed failure is at the KernelSU userspace handoff, not at the vulnerability privilege boundary:

```text
uid=0/euid=0
error: unexpected argument '--kmi' found
Usage: ksud late-load [OPTIONS]
late-load exit=2
```

GhostLock unconditionally invoked:

```text
ksud late-load --kmi <KMI> --allow-shell
```

The project does not bundle a `ksud` binary. `AndroidGhostlockRepository` copies `libksud.so` from an installed KernelSU-family manager, and the native root script then selects that staged file. Therefore the runtime CLI is supplied by the installed manager, not pinned by this repository.

Authoritative upstream source comparison shows the interface changed:

* KernelSU current `userspace/ksud/src/cli.rs` defines both `--kmi` and `--allow-shell` for `late-load`.
* KernelSU v3.2.1 and v3.2.0 define `late-load` without those two options; their implementation auto-detects the current KMI and loads the corresponding embedded module.
* The official module guide states that `ksud late-load` detects the current KMI and loads the corresponding `kernelsu.ko`.

Consequently, the observed `unexpected argument '--kmi'` is a concrete CLI compatibility failure: GhostLock expected the newer explicit-KMI/allow-shell interface while the selected installed `ksud` exposes the auto-KMI interface. The exact installed release cannot be identified from the repository alone because no runtime binary or version output was supplied.

The KMI parsing itself was not the primary failure for this observation: argument parsing terminated before KernelSU could consume or validate the KMI. The module-loading failure is independent of the initial exploit privilege result.

## Remediation

The root handoff now:

1. Uses only the staged `HOME_DIR/ksud` or the known installed manager library locations.
2. Removes writable temporary and generic `/data/adb/ksu/bin/ksud` fallbacks.
3. Captures and preserves `ksud late-load --help` output in the per-run log.
4. Accepts only two verified capability shapes:
   * both `--kmi` and `--allow-shell`: invoke the explicit interface;
   * neither option: invoke `ksud late-load` and let KernelSU auto-detect KMI.
5. Rejects help failures, malformed help, and mixed option states without attempting an unknown command line.
6. Treats a non-zero late-load exit as a module-load failure and never reports it as success.
7. Emits separate result markers for exploit completion, privilege boundary, CLI compatibility, module loading, and final security state.

No exploit primitive, SELinux bypass, seccomp bypass, kernel profile, or privilege acquisition behavior was changed.

## Security boundary

* **Stage A — before exploitation:** the app runs in its normal Android/Shizuku-controlled process context; no KernelSU module is assumed to be loaded.
* **Stage B — after the vulnerability primitive:** the vulnerable route can repair credentials and related task state, subject to the selected kernel profile and W1/W2/W3 verification.
* **Stage C — worker:** a successful W2 result is logged as a child reaching `uid=0,euid=0`; W3/seccomp clearance is recorded separately. UID 0 is not treated as proof that KernelSU initialized.
* **Stage D — KernelSU:** successful `ksud` compatibility, accepted late-load execution, and observed `kernelsu` module presence are required before `KernelSU ready` is emitted. Otherwise the final state is `temporary-root-only` or an explicit failure.

## Files changed

* `src/core/attack/ops.cpp` — strict ksud source selection, help-driven interface selection, fail-closed late-load handling, and root-side result markers.
* `src/core/session/root_child_frontend.cpp` — explicit exploit, privilege-boundary, module-load, and final-security result logging.
* `src/core/session/ksud_cli_compat.hpp` — compatibility classification contract.
* `src/core/session/ksud_cli_compat.cpp` — exact-option classifier used by host regression coverage.
* `src/core/tests/ksud_cli_compat_test.cpp` — vectors for current explicit interface, auto-KMI interface, missing/malformed help, mixed options, and similarly named options.
* `src/Makefile` — builds the classifier and regression test and includes it in `native-host-tests`.
* `.github/workflows/build.yml` — runs native regression coverage before the native build and verifies/publishes the APK SHA-256 checksum.

## Tests

Passed locally:

* `ksud_cli_compat_test: ok`
* `handoff_probe_test: ok`
* `host_attack_dataflow_test: ok`

The local sandbox does not have Cargo, CMake, or an Android NDK installed, so the full Android/native release build must be validated by GitHub Actions.

## CI and artifact

CI run, job results, artifact filename, and APK SHA-256 are intentionally left blank until the remediation branch is pushed and its pull-request workflow completes. The workflow now verifies that the APK is non-empty, passes `unzip -t`, and writes `GhostLock-release.apk.sha256`.

## Remaining risk

The installed KernelSU/ReSukiSU/KowSU manager and kernel must still be mutually compatible, and the selected `ksud` must contain the target KMI asset. A CLI-compatible invocation can still fail later because of missing KMI assets, kernel-side load errors, permissions, SELinux state, or vendor-specific restrictions. Those failures remain visible and are not converted into success.
