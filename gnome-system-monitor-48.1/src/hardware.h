/* Additional read-only sensors for System Monitor Enhanced.
 * Copyright (C) 2026 System Monitor Enhanced contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

#include "memory-info.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

namespace enhanced {

struct DiskTemperature {
  std::string model;
  std::string stable_id;
  std::optional<double> celsius;
};

struct GpuSample {
  std::optional<double> utilization;
  std::optional<double> temperature;
  std::optional<double> clock_mhz;
  std::optional<double> memory_used;
  std::optional<double> memory_total;
  std::optional<double> shared_used;
  std::optional<double> shared_total;
  std::optional<double> power_watts;
  std::optional<double> fan_percent;
  std::string performance_state;
};

struct Sample {
  std::string cpu_model;
  std::optional<double> cpu_frequency_ghz;
  std::optional<double> cpu_tctl;
  std::vector<double> dimm_temperatures;
  std::vector<MemoryModule> memory_modules;
  std::vector<DiskTemperature> disks;
  GpuSample nvidia;
  GpuSample amd;
};

Sample collect_hardware (bool sample_nvidia = true);
class Sampler;
Sampler& hardware_sampler ();

class Sampler {
public:
  Sampler ();
  ~Sampler ();
  Sample latest ();
  void set_active (bool active);
private:
  void run ();
  std::mutex mutex_;
  std::condition_variable condition_;
  std::thread worker_;
  Sample sample_;
  bool stop_ = false;
  bool active_ = false;
};

} // namespace enhanced
