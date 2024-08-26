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

#include <libhal-util/output_pin.hpp>
#include <libhal-util/spi.hpp>
#include <libhal-soft/inert_drivers/inert_output_pin.hpp>

namespace hal::display {

template<std::size_t PixelCount>
struct ws2812b_spi_frame
{
  static constexpr std::size_t colors_available = 3;
  static constexpr std::size_t bits_per_pixel_color = 8;
  static constexpr std::size_t spi_bits_to_encode_each_bit = 4;
  static constexpr std::size_t bytes_to_store_one_pixels_data =
    (colors_available * bits_per_pixel_color * spi_bits_to_encode_each_bit) / 8;

  static constexpr std::size_t array_length =
    PixelCount * bytes_to_store_one_pixels_data;

  std::array<hal::byte, array_length> data;
};

struct rgb888
{
  hal::byte r;
  hal::byte g;
  hal::byte b;
};

class ws2812b
{
public:
  ws2812b(hal::spi& p_spi, hal::output_pin* p_chip_select = &hal::soft::default_inert_output_pin()); 

  template<std::size_t PixelCount>
  void update(std::array<hal::byte,PixelCount>& p_frame_data)
  {
    update(std::span<hal::byte>(p_frame_data));
  }

private:
  void update(std::span<hal::byte> p_data);

  hal::spi* m_spi;
  hal::output_pin* m_chip_select;
};

}  // namespace hal::display
