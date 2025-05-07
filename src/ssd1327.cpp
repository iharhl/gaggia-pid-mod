#include "ssd1327.h"

#include <cstring>
#include <cstdlib>


/* ==================== PUBLIC METHODS ======================= */

SSD1327::SSD1327(I2CDevice* i2c_device, const uint8_t size_x, const uint8_t size_y) :
    m_i2cdevice(i2c_device),
    m_sizeX(size_x),
    m_sizeY(size_y) {
    // Send a sequence of commands to initialize the display
    sendCommandList(init_cmd_list, sizeof(init_cmd_list));
    // Turn off all the pixels at initialization
    setAllPixelsOff();
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

/* Clear prev commands like tune all pixels ON/OFF */
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
    // Allocate memory for len + 1 elements.
    // Need to dynamically allocate as the buffer size is not known during compile-time.
    // It is a bad idea to allocate the variable-size buffer on the stack.
    // Could use vector here, but it blows up the size of the binary.
    auto* buff = static_cast<uint8_t*>(malloc((len + 1) * sizeof(uint8_t)));
    // Handle memory allocation failure
    if (buff == nullptr)
        return; // todo: handle error
    // Set control byte
    buff[0] = 0x40;  // Control byte: Co=0, D/C#=1 (Data mode)
    // Copy the data into the remaining buffer space
    memcpy(&buff[1], data, len);
    // Send it over I2C
    m_i2cdevice->write8(buff, len+1);
    // Free the allocated memory
    free(buff);
}

void SSD1327::setAllPixelsOff() {
    configureDrawingRegion(0, 0, m_sizeX, m_sizeY);
    // Write pixel data to the display
    const unsigned size = m_sizeX * m_sizeY / 2;
    // Allocate and zero-initialize the memory
    auto* buff = static_cast<uint8_t*>(calloc(size + 1, sizeof(uint8_t)));
    // Handle allocation failure
    if (buff == nullptr)
        return; // todo: handle error
    // Set control byte
    buff[0] = 0x40;  // Control byte: Co=0, D/C#=1 (Data mode)
    // Send it over I2C
    m_i2cdevice->write8(buff, size+1);
    // Free the allocated memory
    free(buff);
}
