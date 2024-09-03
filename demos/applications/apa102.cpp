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

hal::byte build_brightness_byte(unsigned p_brightness)
{
  hal::byte starting_bits = 0b11100000;
  if (p_brightness > 31) {
    p_brightness = 31;
  }
  return (starting_bits | p_brightness);
}

template<std::size_t PixelCount>
void update_single(hal::display::apa102_pixel& p_rgb,
                   unsigned p_brightness,
                   size_t p_led_number,
                   hal::display::apa102_frame<PixelCount>& p_led_frames)
{
  if (p_led_number < PixelCount) {
    p_led_frames.pixels[p_led_number].brightness =
      build_brightness_byte(p_brightness);
    p_led_frames.pixels[p_led_number].blue = p_rgb.blue;
    p_led_frames.pixels[p_led_number].green = p_rgb.green;
    p_led_frames.pixels[p_led_number].red = p_rgb.red;
  }
}

template<std::size_t PixelCount>
void update_all(std::span<hal::display::apa102_pixel> p_leds,
                unsigned p_brightness,
                hal::display::apa102_frame<PixelCount>& p_led_frames)
{
  for (size_t i = 0; i < p_leds.size(); i++) {
    update_single(p_leds[i], p_brightness, i, p_led_frames);
  }
}

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
  hal::display::apa102_frame<led_count> apa_frame;
  unsigned brightness = 1;

  // predefined colors
  hal::display::apa102_pixel red{ .blue = 0x00, .green = 0x00, .red = 0xFF };
  hal::display::apa102_pixel green{ .blue = 0x00, .green = 0xFF, .red = 0x00 };
  hal::display::apa102_pixel blue{ .blue = 0xFF, .green = 0x00, .red = 0x00 };
  hal::display::apa102_pixel white{ .blue = 0xFF, .green = 0xFF, .red = 0xFF };
  std::array<hal::display::apa102_pixel, led_count> all_off{};

  std::array<hal::display::apa102_pixel, led_count> rgb_array = {
    { white, blue, green, red }
  };

  hal::print(console, "Demo Application Starting...\n\n");
  hal::display::apa102 led_strip(spi, chip_select);
  while (true) {
    // reset LEDs by turning them all off
    update_all(all_off, brightness, apa_frame);

    // update one at a time, all other LEDs should remain the same state
    // added delays to visually see individual activations
    hal::print(console, "Updating single LEDS\n");
    update_single(red, brightness, 0, apa_frame);
    led_strip.update(apa_frame);
    hal::delay(clock, 500ms);
    update_single(green, brightness, 1, apa_frame);
    led_strip.update(apa_frame);
    hal::delay(clock, 500ms);
    update_single(blue, brightness, 2, apa_frame);
    led_strip.update(apa_frame);
    hal::delay(clock, 500ms);
    update_single(white, brightness, 3, apa_frame);
    led_strip.update(apa_frame);

    // update all LEDs at once
    hal::delay(clock, 3s);
    hal::print(console, "Updating all LEDS\n");
    update_all(rgb_array, brightness, apa_frame);
    led_strip.update(apa_frame);

    // cycle through RGB colors, start and end with red
    hal::delay(clock, 3s);
    hal::display::apa102_pixel rainbow = red;
    hal::print(console, "Rainbow Cycle\n");
    for (int i = 0; i <= 255; i++) {
      // decrease red, increase blue
      rainbow.red = 255 - i;
      rainbow.blue = i;
      std::array<hal::display::apa102_pixel, led_count> rainbow_array = {
        { rainbow, rainbow, rainbow, rainbow }
      };
      update_all(rainbow_array, brightness, apa_frame);
      led_strip.update(apa_frame);
      hal::delay(clock, 10ms);
    }
    for (int i = 0; i <= 255; i++) {
      // decrease blue, increase green
      rainbow.blue = 255 - i;
      rainbow.green = i;
      std::array<hal::display::apa102_pixel, led_count> rainbow_array = {
        { rainbow, rainbow, rainbow, rainbow }
      };
      update_all(rainbow_array, brightness, apa_frame);
      led_strip.update(apa_frame);
      hal::delay(clock, 10ms);
    }
    for (int i = 0; i <= 255; i++) {
      // decrease green, increase red
      rainbow.green = 255 - i;
      rainbow.red = i;
      std::array<hal::display::apa102_pixel, led_count> rainbow_array = {
        { rainbow, rainbow, rainbow, rainbow }
      };
      update_all(rainbow_array, brightness, apa_frame);
      led_strip.update(apa_frame);
      hal::delay(clock, 10ms);
    }
  }
  hal::delay(clock, 50ms);
}
