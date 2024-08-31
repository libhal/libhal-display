#include <libhal-display/apa102.hpp>

#include <libhal-util/spi.hpp>

namespace hal::display {

apa102::apa102(hal::spi& p_spi, hal::output_pin& p_chip_select)
  : m_spi(&p_spi)
  , m_chip_select(&p_chip_select)
{
  m_spi->configure(hal::spi::settings{ 250.0_kHz, { false }, { false } });
}

// public
void apa102::update(std::span<hal::byte> p_data)
{
  m_chip_select->level(false);
  hal::write(*m_spi, std::array<hal::byte, 4>{ 0x00, 0x00, 0x00, 0x00 });
  hal::write(*m_spi, p_data);
  hal::write(*m_spi, std::array<hal::byte, 4>{ 0xFF, 0xFF, 0xFF, 0xFF });
  m_chip_select->level(true);
}

}  // namespace hal::display
