/* Read-only Linux sysfs and NVIDIA NVML hardware sampling.
 * Copyright (C) 2026 System Monitor Enhanced contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <config.h>
#include "hardware.h"

#include <gio/gio.h>
#ifdef HAVE_SYSTEMD
#include <systemd/sd-device.h>
#endif

#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <dlfcn.h>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>

namespace enhanced {
namespace {
namespace fs = std::filesystem;

std::string read (const fs::path& path) {
  std::ifstream stream (path);
  std::string value;
  std::getline (stream, value);
  if (!value.empty () && value.back () == '\r') value.pop_back ();
  while (!value.empty () && std::isspace (static_cast<unsigned char> (value.back ()))) value.pop_back ();
  return value;
}

std::optional<double> number (const fs::path& path) {
  auto value = read (path);
  if (value.empty ()) return {};
  try {
    size_t used = 0;
    double result = std::stod (value, &used);
    if (used == value.size ()) return result;
  } catch (...) {}
  return {};
}

std::vector<fs::path> entries (const fs::path& parent, const std::string& prefix) {
  std::vector<fs::path> result;
  std::error_code ec;
  for (fs::directory_iterator it (parent, ec), end; !ec && it != end; it.increment (ec)) {
    auto name = it->path ().filename ().string ();
    if (name.starts_with (prefix)) result.push_back (it->path ());
  }
  std::sort (result.begin (), result.end ());
  return result;
}

fs::path resolved_path (const fs::path& path) {
  std::error_code ec;
  auto resolved = fs::weakly_canonical (path, ec);
  return ec ? fs::path {} : resolved;
}

std::optional<double> temperature (const fs::path& hwmon, const std::string& label = {}) {
  for (const auto& path : entries (hwmon, "temp")) {
    auto name = path.filename ().string ();
    if (!name.ends_with ("_input")) continue;
    if (!label.empty () && read (hwmon / (name.substr (0, name.size () - 6) + "_label")) != label) continue;
    auto value = number (path);
    if (value && *value >= -40000 && *value <= 150000) return *value / 1000;
  }
  return {};
}

std::optional<double> temperature_for (const std::string& sensor, const std::string& label) {
  for (const auto& path : entries ("/sys/class/hwmon", "hwmon"))
    if (read (path / "name") == sensor) {
      auto value = temperature (path, label);
      if (value) return value;
    }
  return {};
}

std::optional<double> cpu_frequency () {
  double sum = 0;
  size_t count = 0;
  for (const auto& cpu : entries ("/sys/devices/system/cpu", "cpu")) {
    auto name = cpu.filename ().string ();
    if (name.size () < 4 || !std::all_of (name.begin () + 3, name.end (), ::isdigit)) continue;
    auto value = number (cpu / "cpufreq/scaling_cur_freq");
    if (value && *value > 0) { sum += *value; ++count; }
  }
  if (!count) return {};
  return sum / count / 1000000.0;
}

std::string cpu_model () {
  std::ifstream input ("/proc/cpuinfo");
  std::string line;
  while (std::getline (input, line)) {
    if (line.starts_with ("model name")) {
      auto colon = line.find (':');
      if (colon != std::string::npos) {
        auto value = line.substr (colon + 1);
        auto first = value.find_first_not_of (" \t");
        return first == std::string::npos ? "CPU" : value.substr (first);
      }
    }
  }
  return "CPU";
}

std::vector<MemoryModule> memory_modules () {
  std::vector<MemoryModule> modules;
#ifdef HAVE_SYSTEMD
  // udev caches SMBIOS data at boot, so the UI needs no privileged DMI reads.
  sd_device *device = nullptr;
  if (sd_device_new_from_subsystem_sysname (&device, "dmi", "id") < 0) return modules;
  auto property = [&] (const std::string& key) -> std::string {
    const char *value = nullptr;
    return sd_device_get_property_value (device, key.c_str (), &value) >= 0 ? value : "";
  };
  const auto count = positive_integer (property ("MEMORY_ARRAY_NUM_DEVICES"));
  if (count && *count <= 4096) {
    for (unsigned long long i = 0; i < *count; ++i) {
      const auto prefix = "MEMORY_DEVICE_" + std::to_string (i) + "_";
      if (property (prefix + "PRESENT") == "0" || !positive_integer (property (prefix + "SIZE")))
        continue;
      // SPEED_MTS is the module rating; only CONFIGURED_SPEED_MTS describes
      // its configured operating rate. Do not substitute the rating if absent.
      modules.push_back ({property (prefix + "TYPE"),
                         positive_integer (property (prefix + "CONFIGURED_SPEED_MTS"))});
    }
  }
  sd_device_unref (device);
#endif
  return modules;
}

std::optional<double> ata_temperature (const std::string& block_name) {
  // UDisks exposes cached ATA SMART data. Reading properties does not issue
  // SmartUpdate and therefore does not request a drive wake-up.
  GDBusConnection *bus = g_bus_get_sync (G_BUS_TYPE_SYSTEM, nullptr, nullptr);
  if (!bus) return {};
  const std::string block_path = "/org/freedesktop/UDisks2/block_devices/" + block_name;
  GVariant *reply = g_dbus_connection_call_sync (
    bus, "org.freedesktop.UDisks2", block_path.c_str (),
    "org.freedesktop.DBus.Properties", "Get",
    g_variant_new ("(ss)", "org.freedesktop.UDisks2.Block", "Drive"),
    G_VARIANT_TYPE ("(v)"), G_DBUS_CALL_FLAGS_NONE, 1500, nullptr, nullptr);
  if (!reply) {
    g_object_unref (bus);
    return {};
  }
  GVariant *drive = nullptr;
  g_variant_get (reply, "(v)", &drive);
  const std::string drive_path = g_variant_get_string (drive, nullptr);
  g_variant_unref (drive);
  g_variant_unref (reply);
  if (drive_path == "/" || !g_variant_is_object_path (drive_path.c_str ())) {
    g_object_unref (bus);
    return {};
  }

  reply = g_dbus_connection_call_sync (
    bus, "org.freedesktop.UDisks2", drive_path.c_str (),
    "org.freedesktop.DBus.Properties", "GetAll",
    g_variant_new ("(s)", "org.freedesktop.UDisks2.Drive.Ata"),
    G_VARIANT_TYPE ("(a{sv})"), G_DBUS_CALL_FLAGS_NONE, 1500, nullptr, nullptr);
  g_object_unref (bus);
  if (!reply) return {};
  GVariant *properties = nullptr;
  g_variant_get (reply, "(@a{sv})", &properties);
  guint64 updated = 0;
  double kelvin = 0;
  const bool available = g_variant_lookup (properties, "SmartUpdated", "t", &updated)
                         && g_variant_lookup (properties, "SmartTemperature", "d", &kelvin);
  g_variant_unref (properties);
  g_variant_unref (reply);
  // Per UDisks, zero denotes unknown. Reject implausible sensor values too.
  if (!available || !updated || kelvin < 233.15 || kelvin > 423.15) return {};
  return kelvin - 273.15;
}

std::vector<DiskTemperature> disks () {
  std::vector<DiskTemperature> result;
  auto hwmons = entries ("/sys/class/hwmon", "hwmon");
  for (const auto& controller : entries ("/sys/class/nvme", "nvme")) {
    auto model = read (controller / "model");
    auto serial = read (controller / "serial");
    if (model.empty () || serial.empty ()) continue;
    std::optional<double> value;
    auto identity = resolved_path (controller);
    for (const auto& hwmon : hwmons) {
      if (read (hwmon / "name") == "nvme" && resolved_path (hwmon).parent_path () == identity) {
        value = temperature (hwmon, "Composite");
        break;
      }
    }
    result.push_back ({model, serial, value});
  }
  for (const auto& block : entries ("/sys/block", "sd")) {
    auto model = read (block / "device/model");
    auto stable = read (block / "device/wwid");
    if (stable.empty ()) stable = read (block / "device/serial");
    if (!model.empty () && !stable.empty ())
      result.push_back ({model, stable, ata_temperature (block.filename ().string ())});
  }
  std::sort (result.begin (), result.end (), [] (const auto& a, const auto& b) {
    return a.stable_id < b.stable_id;
  });
  return result;
}

GpuSample amd_gpu () {
  GpuSample gpu;
  for (const auto& card : entries ("/sys/class/drm", "card")) {
    auto device = card / "device";
    if (read (device / "vendor") != "0x1002") continue;
    gpu.utilization = number (device / "gpu_busy_percent");
    std::ifstream clocks (device / "pp_dpm_sclk");
    std::string line;
    while (std::getline (clocks, line)) {
      if (line.find ('*') == std::string::npos) continue;
      std::smatch match;
      if (std::regex_search (line, match, std::regex (R"((\d+)\s*[Mm][Hh][Zz])")))
        gpu.clock_mhz = std::stod (match[1]);
    }
    for (const auto& hwmon : entries ("/sys/class/hwmon", "hwmon")) {
      if (read (hwmon / "name") == "amdgpu" && resolved_path (hwmon / "device") == resolved_path (device)) {
        gpu.temperature = temperature (hwmon);
        auto frequency_hz = number (hwmon / "freq1_input");
        if (frequency_hz && *frequency_hz > 0) gpu.clock_mhz = *frequency_hz / 1000000;
        break;
      }
    }
    gpu.memory_used = number (device / "mem_info_vram_used");
    gpu.memory_total = number (device / "mem_info_vram_total");
    gpu.shared_used = number (device / "mem_info_gtt_used");
    gpu.shared_total = number (device / "mem_info_gtt_total");
    break;
  }
  return gpu;
}

struct NvmlUtilization { unsigned gpu, memory; };
struct NvmlMemory { unsigned long long total, free, used; };

GpuSample nvidia_gpu () {
  GpuSample gpu;
  void *library = dlopen ("libnvidia-ml.so.1", RTLD_LAZY | RTLD_LOCAL);
  if (!library) return gpu;
  auto init = reinterpret_cast<int (*)()> (dlsym (library, "nvmlInit_v2"));
  auto shutdown = reinterpret_cast<int (*)()> (dlsym (library, "nvmlShutdown"));
  auto count_fn = reinterpret_cast<int (*)(unsigned *)> (dlsym (library, "nvmlDeviceGetCount_v2"));
  auto handle_fn = reinterpret_cast<int (*)(unsigned, void **)> (dlsym (library, "nvmlDeviceGetHandleByIndex_v2"));
  auto name_fn = reinterpret_cast<int (*)(void *, char *, unsigned)> (dlsym (library, "nvmlDeviceGetName"));
  if (!init || !shutdown || !count_fn || !handle_fn || !name_fn || init () != 0) {
    dlclose (library);
    return gpu;
  }
  unsigned count = 0;
  void *device = nullptr;
  if (count_fn (&count) == 0) for (unsigned i = 0; i < count; ++i) {
    void *candidate = nullptr;
    char name[128] = {};
    if (handle_fn (i, &candidate) == 0 && name_fn (candidate, name, sizeof name) == 0 &&
        std::string (name).find ("RTX 5090") != std::string::npos) {
      device = candidate;
      break;
    }
  }
  if (device) {
    auto uint_call = [&] (const char *symbol, unsigned argument = 0, bool with_argument = false) -> std::optional<double> {
      unsigned value = 0;
      if (with_argument) {
        auto fn = reinterpret_cast<int (*)(void *, unsigned, unsigned *)> (dlsym (library, symbol));
        if (fn && fn (device, argument, &value) == 0) return value;
      } else {
        auto fn = reinterpret_cast<int (*)(void *, unsigned *)> (dlsym (library, symbol));
        if (fn && fn (device, &value) == 0) return value;
      }
      return {};
    };
    auto util_fn = reinterpret_cast<int (*)(void *, NvmlUtilization *)> (dlsym (library, "nvmlDeviceGetUtilizationRates"));
    NvmlUtilization util {};
    if (util_fn && util_fn (device, &util) == 0) gpu.utilization = util.gpu;
    auto mem_fn = reinterpret_cast<int (*)(void *, NvmlMemory *)> (dlsym (library, "nvmlDeviceGetMemoryInfo"));
    NvmlMemory memory {};
    if (mem_fn && mem_fn (device, &memory) == 0) {
      gpu.memory_used = memory.used;
      gpu.memory_total = memory.total;
    }
    gpu.temperature = uint_call ("nvmlDeviceGetTemperature", 0, true);
    gpu.clock_mhz = uint_call ("nvmlDeviceGetClockInfo", 0, true);
    gpu.fan_percent = uint_call ("nvmlDeviceGetFanSpeed");
    auto power = uint_call ("nvmlDeviceGetPowerUsage");
    if (power) gpu.power_watts = *power / 1000;
    auto pstate = uint_call ("nvmlDeviceGetPerformanceState");
    if (pstate) gpu.performance_state = "P" + std::to_string (static_cast<int> (*pstate));
  }
  shutdown ();
  dlclose (library);
  return gpu;
}
} // namespace

Sample collect_hardware (bool sample_nvidia) {
  Sample sample;
  sample.cpu_model = cpu_model ();
  sample.cpu_frequency_ghz = cpu_frequency ();
  sample.cpu_tctl = temperature_for ("k10temp", "Tctl");
  sample.memory_modules = memory_modules ();
  for (const auto& hwmon : entries ("/sys/class/hwmon", "hwmon"))
    if (read (hwmon / "name") == "spd5118") {
      auto value = temperature (hwmon);
      if (value) sample.dimm_temperatures.push_back (*value);
    }
  sample.disks = disks ();
  sample.amd = amd_gpu ();
  if (sample_nvidia) sample.nvidia = nvidia_gpu ();
  return sample;
}

Sampler::Sampler () : worker_ (&Sampler::run, this) {}
Sampler::~Sampler () {
  {
    std::lock_guard lock (mutex_);
    stop_ = true;
  }
  condition_.notify_all ();
  worker_.join ();
}
Sample Sampler::latest () {
  std::lock_guard lock (mutex_);
  return sample_;
}
void Sampler::set_active (bool active) {
  {
    std::lock_guard lock (mutex_);
    active_ = active;
  }
  condition_.notify_all ();
}
void Sampler::run () {
  std::unique_lock lock (mutex_);
  while (!stop_) {
    condition_.wait (lock, [&] { return stop_ || active_; });
    if (stop_) break;
    lock.unlock ();
    auto current = collect_hardware ();
    lock.lock ();
    sample_ = std::move (current);
    condition_.wait_for (lock, std::chrono::seconds (5), [&] { return stop_ || !active_; });
  }
}
Sampler& hardware_sampler () {
  static Sampler sampler;
  return sampler;
}
} // namespace enhanced
