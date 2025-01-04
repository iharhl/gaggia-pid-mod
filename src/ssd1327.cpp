#include "ssd1327.h"

SSD1327::SSD1327(I2CDevice* i2c_device) : m_i2cdevice(i2c_device) {
    // Send a sequence of commands to initalize the display
    m_i2cdevice->write8(init_cmd, sizeof(init_cmd));
}

void SSD1327::clearDisplay() const {
    // Send command to set all pixels of the display OFF
    m_i2cdevice->write8(SSD1327_DISPLAY_ALL_OFF);
}

void SSD1327::fillDisplay() const {
    // Send command to set all pixels of the display ON
    m_i2cdevice->write8(SSD1327_DISPLAY_ALL_ON);
}

void SSD1327::drawRegion(const uint8_t* data, const uint8_t x, const uint8_t y,
                    const uint8_t width, const uint8_t height) {
    // Allocate buffer to send set column/row cmds
    uint8_t buff[3];

    // Set column address
    buff[0] = SSD1327_SET_COLUMN_ADDR;
    buff[1] = static_cast<uint8_t>(x / 2); // start column (divided by 2 for 4-bit grayscale)
    buff[2] = static_cast<uint8_t>((x + width - 1) / 2); // end column
    m_i2cdevice->write8(buff, 3);

    // Set row address
    buff[0] = SSD1327_SET_ROW_ADDR;
    buff[1] = y; // start row
    buff[2] = y + height - 1; // end row
    m_i2cdevice->write8(buff, 3);

    // Write pixel data to the display
    // Divide by 2 as each byte holds two 4-bit pixels
    m_i2cdevice->write8(data, sizeof(data) / 2);
}