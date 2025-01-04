#ifndef SSD1327_H
#define SSD1327_H

#include <cstdint>
#include "i2c.h"


typedef enum ssd1327_cmd {
    SSD1327_SET_COLUMN_ADDR = 0x15,
    SSD1327_SET_ROW_ADDR = 0x75,
    SSD1327_SET_CONTRAST = 0x81,
    SSD1327_SEG_REMAP = 0xA0,
    SSD1327_SET_START_LINE = 0xA1,
    SSD1327_SET_DISPLAY_OFFSET = 0xA2,
    SSD1327_SET_MODE_NORMAL = 0xA4,
    SSD1327_DISPLAY_ALL_ON = 0xA5, // set all pixels to ON
    SSD1327_DISPLAY_ALL_OFF = 0xA6, // set all pixels to OFF
    SSD1327_INVERT_DISPLAY = 0xA7,
    SSD1327_SET_MULTIPLEX = 0xA8,
    SSD1327_ENABLE_VDD_REGULATOR = 0xAB,
    SSD1327_DISPLAY_OFF = 0xAE, // display sleep cmd
    SSD1327_DISPLAY_ON = 0xAF, // display wake up cmd
    SSD1327_SET_PHASE_LEN = 0xB1,
    SSD1327_SET_CLK_DIV = 0xB3,
    SSD1327_SET_PRECHARGE2 = 0xB6,
    SSD1327_SET_GRAY_TABLE = 0xB8,
    SSD1327_SET_PRECHARGE = 0xBC,
    SSD1327_SET_VCOM = 0xBE,
    SSD1327_FUNC_SEL_B = 0xD5,
} ssd1327_cmd_e;

// Only use 3 grayscale colors
typedef enum ssd1327_pixel_color {
    SSD1327_BLACK = 0x0,
    SSD1327_GRAY = 0x8,
    SSD1327_WHITE = 0xF,
} ssd1327_pixel_color_e;

constexpr uint8_t init_cmd[] = {
    SSD1327_DISPLAY_OFF,
    // SSD1327_SET_CONTRAST, 0x80,
    SSD1327_SEG_REMAP, 0x46, // different from adafruit driver
    SSD1327_SET_START_LINE, 0x00,
    SSD1327_SET_DISPLAY_OFFSET, 0x00,
    SSD1327_DISPLAY_ALL_OFF,
    SSD1327_SET_MULTIPLEX, 0x7F,
    SSD1327_SET_PHASE_LEN, 0x11, // increase for better stability/brightness
    SSD1327_SET_CLK_DIV, 0xF1, // lower refresh rate as only static images are used
    SSD1327_ENABLE_VDD_REGULATOR, 0x01,
    SSD1327_SET_PRECHARGE2, 0x04, // same consideration as for phase length
    SSD1327_SET_VCOM, 0x0F, // same as above
    SSD1327_SET_PRECHARGE, 0x08, // same as above
    SSD1327_FUNC_SEL_B, 0x62, // todo: should be before or after precharge2?
    // SSD1327_CMDLOCK, 0x12, // 0xFD
    SSD1327_SET_MODE_NORMAL,
    SSD1327_DISPLAY_ON
};


class SSD1327 {
public:
    explicit SSD1327(I2CDevice* i2c_device);
    ~SSD1327() = default;

    void clearDisplay() const;
    void fillDisplay() const; // turn on all of the pixels

    void drawRegion(const uint8_t* data, uint8_t x, uint8_t y,
        uint8_t width, uint8_t height);
private:
    I2CDevice* m_i2cdevice;
};


#endif //SSD1327_H
