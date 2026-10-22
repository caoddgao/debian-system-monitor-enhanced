# Final test report — 2026-10-20

## Environment and provenance

- Host: Debian 13.7, GNOME Wayland session, original `gnome-system-monitor 48.1-2`.
- Source: Debian `gnome-system-monitor 48.1-2` `.dsc`, `48.1.orig.tar.xz`, and `48.1-2.debian.tar.xz` downloaded with `apt-get source gnome-system-monitor=48.1-2`. Debian patches are applied in `gnome-system-monitor-48.1/`; the source and archives are retained.
- New package: `system-monitor-enhanced 48.1-2+enhanced1`, amd64, built from this C++/GTK4/gtkmm/libadwaita source. The old Python implementation is not shipped.

## Automated and package checks

| Check | Result |
| --- | --- |
| Meson setup and compile | Passed; upstream source and added C++ modules link successfully |
| Meson tests | 3/3 passed: upstream unit tests, desktop validation, metainfo validation |
| Earlier Python collector tests | 6/6 passed before the superseded prototype was removed |
| C++ hardware probe compilation | Passed with `g++ -std=gnu++20 -Wall -Wextra` |
| Package path collision with original | 756 packaged paths checked; zero original package file collisions |
| `.deb` install | Passed through `apt-get install` with Polkit authentication |
| `.deb` removal | Passed through `apt-get remove`; enhanced binary disappeared, original package stayed installed |
| `.deb` reinstall | Passed; final host state has both packages installed |
| GSettings and D-Bus names | Both original and enhanced schema families present; enhanced owned `io.github.renping.SystemMonitorEnhanced` on the user bus |

The `.deb` includes a separate executable, launcher, icon names, resources, settings schema, policy actions, helpers, translations and help directory. It contains original `COPYING` and Debian `copyright` files. Package SHA-256: `9824bfe4ac642fecb7b9986414489d0d94787f2e36c8d5689f213c430a38308b`.

## Live hardware checks

All numbers below were read on the actual host via sysfs or NVML, not generated example values. They vary over time.

| Device or metric | Observed |
| --- | --- |
| CPU model | AMD Ryzen 7 7700X 8-Core Processor |
| CPU aggregate curve and label | One `LoadGraph` CPU series; live label observed between 1.8% and 5.8% |
| Mean CPU current frequency | 3.26–3.99 GHz observed |
| CPU k10temp Tctl | 41.0–47.875 °C observed |
| DDR5 DIMM temperatures | Two `spd5118` readings, around 41.8–42 °C |
| Samsung 990 PRO 2 TB, serial `S73HNJ0W604010P` | 32.9 °C |
| Samsung 990 PRO 4 TB, serial `S7DPNJ0YB04394E` | 30.9 °C |
| Samsung 990 PRO 2 TB, serial `S7DRNJ0X504402P` | 39.9–41.85 °C |
| Toshiba MQ04ABF1, WWID `naa.500003995b6163fa` | N/A; no readable sysfs temperature was exposed |
| RTX 5090 NVML | 0% utilization, about 425.5 MB / 34.2 GB VRAM, 33 °C, 195 MHz, 0% fan, P8; power 3.915–5.11 W observed |
| AMD Radeon Graphics | 0% utilization, 600 MHz, 38–39 °C, driver-reported VRAM and GTT shared system memory both readable |

The NVIDIA device nodes exist on the host and the C++ NVML probe succeeded outside the sandbox. During the enhanced app's Resources session, `nvidia-smi` still reported **P8, 0% fan, 0% GPU use, about 4.43–5.11 W**. After the app closed it reported **P8, 0% fan, 4.46 W**. The driver and GPU settings were not changed. A longer controlled idle-power study was not performed, so these observations do not prove every possible idle workload.

## GNOME interface checks

- The enhanced app launched in the real GNOME Wayland session for 12 seconds without startup errors, then ran persistently for interactive inspection. GTK accessibility exposed the `System Monitor Enhanced` window and Resources, Processes, and File Systems tabs.
- Resources showed one `Total CPU` percentage, model, frequency and Tctl; original Memory and Swap usage, Network receive/send rates and totals, and Disk read/write rates and totals; two DDR5 values; the four identified disk temperature rows; RTX 5090 metrics; and AMD metrics. The memory expander was opened and its live values verified.
- Processes displayed the original Name, CPU, ID, Memory, Disk Read and Disk Write sorting headers. File Systems displayed Device, Directory, Type, Total, Available and Used, with live `/` and `/boot/efi` rows. Navigation among all three tabs worked and returned to Resources.
- Debian's original `gnome-system-monitor` also launched in the same GNOME session after the enhanced package was installed. Both packages were installed together and the original stayed installed through the enhanced removal/reinstall cycle.
- GNOME Shell denied the external ScreenshotWindow D-Bus request (`AccessDenied`), so a screenshot and pixel-level visual comparison were unavailable. Curve implementation is the original `LoadGraph` component; GPU curves receive live utilization samples. Destructive process actions (kill/renice) were retained in source but not exercised against user processes. A long unattended GUI soak test was not performed.

