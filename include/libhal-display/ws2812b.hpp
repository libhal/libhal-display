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

#include <libhal-util/inert_drivers/inert_output_pin.hpp>
#include <libhal-util/output_pin.hpp>
#include <libhal-util/spi.hpp>

namespace hal::display {

/**
 * @brief Represents the frame of data for the ws2812b pixels to be transmitted
 * over SPI.
 *
 * This struct is used to store the frame of color data that will be used to
 * update the pixels values over SPI. It calculates the necessary size of the
 * data array based on the number of pixels the user specifies.
 *
 * @tparam PixelCount - The number of pixels that are intended to be used.
 */
template<std::size_t PixelCount>
struct ws2812b_spi_frame
{
  /// The three LEDs internal to each pixel: Red, Green, and Blue.
  static constexpr std::size_t colors_available = 3;
  /// The amount of bits used to represent each internal LEDs value.
  static constexpr std::size_t bits_per_pixel_color = 8;
  /// The amount of bits SPI needs to generate the proper pulses for the data.
  static constexpr std::size_t spi_bits_to_encode_each_bit = 4;
  /// Calculates the amount of bytes needed to store the data for one pixel.
  static constexpr std::size_t bytes_to_store_one_pixels_data =
    (colors_available * bits_per_pixel_color * spi_bits_to_encode_each_bit) / 8;

  // Calculates the array size with the user specified amount of pixels.
  static constexpr std::size_t array_length =
    PixelCount * bytes_to_store_one_pixels_data;

  // The array to store the pixels' data.
  std::array<hal::byte, array_length> data;
};

/**
 * @brief Driver for the ws2812b individually addressable RGB LED strip
 *
 */
class ws2812b
{
public:
  /**
   * @brief Construct a ws2812b driver.
   *
   * @param p_spi - The driver for the SPI bus the ws2812b is connected to. The
   * SPI driver MUST support a 4.0MHz clock rate, as this clock rate is
   * specifically used to generate the proper pulse widths for sending the data.
   * If 4.0MHz is not supported, then the device may show incorrect colors, or
   * fail to work at all.
   * @param p_chip_select - The driver for the output pin to be used as the chip
   * select if the devices data line is connected to a multiplexer/switch.
   * Defaults to an inert output pin if one is not provided.
   */
  ws2812b(hal::spi& p_spi,
          hal::output_pin& p_chip_select = hal::default_inert_output_pin());

  /**
   * @brief Update the pixels to the currently stored color information.
   *
   * @tparam PixelCount - The amount of pixels the ws2812b device is using.
   * @param p_spi_frame - The frame storing the pixels' color information.
   */
  template<std::size_t PixelCount>
  void update(ws2812b_spi_frame<PixelCount>& p_spi_frame)
  {
    update(p_spi_frame.data);
  }

private:
  void update(std::span<hal::byte> p_data);

  hal::spi* m_spi;
  hal::output_pin* m_chip_select;
};

}  // namespace hal::display
