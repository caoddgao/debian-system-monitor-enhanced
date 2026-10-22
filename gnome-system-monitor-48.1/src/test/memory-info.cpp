#include "../memory-info.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("memory configuration with missing and mixed firmware data", "[enhanced]") {
  using enhanced::memory_speed_text;
  CHECK(memory_speed_text ({}) == "N/A");
  CHECK(memory_speed_text ({{"DDR5", 5200}, {"DDR5", 5200}}) == "DDR5 5200");
  CHECK(memory_speed_text ({{"DDR5", {}}, {"DDR5", 5200}}) == "DDR5 N/A / DDR5 5200");
  CHECK(memory_speed_text ({{"DDR5", 5200}, {"DDR5", 4800}}) == "DDR5 5200 / DDR5 4800");
  CHECK(memory_speed_text ({{"Unknown", {}}}) == "N/A");
  CHECK(memory_speed_text ({{"", 3200}}) == "3200");
}

TEST_CASE("udev memory integers reject unknown and malformed values", "[enhanced]") {
  CHECK(enhanced::positive_integer ("5200") == 5200);
  for (const auto *value : {"", "0", "-1", "Unknown", "5200 MT/s", "18446744073709551616"})
    CHECK_FALSE(enhanced::positive_integer (value).has_value ());
}