## Limits and expected N/A behavior

The HDD exposes a stable WWID but no readable temperature in the inspected sysfs interfaces, so N/A is correct. NVIDIA NVML data is N/A inside the restricted sandbox because `/dev/nvidia*` is hidden there; it was successfully tested on the host after approved execution. AMD clock parsing uses the current `pp_dpm_sclk` entry and `freq1_input`; unavailable driver fields remain N/A. GTT is explicitly shared system memory. Memory module labels are ordinal readings of the two `spd5118` sensors; no slot identity is asserted.

## V3.1 interface update — 2026-10-20

- Built and installed `system-monitor-enhanced 48.1-2+enhanced2`; original `gnome-system-monitor 48.1-2` remains installed. Package SHA-256: `e925bd96d08b7756d164cf6982d8300d76fdc817482e0ab4546c9e024469722a`.
- Meson tests passed 3/3, including new exact boundary cases for temperature and capacity colors. Package path collision check passed for 756 files.
- In the real GNOME session, the enhanced window launched without console errors. The accessibility tree showed Resources, Processes, and File Systems, and the Resources sections in order: CPU, Memory and Swap, NVIDIA RTX 5090, AMD Radeon Graphics, Network, Disk.
- Live title values included CPU 2.5%, AMD Ryzen 7 7700X, 3.27 GHz, 41.0°C; DIMM1 41.0°C and DIMM2 41.2°C; NVIDIA 32.0°C, 6.1 W, 0.0%; AMD 38.0°C, 0.0%. CPU total uses aggregate libgtop counters while the graph uses 16 per-core counters. The schema now provides 16 distinct curve colors.
- The disk list exposed three Samsung SSD model/serial/temperature pairs and Toshiba HDD `N/A` as separate labels. Source layout sets the left caption to top alignment with the disk rows. The accessibility API did not provide usable screen coordinates for a pixel-level alignment check.
- During the live Resources session, NVIDIA remained at P8, 0% fan, 0% GPU use, about 6.24 W. This is a spot check, not a long idle-power study.
- Temperature color levels are UI warnings and do not represent manufacturer safety limits; no official limit is asserted here. The thresholds are documented in the project README.

## CPU legend follow-up — 2026-10-20

- The original GNOME System Monitor CPU legend was restored below the graph: four columns of colored CPU1–CPU16 squares, live per-thread percentages, and per-thread color pickers. The aggregate CPU reading and hardware details remain in the title row.
- `meson test` passed 3/3 and the package collision check passed for 756 files. The installed package is `system-monitor-enhanced 48.1-2+enhanced3`; original `gnome-system-monitor 48.1-2` remains installed. Package SHA-256: `07391cfce5f4d96e298c2b226af5c36c5c83e29164530fadde6845b7d54ff320`.
- The first launch after installation reached an already-running older process. After quitting it and launching the new binary, the real GNOME accessibility tree showed CPU1 through CPU16 with distinct live percentages beneath the unchanged CPU title metrics.
- NVIDIA Performance state remains visible because P8 is a useful read-only indication of a lower-power state. Its value now has a tooltip explaining that it is not a safety warning.

## Section spacing and DIMM readability follow-up — 2026-10-20

- Added 20 px top margin before Memory and Swap and before NVIDIA RTX 5090, leaving the spacing of the other sections unchanged.
- Changed the Memory title labels to separate `DIMM-1` and `DIMM-2` identifiers from their live temperature labels with a fixed 12 px gap. Only the temperature labels carry temperature status color.
- Meson tests passed 3/3; package collision check passed for 756 files. The installed enhanced package is `48.1-2+enhanced4`, SHA-256 `f13c74c72098ea84f707c7740b2c788ee9ee7c75b26bfc812e03c12f3df4644e`. The original `gnome-system-monitor 48.1-2` remains installed.
- A real GNOME session launch exposed `Memory and Swap`, `DIMM-1`, a live temperature, `DIMM-2`, and a second live temperature as separate accessible labels. CPU1–CPU16 and the remaining Resources sections were still present.

## Toshiba ATA SMART temperature follow-up — 2026-10-20

- Earlier `N/A` was caused by the collector leaving every `sd*` disk temperature unset, rather than an unavailable drive sensor.
- On the host, UDisks reported `SmartSupported=true`, `SmartEnabled=true`, a nonzero `SmartUpdated`, and `SmartTemperature=296.15 K` for `TOSHIBA MQ04ABF100` (`/dev/sda`). This is 23.0 °C. UDisks documents the property in Kelvin and zero as unknown.
- The collector now maps each `sd*` block object to its UDisks Drive object, reads the cached Drive.Ata SMART properties without calling `SmartUpdate`, checks the timestamp and plausible range, and converts Kelvin to Celsius. This avoids an explicit disk wake-up request. The reading may refresh less frequently than the application's five-second UI sample interval.
- The read-only host probe returned `TOSHIBA MQ04ABF1 [naa.500003995b6163fa] 23`, while the three Samsung NVMe readings remained available. The real GNOME Resources accessibility tree then showed the Toshiba disk row at `23.0°C`.
- Meson tests passed 3/3; package collision check passed for 756 files. `system-monitor-enhanced 48.1-2+enhanced5` is installed alongside original `gnome-system-monitor 48.1-2`. Package SHA-256: `75d6b135c1a09cc009791f546db6d69dedc81dd8407b55a2ccaae125cf56e1b0`.

