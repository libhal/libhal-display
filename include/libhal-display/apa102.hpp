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
#include <span>

#include <libhal-soft/inert_drivers/inert_output_pin.hpp>
#include <libhal-util/output_pin.hpp>
#include <libhal-util/spi.hpp>

namespace hal::display {

struct apa102_pixel
{
  /// bits 7 - 5 must be all 1's, otherwise undefined behavior
  hal::byte brightness = 0b1111'1111;
  hal::byte blue = 0;
  hal::byte green = 0;
  hal::byte red = 0;
};

static_assert(4U == sizeof(apa102_pixel),
              "APA102 Pixel structure must be 4 bytes in length");
/**
 * @brief Contains data to send over SPI and information about size of the data
 * to send over
 *
 * @tparam pixel_count - Number of pixels to control
 */
template<std::size_t pixel_count>
struct apa102_frame
{
  std::array<apa102_pixel, pixel_count> pixels;
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
   * @tparam pixel_count - Number of pixels to control is set implicitly, user
   * should not set it manually
   * @param p_spi_frame spi frame to send to control LEDs
   */
  template<std::size_t pixel_count>
  void update(apa102_frame<pixel_count>& p_spi_frame)
  {
    update(p_spi_frame.pixels);
  }

private:
  void update(std::span<apa102_pixel> p_data);

  hal::spi* m_spi;

  hal::output_pin* m_chip_select;
};
}  // namespace hal::display
