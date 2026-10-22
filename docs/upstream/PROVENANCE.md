# 上游来源 / Upstream provenance

本项目是面向 Debian Linux 的 GNOME System Monitor 衍生版本，基于 Debian `gnome-system-monitor 48.1-2`，并非从零编写。完整源码、翻译、帮助文档及原有版权声明保存在 `gnome-system-monitor-48.1/`。

This project is a GNOME System Monitor derivative for Debian Linux, based on Debian's `gnome-system-monitor 48.1-2`. It was adapted from the original application. The complete source tree, translations, help files, and original copyright notices are retained in `gnome-system-monitor-48.1/`.

- GNOME project: <https://apps.gnome.org/SystemMonitor/>
- Upstream source: <https://gitlab.gnome.org/GNOME/gnome-system-monitor>
- Debian source packaging: <https://salsa.debian.org/gnome-team/gnome-system-monitor>
- Original signed source descriptor: [gnome-system-monitor_48.1-2.dsc](gnome-system-monitor_48.1-2.dsc)
- Original copyright and component licenses: [debian/copyright](../../gnome-system-monitor-48.1/debian/copyright)
- GPL text: [COPYING](../../COPYING)

下载的原始压缩包已解压为仓库中的源码，为避免重复不再保存压缩包。原始校验值记录如下，也包含在签名的 `.dsc` 中。

The downloaded source archives have already been extracted into the source tree and are omitted to avoid duplication. Their original SHA-256 checksums are recorded here and in the signed `.dsc` descriptor.

| Original file | SHA-256 |
| --- | --- |
| `gnome-system-monitor_48.1.orig.tar.xz` | `133c3a6e286366e659fd2d80582409a7bd5edae456e63c4f69ccf66ed6fc6638` |
| `gnome-system-monitor_48.1-2.debian.tar.xz` | `0770f35189ad072c7565aa02c7787bbb3cf5f7bd207071b9816362878ef61c4c` |

增强部分采用 C++、GTK4、gtkmm 和 libadwaita，添加了 CPU、内存、GPU、磁盘的只读硬件采集及资源页面显示。原有进程管理、资源曲线和文件系统页面保留。

The enhancements use C++, GTK4, gtkmm, and libadwaita to add read-only CPU, memory, GPU, and disk hardware readings to the Resources page. The original process management, resource graphs, and File Systems page are retained.