## Repository cleanup — 2026-10-20

- Removed the superseded Python/GTK prototype, its old packaging entry point and launcher, its Python tests, and old staging directories. The current Meson build output was removed as regenerable cache; `packaging/build-fork.sh` now runs `meson setup` if needed.
- The installed C++ application and the latest `enhanced5` `.deb` were kept. Original GNOME System Monitor remains installed as a separate application.

## Original package removal — 2026-10-20

- Built `system-monitor-enhanced 48.1-2+enhanced6` with a versioned `Provides: gnome-system-monitor (= 48.1-2)` so the installed GNOME desktop metapackages retain their dependency. Meson tests passed 3/3 before installation.
- Installed `enhanced6`, then simulated removing the original package. APT proposed only `gnome-system-monitor`; the actual removal likewise removed only that package, freeing 5989 kB.
- Verified `gnome`, `gnome-core`, and `task-gnome-desktop` remain installed. `apt-get -s autoremove` proposed zero removals. `/usr/bin/system-monitor-enhanced` exists and `/usr/bin/gnome-system-monitor` is absent.
- Kept only the latest `enhanced6` installer in `dist/` and removed the regenerable Meson build directory. Package SHA-256: `0b52d5b6c353e5aa1041ea899ce448ee0eff090cf5df76a70712e84f6963efc3`.

## Memory configured transfer rate — 2026-10-22

- Added memory type and configured transfer rate after both DIMM temperatures in the Memory and Swap title row. The host displays `DDR5 5200`; its two populated modules report `CONFIGURED_SPEED_MTS=5200` and nominal `SPEED_MTS=5600`. The nominal rating is not substituted for the configured rate. Tooltip text identifies MT/s and BIOS/SMBIOS as the source and explains that this is not a live clock reading.
- The collector uses libsystemd's sd-device API to read udev's cached DMI properties without privileged reads or helper processes. Empty slots are skipped. Missing configured rates show N/A, identical module descriptions appear once, and differing configurations are listed separately. Builds with systemd support disabled return no memory configuration. The build dependency now requires libsystemd >=251 for sd-device support.
- Full Meson compilation succeeded and all 3 Meson test suites passed. Added cases cover identical and differing module configurations, unavailable rates/types, and zero, malformed, and overflowing firmware integers. `git diff --check` passed.
- The live collector probe returned `memory_speed=DDR5 5200`. It can be built with `g++ -std=gnu++20 -Wall -Wextra -Ignome-system-monitor-48.1/build gnome-system-monitor-48.1/src/test/hardware-probe.cpp gnome-system-monitor-48.1/src/hardware.cpp -o /tmp/system-monitor-memory-probe $(pkg-config --cflags --libs gio-2.0 libsystemd) -ldl -pthread` after Meson setup.
- Built and installed `system-monitor-enhanced 48.1-2+enhanced7`; APT upgraded only this package. Packaging checked 756 staged paths; the original package remains absent. Package SHA-256: `b5601915b9bd4b0b0c923d12b94b59d72702746b3211fb45d6ea19f0aada95ac`.
- Started the installed application in the GNOME session. The accessibility tree confirmed this exact sibling order in the title row: `Memory and Swap`, a group containing `DIMM-1` and `36.5°C`, a group containing `DIMM-2` and `37.0°C`, then `DDR5 5200`. Temperatures are a live spot reading. Pixel-level rendering was not inspected.

## Publication preparation — 2026-10-22

- Removed the regenerable Meson build directory, the superseded enhanced6 installer, unused Quilt bookkeeping, the upstream GNOME-specific GitLab pipeline, and the downloaded source archives already represented by the full source tree. The current enhanced7 installer remains available locally under ignored `dist/`.
- Moved the signed Debian source descriptor to `docs/upstream/` and recorded the original archive checksums in `docs/upstream/PROVENANCE.md`. Original source, translations, help, copyright, and component license notices are retained.
- Rewrote the project README in Chinese and English, identifying Debian Linux as the target platform and GNOME System Monitor 48.1 / Debian 48.1-2 as the original project. Both introductions link directly to the original authors' repository.
- Moved hardware collection details to `docs/HARDWARE.md`. Checked local documentation links, packaging script syntax, and whitespace. Publication preparation did not change the compiled application; its enhanced7 build, tests, installation, and live UI validation are recorded above.
