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

#include <libhal/units.hpp>

#include <libhal-armcortex/dwt_counter.hpp>
#include <libhal-armcortex/startup.hpp>
#include <libhal-armcortex/system_control.hpp>

#include <libhal-arm-mcu/stm32f1/clock.hpp>
#include <libhal-arm-mcu/stm32f1/constants.hpp>
#include <libhal-arm-mcu/stm32f1/input_pin.hpp>
#include <libhal-arm-mcu/stm32f1/output_pin.hpp>
#include <libhal-arm-mcu/stm32f1/spi.hpp>
#include <libhal-arm-mcu/stm32f1/uart.hpp>
#include <libhal-soft/bit_bang_spi.hpp>
#include <libhal-util/steady_clock.hpp>

#include <resource_list.hpp>

resource_list initialize_platform()
{
  using namespace hal::literals;

  constexpr bool use_bit_bang_spi = true;

  // Set the MCU to the maximum clock speed
  hal::stm32f1::maximum_speed_using_internal_oscillator();

  static hal::cortex_m::dwt_counter counter(
    hal::stm32f1::frequency(hal::stm32f1::peripheral::cpu));

  static hal::stm32f1::uart uart1(hal::port<1>,
                                  hal::buffer<128>,
                                  hal::serial::settings{
                                    .baud_rate = 115200,
                                  });

  auto cpu_frequency = hal::stm32f1::frequency(hal::stm32f1::peripheral::cpu);
  static hal::cortex_m::dwt_counter steady_clock(cpu_frequency);
  static hal::stm32f1::output_pin led('C', 13);
  static hal::stm32f1::output_pin spi_chip_select('A', 4);
  spi_chip_select.level(true);

  hal::spi* spi = nullptr;
  static hal::spi::settings spi_settings{
    .clock_rate = 250.0_kHz,
    .clock_polarity = false,
    .clock_phase = true,
  };

  if constexpr (use_bit_bang_spi) {
    static hal::stm32f1::output_pin sck('A', 5);
    static hal::stm32f1::output_pin copi('A', 6);
    static hal::stm32f1::input_pin cipo('A', 7);
    static hal::soft::bit_bang_spi::pins bit_bang_spi_pins{ .sck = &sck,
                                                            .copi = &copi,
                                                            .cipo = &cipo };

    static hal::soft::bit_bang_spi bit_bang_spi(
      bit_bang_spi_pins, steady_clock, spi_settings);
    spi = &bit_bang_spi;
  } else {
    static hal::stm32f1::spi spi1(hal::bus<1>, spi_settings);
    spi = &spi1;
  }

  return { .reset = +[]() { hal::cortex_m::reset(); },
           .console = &uart1,
           .clock = &counter,
           .status_led = &led,
           .spi = spi,
           .spi_chip_select = &spi_chip_select };
}
