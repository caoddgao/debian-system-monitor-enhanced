/* Read-only memory configuration reported by firmware through udev.
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

#include <algorithm>
#include <charconv>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace enhanced {

inline std::optional<unsigned long long> positive_integer (std::string_view text) {
  if (text.empty ()) return {};
  unsigned long long value = 0;
  const auto result = std::from_chars (text.data (), text.data () + text.size (), value);
  if (result.ec != std::errc {} || result.ptr != text.data () + text.size () || !value)
    return {};
  return value;
}

struct MemoryModule {
  std::string type;
  std::optional<unsigned long long> configured_speed_mts;
};

inline std::string memory_speed_text (const std::vector<MemoryModule>& modules) {
  std::vector<std::string> values;
  for (const auto& module : modules) {
    std::string value = module.type.empty () || module.type == "Unknown" ? "" : module.type + " ";
    value += module.configured_speed_mts ? std::to_string (*module.configured_speed_mts) : "N/A";
    if (std::find (values.begin (), values.end (), value) == values.end ())
      values.push_back (value);
  }
  std::string result;
  for (const auto& value : values) {
    if (!result.empty ()) result += " / ";
    result += value;
  }
  return result.empty () ? "N/A" : result;
}

} // namespace enhanced
