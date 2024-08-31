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

#include <libhal-display/apa102.hpp>
#include <libhal-util/serial.hpp>
#include <libhal-util/steady_clock.hpp>

#include "../resource_list.hpp"

template<std::size_t PixelCount>
void update_single(hal::display::rgb_values p_rgb,
                   int p_brightnes,
                   size_t p_led_number,
                   std::span<hal::byte> p_led_frames);

template<std::size_t PixelCount>
void update_all(std::span<hal::display::rgb_values> p_leds,
                int p_brightnes,
                std::span<hal::byte> p_led_frames);

hal::byte build_brightness_byte(int p_brightnes);

void application(resource_list& p_map)
{
  using namespace std::chrono_literals;
  using namespace hal::literals;

  // get resources
  auto& clock = *p_map.clock.value();
  auto& console = *p_map.console.value();
  auto& chip_select = *p_map.spi_chip_select.value();
  auto& spi = *p_map.spi.value();

  // variables for LEDs
  constexpr std::size_t led_count = 4;
  hal::display::apa102_spi_frame<led_count> spi_frame;
  int brightness = 1;

  // predefined colors
  hal::display::rgb_values red{ 0xFF, 0x00, 0x00 };
  hal::display::rgb_values green{ 0x00, 0xFF, 0x00 };
  hal::display::rgb_values blue{ 0x00, 0x00, 0xFF };
  hal::display::rgb_values white{ 0xFF, 0xFF, 0xFF };
  std::array<hal::display::rgb_values, led_count> all_off = {
    { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } }
  };
  std::array<hal::display::rgb_values, led_count> rgb_array = {
    { { 255, 0, 0 }, { 125, 0, 125 }, { 0, 0, 255 }, { 0, 125, 125 } }
  };

  hal::print(console, "Demo Application Starting...\n\n");
  hal::display::apa102 led_strip(spi, chip_select);
  while (true) {
    // reset LEDs by turning them all off
    update_all<led_count>(all_off, brightness, spi_frame.data);
    hal::print<32>(console, "Brightness byte: 0x%02X ", spi_frame.data[0]);

    // update one at a time, all other LEDs should remain the same state
    // added delays to visually see individual activations
    hal::print(console, "Updating single LEDS\n");
    update_single<led_count>(red, brightness, 0, spi_frame.data);
    led_strip.update(spi_frame.data);
    hal::delay(clock, 500ms);
    update_single<led_count>(green, brightness, 1, spi_frame.data);
    led_strip.update(spi_frame.data);
    hal::delay(clock, 500ms);
    update_single<led_count>(blue, brightness, 2, spi_frame.data);
    led_strip.update(spi_frame.data);
    hal::delay(clock, 500ms);
    update_single<led_count>(white, brightness, 3, spi_frame.data);
    led_strip.update(spi_frame.data);

    // update all LEDs at once
    hal::delay(clock, 3s);
    hal::print(console, "Updating all LEDS\n");
    update_all<led_count>(rgb_array, brightness, spi_frame.data);
    led_strip.update(spi_frame.data);

    // cycle through RGB colors, start and end with red
    hal::delay(clock, 3s);
    hal::display::rgb_values rainbow = red;
    hal::print(console, "Rainbow Cycle\n");
    for (int i = 0; i <= 255; i++) {
      // decrease red, increase blue
      rainbow.red = 255 - i;
      rainbow.blue = i;
      std::array<hal::display::rgb_values, led_count> rainbow_array = {
        { rainbow, rainbow, rainbow, rainbow }
      };
      update_all<led_count>(rainbow_array, brightness, spi_frame.data);
      led_strip.update(spi_frame.data);
      hal::delay(clock, 10ms);
    }
    for (int i = 0; i <= 255; i++) {
      // decrease blue, increase green
      rainbow.blue = 255 - i;
      rainbow.green = i;
      std::array<hal::display::rgb_values, led_count> rainbow_array = {
        { rainbow, rainbow, rainbow, rainbow }
      };
      update_all<led_count>(rainbow_array, brightness, spi_frame.data);
      led_strip.update(spi_frame.data);
      hal::delay(clock, 10ms);
    }
    for (int i = 0; i <= 255; i++) {
      // decrease green, increase red
      rainbow.green = 255 - i;
      rainbow.red = i;
      std::array<hal::display::rgb_values, led_count> rainbow_array = {
        { rainbow, rainbow, rainbow, rainbow }
      };
      update_all<led_count>(rainbow_array, brightness, spi_frame.data);
      led_strip.update(spi_frame.data);
      hal::delay(clock, 10ms);
    }
  }
  hal::delay(clock, 50ms);
}

template<std::size_t PixelCount>
void update_single(hal::display::rgb_values p_rgb,
                   int p_brightness,
                   size_t p_led_number,
                   std::span<hal::byte> p_led_frames)
{
  size_t index = p_led_number * 4;
  p_led_frames[index] = build_brightness_byte(p_brightness);
  p_led_frames[index + 1] = p_rgb.blue;
  p_led_frames[index + 2] = p_rgb.green;
  p_led_frames[index + 3] = p_rgb.red;
}

template<std::size_t PixelCount>
void update_all(std::span<hal::display::rgb_values> p_leds,
                int p_brightness,
                std::span<hal::byte> p_led_frames)
{
  for (size_t i = 0; i < p_leds.size(); i++) {
    p_led_frames[i * 4] = build_brightness_byte(p_brightness);
    p_led_frames[(i * 4) + 1] = p_leds[i].blue;
    p_led_frames[(i * 4) + 2] = p_leds[i].green;
    p_led_frames[(i * 4) + 3] = p_leds[i].red;
  }
}

hal::byte build_brightness_byte(int p_brightness)
{
  if (p_brightness > 31) {
    p_brightness = 31;
  }
  return (0b11100000 | p_brightness);
}
