#include "../metric-color.h"

#include <catch2/catch_test_macros.hpp>

using enhanced::MetricColor;

TEST_CASE("temperature warning boundaries", "[enhanced]") {
  CHECK(enhanced::temperature_color ({}, 50, 70) == MetricColor::neutral);
  CHECK(enhanced::temperature_color (49.9, 50, 70) == MetricColor::normal);
  CHECK(enhanced::temperature_color (50, 50, 70) == MetricColor::elevated);
  CHECK(enhanced::temperature_color (70, 50, 70) == MetricColor::warning);
  CHECK(enhanced::temperature_color (65, 50, 65) == MetricColor::warning);
  CHECK(enhanced::temperature_color (45, 45, 60) == MetricColor::elevated);
  CHECK(enhanced::temperature_color (60, 45, 60) == MetricColor::warning);
  CHECK(enhanced::temperature_color (40, 40, 50) == MetricColor::elevated);
  CHECK(enhanced::temperature_color (50, 40, 50) == MetricColor::warning);
}

TEST_CASE("capacity and GPU load colors", "[enhanced]") {
  CHECK(enhanced::pressure_color ({}) == MetricColor::neutral);
  CHECK(enhanced::pressure_color (69.9) == MetricColor::normal);
  CHECK(enhanced::pressure_color (70) == MetricColor::elevated);
  CHECK(enhanced::pressure_color (90) == MetricColor::warning);
  CHECK(enhanced::pressure_color (100, true) == MetricColor::accent);
}
