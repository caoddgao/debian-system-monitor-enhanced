# Debian System Monitor Enhanced

[中文](#中文说明) · [English](#english) · [构建与安装 / Build and install](#build-and-install)

## 中文说明

**面向 Debian Linux 的系统监控程序**，基于原版 **GNOME System Monitor（系统监视器）48.1** 改编，使用 Debian `gnome-system-monitor 48.1-2` 源码作为基础，增加硬件状态读取与显示。当前已在 **Debian 13、GNOME Wayland、amd64** 环境验证。

**原作者 / 上游项目仓库：[GNOME / gnome-system-monitor](https://gitlab.gnome.org/GNOME/gnome-system-monitor)**
官方项目页：[GNOME System Monitor](https://apps.gnome.org/SystemMonitor/)
Debian 源码打包仓库：[Debian GNOME / gnome-system-monitor](https://salsa.debian.org/gnome-team/gnome-system-monitor)

本项目是上述项目的衍生版本，保留原作者版权及许可证声明，并独立使用 `System Monitor Enhanced` 名称和应用 ID。

- 保留原版进程管理、资源历史曲线和文件系统页面。
- CPU 标题显示总占用率、型号、平均当前频率和 Tctl 温度；图下保留各逻辑处理器的曲线图例与占用率。
- 内存标题显示两条 DIMM 的温度，随后显示内存类型和 BIOS 配置运行速率，例如 `DDR5 5200`。速率单位为 MT/s，悬停可查看说明。
- 添加 NVIDIA RTX 5090、AMD Radeon Graphics 的硬件读数，以及 NVMe / ATA 磁盘温度。
- 硬件采集为只读操作，仅在资源页面激活时每 5 秒采样。无法读取的数据显示 `N/A`。

## English

**A system monitoring application for Debian Linux**, adapted from the original **GNOME System Monitor 48.1**, using Debian's `gnome-system-monitor 48.1-2` source package as its base. It adds hardware readings and extends the Resources page. Validated on **Debian 13, GNOME Wayland, amd64**.

**Original authors' / upstream repository: [GNOME / gnome-system-monitor](https://gitlab.gnome.org/GNOME/gnome-system-monitor)**
Official project page: [GNOME System Monitor](https://apps.gnome.org/SystemMonitor/)
Debian source packaging: [Debian GNOME / gnome-system-monitor](https://salsa.debian.org/gnome-team/gnome-system-monitor)

This is a derivative of the upstream project. Original copyright and license notices are retained. It uses its own `System Monitor Enhanced` name and application ID.

- Retains the original process management, resource history graphs, and File Systems page.
- Shows total CPU use, model, mean current frequency, and Tctl temperature in the CPU title row, with per-thread graph legends and usage below the graph.
- Shows two DIMM temperatures followed by memory type and the BIOS-configured transfer rate, such as `DDR5 5200`. The tooltip explains the MT/s unit.
- Adds hardware readings for NVIDIA RTX 5090 and AMD Radeon Graphics, plus NVMe / ATA disk temperatures.
- Collects hardware readings without changing hardware settings, every 5 seconds while Resources is active. Unavailable readings show `N/A`.

<a id="build-and-install"></a>
## 构建与安装 / Build and install

在 Debian 13 安装构建依赖。以下命令修改系统软件包，需要管理员权限。
Install build dependencies on Debian 13. These commands modify system packages and require administrator privileges.

```sh
sudo apt update
sudo apt install build-essential meson ninja-build pkg-config gettext appstream \
  desktop-file-utils libcatch2-dev itstool libglibmm-2.68-dev libgtkmm-4.0-dev \
  libgtop2-dev librsvg2-dev libxml2-dev libadwaita-1-dev libsystemd-dev \
  libgdk-pixbuf-2.0-dev dpkg-dev
```

构建 Debian 安装包；脚本会自动创建构建目录。
Build the Debian package; the script creates the build directory when needed.

```sh
./packaging/build-fork.sh
meson test -C gnome-system-monitor-48.1/build --print-errorlogs
```

安装并启动 / Install and launch:

```sh
sudo apt install ./dist/system-monitor-enhanced_48.1-2+enhanced7_amd64.deb
system-monitor-enhanced
```

卸载 / Uninstall:

```sh
sudo apt remove system-monitor-enhanced
```

安装包使用独立的程序、启动器、D-Bus 名称、GSettings、资源及图标路径，提供 `gnome-system-monitor (= 48.1-2)` 软件包能力。硬件支持取决于机器和驱动提供的接口。
The package uses separate executable, launcher, D-Bus, GSettings, resource, and icon paths. It provides the `gnome-system-monitor (= 48.1-2)` package capability. Hardware availability depends on the machine and driver interfaces.

## 目录与文档 / Layout and documentation

| 路径 / Path | 说明 / Description |
| --- | --- |
| `gnome-system-monitor-48.1/` | 完整 C++/GTK4 源码、翻译、帮助及原作者版权 / Complete source, translations, help, and original copyright |
| `packaging/build-fork.sh` | Debian 安装包构建脚本 / Debian package build script |
| [docs/HARDWARE.md](docs/HARDWARE.md) | 硬件采集来源、温度颜色与读取限制 / Hardware sources, temperature colors, and limits |
| [docs/TEST_REPORT.md](docs/TEST_REPORT.md) | 本机验证记录 / Local validation history |
| [docs/upstream/PROVENANCE.md](docs/upstream/PROVENANCE.md) | 上游来源与原始源码校验值 / Upstream provenance and source checksums |
| `dist/` | 本地生成的安装包，不纳入 Git / Locally generated packages, excluded from Git |

构建缓存、旧版安装包、过时的 Python 原型和已解压的下载压缩包不作为当前项目源码发布。
Build caches, superseded installers, the old Python prototype, and already extracted source archives are omitted from the current source distribution.

## 许可证与致谢 / License and credits

感谢 GNOME System Monitor 原作者、GNOME 社区和 Debian 维护者。本项目沿用上游许可证；主体代码为 GPL-2.0-or-later，部分组件使用各自的许可证。
Credit belongs to the original GNOME System Monitor authors, the GNOME community, and Debian maintainers. This derivative retains the upstream licenses: the main code is GPL-2.0-or-later, with separate licenses for some components.

请参阅 / See [COPYING](COPYING), [AUTHORS](gnome-system-monitor-48.1/AUTHORS), and [Debian copyright](gnome-system-monitor-48.1/debian/copyright).
