#include <array>
#include <span>

#include <libhal-display/ws2812b.hpp>
#include <libhal-util/serial.hpp>
#include <libhal-util/steady_clock.hpp>

#include <resource_list.hpp>

struct rgb888
{
  hal::byte r;
  hal::byte g;
  hal::byte b;
};

template<std::size_t PixelCount>
void set_all_pixel(hal::display::ws2812b_spi_frame<PixelCount>& p_frame,
                   rgb888& p_pixel);
template<std::size_t PixelCount>
void set_range_pixel(hal::display::ws2812b_spi_frame<PixelCount>& p_frame,
                     rgb888& p_pixel,
                     uint8_t p_start_pixel,
                     uint8_t p_end_pixel);
void set_range_ws2812b_frame_pixel(std::span<hal::byte>& p_frame,
                                   rgb888& p_pixel);

void application(resource_list& p_map)
{
  using namespace hal::literals;

  // Grab resources
  auto& clock = *p_map.clock.value();
  auto& console = *p_map.console.value();
  auto& chip_select = *p_map.status_led.value();
  auto& spi = *p_map.spi.value();

  hal::print(console, "Demo Application Starting...\n\n");

  // Set chip_select high
  chip_select.level(true);

  // Create ws2812b object
  hal::display::ws2812b ws2812b_driver(spi, chip_select);

  // Create frame buffer for 5 pixels
  constexpr std::size_t PixelCount = 5;
  hal::display::ws2812b_spi_frame<PixelCount> spi_frame;
  spi_frame.data.fill(0x00);

  // Definining some RGB888 Colors as well as an array of colors
  rgb888 red_color = { 255, 0, 0 };
  rgb888 blue_color = { 0, 0, 255 };
  rgb888 green_color = { 0, 255, 0 };
  std::array<rgb888, 5> rainbow_array = { { { 255, 0, 0 },
                                            { 255, 255, 0 },
                                            { 0, 255, 0 },
                                            { 0, 0, 255 },
                                            { 160, 32, 240 } } };

  // Example of using the set_all_pixel function
  hal::print(console, "Setting all pixels to the color red...\n");
  set_all_pixel(spi_frame, red_color);
  ws2812b_driver.update(spi_frame);
  hal::delay(clock, std::chrono::milliseconds(3000));

  // Example of using the set_range_pixel function
  hal::print(console, "Setting pixel index 1-3 to the color green...\n");
  set_range_pixel(spi_frame, green_color, 1, 3);
  ws2812b_driver.update(spi_frame);
  hal::delay(clock, std::chrono::milliseconds(3000));

  // Example of using the set_range_ws2812b_frame_pixel function
  hal::print(
    console,
    "Setting the span of bytes 0-23 (pixel index 0-1) to the color blue...\n");
  std::span<hal::byte> frame_span(spi_frame.data);
  std::span<hal::byte> sub_span = frame_span.subspan(0, 24);
  set_range_ws2812b_frame_pixel(sub_span, blue_color);
  ws2812b_driver.update(spi_frame);
  hal::delay(clock, std::chrono::milliseconds(3000));

  // Example of a infinite scrolling array of colors
  hal::print(console, "Starting rainbow loop...\n");
  int current_color = 0;
  while (true) {
    using namespace std::literals;

    set_range_pixel(spi_frame, rainbow_array[current_color], 0, 0);
    set_range_pixel(spi_frame, rainbow_array[(current_color + 1) % 5], 1, 1);
    set_range_pixel(spi_frame, rainbow_array[(current_color + 2) % 5], 2, 2);
    set_range_pixel(spi_frame, rainbow_array[(current_color + 3) % 5], 3, 3);
    set_range_pixel(spi_frame, rainbow_array[(current_color + 4) % 5], 4, 4);

    current_color = (current_color + 1) % 5;
    ws2812b_driver.update(spi_frame);
    hal::delay(clock, 100ms);
  }
}

template<std::size_t PixelCount>
void set_all_pixel(hal::display::ws2812b_spi_frame<PixelCount>& p_frame,
                   rgb888& p_pixel)
{
  set_range_pixel(p_frame, p_pixel, 0, (PixelCount - 1));
}

template<std::size_t PixelCount>
void set_range_pixel(hal::display::ws2812b_spi_frame<PixelCount>& p_frame,
                     rgb888& p_pixel,
                     uint8_t p_start_pixel,
                     uint8_t p_end_pixel)
{

  std::size_t num_pixels = (p_end_pixel - p_start_pixel + 1);
  constexpr std::size_t bytes_per_pixel = 12;
  std::size_t num_bytes = num_pixels * bytes_per_pixel;

  std::span<hal::byte> frame_span(p_frame.data);
  std::span<hal::byte> sub_span =
    frame_span.subspan((p_start_pixel * bytes_per_pixel), num_bytes);
  set_range_ws2812b_frame_pixel(sub_span, p_pixel);
}

void set_range_ws2812b_frame_pixel(std::span<hal::byte>& p_frame,
                                   rgb888& p_pixel)
{
  static constexpr uint32_t bytes_per_pixel = 12;
  hal::byte current_byte_within_pixel, shift_byte;

  uint32_t pixel_count = p_frame.size() / bytes_per_pixel;
  uint32_t formatted_color_data = p_pixel.g << 16 | p_pixel.r << 8 | p_pixel.b;

  for (std::size_t current_pixel = 0; current_pixel < pixel_count;
       current_pixel++) {
    current_byte_within_pixel = 0;
    shift_byte = 1;

    for (int bit_shift_amount = 23; bit_shift_amount >= 0; bit_shift_amount--) {
      hal::byte formatted_data_current_bit =
        (formatted_color_data >> bit_shift_amount) & 0x01;
      hal::byte bit_encoding =
        (formatted_data_current_bit == 1) ? 0b1110 : 0b1000;

      if (shift_byte == 1) {
        p_frame[current_pixel * bytes_per_pixel + current_byte_within_pixel] &=
          0x0F;
        p_frame[current_pixel * bytes_per_pixel + current_byte_within_pixel] |=
          (bit_encoding << 4);
      } else if (shift_byte == 0) {
        p_frame[current_pixel * bytes_per_pixel + current_byte_within_pixel] &=
          0xF0;
        p_frame[current_pixel * bytes_per_pixel + current_byte_within_pixel] |=
          bit_encoding;
      }

      shift_byte = 1 - shift_byte;

      if (shift_byte == 1) {
        current_byte_within_pixel++;
      }
    }
  }
}
