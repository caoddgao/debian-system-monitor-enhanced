/* Live read-only collector probe. Builds without GTK dependencies. */
#include "../hardware.h"
#include <iostream>

int main () {
  const auto sample = enhanced::collect_hardware ();
  auto print = [] (const char *key, std::optional<double> value) {
    std::cout << key << '=';
    if (value) std::cout << *value;
    else std::cout << "N/A";
    std::cout << '\n';
  };
  std::cout << "cpu_model=" << sample.cpu_model << '\n';
  print ("cpu_frequency_ghz", sample.cpu_frequency_ghz);
  print ("cpu_tctl_celsius", sample.cpu_tctl);
  std::cout << "memory_speed=" << enhanced::memory_speed_text (sample.memory_modules) << '\n';
  for (size_t i = 0; i < sample.dimm_temperatures.size (); ++i)
    print (("dimm_" + std::to_string (i + 1) + "_celsius").c_str (), sample.dimm_temperatures[i]);
  for (const auto& disk : sample.disks) {
    std::cout << "disk=" << disk.model << " [" << disk.stable_id << "] ";
    if (disk.celsius) std::cout << *disk.celsius;
    else std::cout << "N/A";
    std::cout << '\n';
  }
  print ("nvidia_utilization", sample.nvidia.utilization);
  print ("nvidia_vram_used", sample.nvidia.memory_used);
  print ("nvidia_temperature", sample.nvidia.temperature);
  print ("nvidia_power_watts", sample.nvidia.power_watts);
  print ("nvidia_clock_mhz", sample.nvidia.clock_mhz);
  print ("nvidia_fan_percent", sample.nvidia.fan_percent);
  std::cout << "nvidia_pstate=" << (sample.nvidia.performance_state.empty () ? "N/A" : sample.nvidia.performance_state) << '\n';
  print ("amd_utilization", sample.amd.utilization);
  print ("amd_clock_mhz", sample.amd.clock_mhz);
  print ("amd_temperature", sample.amd.temperature);
  print ("amd_vram_used", sample.amd.memory_used);
  print ("amd_gtt_shared_used", sample.amd.shared_used);
}
