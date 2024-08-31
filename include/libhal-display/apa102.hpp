// Copyright 2024 Khalil Estell
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <array>

#include <libhal-soft/inert_drivers/inert_output_pin.hpp>
#include <libhal-util/output_pin.hpp>
#include <libhal-util/spi.hpp>

namespace hal::display {
/**
 * @brief Contains data to send over SPI and information about size of the data
 * to send over
 *
 * @tparam LedCount - Number of LEDs to control
 */
template<std::size_t LedCount>
struct apa102_spi_frame
{
  // each LED needs 1 byte for bightness and 1 byte each for red, green, and
  // blue
  static constexpr std::size_t led_frame_byte_count = 4;
  static constexpr std::size_t array_length = LedCount * led_frame_byte_count;

  std::array<hal::byte, array_length> data;
};

struct rgb_values
{
  hal::byte red;
  hal::byte green;
  hal::byte blue;
};

/**
 * @brief Driver for apa102 RGB LEDs
 *
 */
class apa102
{
public:
  /**
   * @brief Construct a new apa102 object
   *
   * @param p_spi the spi bus that controls the LEDs
   * @param p_chip_select output pin acting as the chip select for the spi bus
   */
  apa102(
    hal::spi& p_spi,
    hal::output_pin& p_chip_select = hal::soft::default_inert_output_pin());

  /**
   * @brief Update the state of the LEDs
   *
   * @tparam LedCount - Number of LEDs to control
   * @param p_frame_data Bytes to send to control LEDs
   */
  template<std::size_t LedCount>
  void update(std::array<hal::byte, LedCount>& p_frame_data)
  {
    update(std::span<hal::byte>(p_frame_data));
  }

private:
  void update(std::span<hal::byte> p_data);

  hal::spi* m_spi;

  hal::output_pin* m_chip_select;
};
}  // namespace hal::display
