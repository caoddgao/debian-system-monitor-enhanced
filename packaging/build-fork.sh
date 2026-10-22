#!/usr/bin/env bash
# Build the Debian 48.1-2 source fork as an independent package.
set -euo pipefail

project_root="$(cd "$(dirname "$0")/.." && pwd)"
source_root="$project_root/gnome-system-monitor-48.1"
build_root="$source_root/build"
stage_root="$(mktemp -d /tmp/system-monitor-enhanced-stage.XXXXXX)"
package_version='48.1-2+enhanced7'
package_file="$project_root/dist/system-monitor-enhanced_${package_version}_amd64.deb"

test "$(dpkg --print-architecture)" = amd64
if test ! -f "$build_root/build.ninja"; then
  meson setup "$build_root" "$source_root" --prefix=/usr --buildtype=release
fi
meson compile -C "$build_root" -j 4
meson install -C "$build_root" --destdir "$stage_root" --no-rebuild > "$build_root/package-install.log"

mkdir -p "$stage_root/usr/share/doc/system-monitor-enhanced" "$stage_root/DEBIAN" "$project_root/dist"
cp "$source_root/COPYING" "$stage_root/usr/share/doc/system-monitor-enhanced/COPYING"
cp "$source_root/debian/copyright" "$stage_root/usr/share/doc/system-monitor-enhanced/copyright"

python3 - "$stage_root" <<'PY'
from pathlib import Path
import subprocess
import sys
stage = Path(sys.argv[1])
files = {'/' + str(path.relative_to(stage)) for path in stage.rglob('*') if path.is_file() or path.is_symlink()}
installed = subprocess.run(['dpkg-query', '-W', '-f=${Status}', 'gnome-system-monitor'],
                           text=True, capture_output=True)
original = (set(subprocess.check_output(['dpkg', '-L', 'gnome-system-monitor'], text=True).splitlines())
            if installed.returncode == 0 and installed.stdout == 'install ok installed' else set())
collisions = sorted(files & original)
if collisions:
    raise SystemExit('Refusing package with original file collisions: ' + ', '.join(collisions))
print(f'Checked {len(files)} installed paths; no original package file collisions'
      if original else f'Original package absent; checked {len(files)} staged paths')
PY

depends="$(cd "$source_root" && dpkg-shlibdeps -O "$stage_root/usr/bin/system-monitor-enhanced" "$stage_root/usr/libexec/system-monitor-enhanced/gsm-kill" "$stage_root/usr/libexec/system-monitor-enhanced/gsm-renice" "$stage_root/usr/libexec/system-monitor-enhanced/gsm-taskset" 2>/dev/null)"
depends="${depends#shlibs:Depends=}"
cat > "$stage_root/DEBIAN/control" <<EOF
Package: system-monitor-enhanced
Version: $package_version
Section: gnome
Priority: optional
Architecture: amd64
Maintainer: System Monitor Enhanced contributors
Depends: dconf-gsettings-backend | gsettings-backend, $depends
Provides: gnome-system-monitor (= 48.1-2)
Recommends: gvfs, udisks2
Suggests: pkexec
Description: GNOME System Monitor 48.1 with hardware sensor extensions
 A separately installable fork of Debian GNOME System Monitor 48.1-2.
 It retains Processes, Resources and File Systems while adding CPU,
 DDR5, disk, NVIDIA and AMD GPU sensor readings.
EOF

dpkg-deb --build --root-owner-group "$stage_root" "$package_file"
dpkg-deb --info "$package_file"
echo "$package_file"
