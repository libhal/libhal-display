#include <libhal-display/apa102.hpp>

#include <libhal-util/as_bytes.hpp>
#include <libhal-util/spi.hpp>
#include <span>

namespace hal::display {

apa102::apa102(hal::spi& p_spi, hal::output_pin& p_chip_select)
  : m_spi(&p_spi)
  , m_chip_select(&p_chip_select)
{
  // 1 MHz is max speed LEDs can handle
  m_spi->configure(hal::spi::settings{ 1.0_MHz, { false }, { false } });
}

// public
void apa102::update(std::span<apa102_pixel> p_data)
{
  m_chip_select->level(false);
  hal::write(*m_spi, std::array<hal::byte, 4>{ 0x00, 0x00, 0x00, 0x00 });
  hal::write(*m_spi, hal::as_bytes(p_data));
  hal::write(*m_spi, std::array<hal::byte, 4>{ 0xFF, 0xFF, 0xFF, 0xFF });
  m_chip_select->level(true);
}
}  // namespace hal::display
