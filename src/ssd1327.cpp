#include "ssd1327.h"
#include <iterator>
#include <string.h> // todo: remove


/* ==================== PUBLIC METHODS ======================= */

SSD1327::SSD1327(I2CDevice* i2c_device, const uint8_t size_x, const uint8_t size_y) :
    m_i2cdevice(i2c_device), m_sizeX(size_x), m_sizeY(size_y)
{
    // Send a sequence of commands to initalize the display
    sendCommandList(init_cmd_list, sizeof(init_cmd_list));
    // Turn off all the pixels at initialization
    turnOffAllPixels();
}

/* Send command to set all pixels of the display OFF */
void SSD1327::clearDisplay() {
    constexpr uint8_t buff[2] = { 0x00, SSD1327_DISPLAY_ALL_OFF };
    m_i2cdevice->write8(buff, 2);
}

/* Send command to set all pixels of the display ON */
void SSD1327::fillDisplay() {
    constexpr uint8_t buff[2] = { 0x00, SSD1327_DISPLAY_ALL_ON };
    m_i2cdevice->write8(buff, 2);
}

void SSD1327::resetDisplay() {
    constexpr uint8_t buff[2] = { 0x00, SSD1327_DISPLAY_NORMAL };
    m_i2cdevice->write8(buff, 2);
}

void SSD1327::drawRegion(const uint8_t* data, const uint8_t x, const uint8_t y,
                         const uint8_t width, const uint8_t height) {
    configureDrawingRegion(x, y, width, height);
    // Write pixel data to the display
    // Divide by 2 as each byte holds two 4-bit pixels
    sendData(data, width * height / 2);
}


/* ==================== PRIVATE METHODS ======================= */

void SSD1327::turnOffAllPixels() {
    configureDrawingRegion(0, 0, m_sizeX, m_sizeY);
    // Write pixel data to the display
    const uint8_t size = m_sizeX * m_sizeY / 2;
    const uint8_t data[size] = { }; // would be nice to make constexpr
    sendData(data, size);
}

void SSD1327::configureDrawingRegion(const uint8_t x, const uint8_t y,
                                     const uint8_t width, const uint8_t height) {
    // Allocate buffer to send set column/row cmds
    uint8_t buff[3];
    // Set column address
    buff[0] = SSD1327_SET_COLUMN_ADDR;
    buff[1] = static_cast<uint8_t>(x / 2); // start column (divided by 2 for 4-bit grayscale)
    buff[2] = static_cast<uint8_t>((x + width) / 2 - 1); // end column
    sendCommandList(buff, 3);
    // Set row address
    buff[0] = SSD1327_SET_ROW_ADDR;
    buff[1] = y; // start row
    buff[2] = y + height - 1; // end row
    sendCommandList(buff, 3);
}

void SSD1327::sendCommandList(const uint8_t* cmd_list, const unsigned len) {
    uint8_t buff[2];
    // Prepend control byte before data byte as per datasheet
    for (auto it = cmd_list; it != cmd_list + len; ++it) {
        buff[0] = 0x00; // Co and D/C bits = 0
        buff[1] = *it;
        m_i2cdevice->write8(buff, 2);
    }
}

void SSD1327::sendData(const uint8_t* data, const unsigned len) {
    // m_i2cdevice->write8(0x40, true); // Co=0 and D/C=1
    // m_i2cdevice->write8(data, len);

    // todo: fix
    uint8_t buffer[1 + len];
    buffer[0] = 0x40;  // Control byte: Co=0, D/C#=1 (Data mode)
    memcpy(buffer + 1, data, len);
    m_i2cdevice->write8(buffer, len+1);
}