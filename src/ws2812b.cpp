// Copyright 2024 - 2025 Khalil Estell and the libhal contributors
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

#include <libhal-display/ws2812b.hpp>
#include <libhal-util/spi.hpp>

namespace hal::display {

ws2812b::ws2812b(hal::spi& p_spi, hal::output_pin& p_chip_select)
  : m_spi(&p_spi)
  , m_chip_select(&p_chip_select)
{
  m_spi->configure(hal::spi::settings{ 4.0_MHz, { false }, { false } });
}

void ws2812b::update(std::span<hal::byte> p_data)
{
  m_chip_select->level(false);
  hal::write(*m_spi, p_data);
  m_chip_select->level(true);
}

}  // namespace hal::display
