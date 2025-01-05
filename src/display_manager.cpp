#include "display_manager.h"
#include "time.h"


DisplayManager::DisplayManager(SSD1327 *display) : m_display(display) {
    displayHome();
}

void DisplayManager::updateTemperature(const uint8_t temp) {
    // Update not faster than once a second (to preserve the display)
    const uint64_t now = Timer::now_ms();
    if (now - m_prevUpdateTime < 1000)
        return;
    m_prevUpdateTime = now;
    // Split value into 3 digits
    const uint8_t hundreds = temp / 100;
    const uint8_t tens = (temp / 10) % 10;
    const uint8_t ones = temp % 10;
    // Update each digit if changed
    if (hundreds != m_temp1) {
        m_temp1 = hundreds;
        m_display->drawRegion(digit_map[hundreds], NUMBER_FIELD1_X, NUMBER_FIELD1_Y,
            NUMBER_FIELD1_WIDTH, NUMBER_FIELD1_HEIGHT);
    }
    if (tens != m_temp2) {
        m_temp2 = tens;
        m_display->drawRegion(digit_map[tens], NUMBER_FIELD2_X, NUMBER_FIELD2_Y,
            NUMBER_FIELD2_WIDTH, NUMBER_FIELD2_HEIGHT);
    }
    if (ones != m_temp3) {
        m_temp3 = ones;
        m_display->drawRegion(digit_map[ones], NUMBER_FIELD3_X, NUMBER_FIELD3_Y,
            NUMBER_FIELD3_WIDTH, NUMBER_FIELD3_HEIGHT);
    }
}

void DisplayManager::displayHome() {
    m_display->drawRegion(THERM, ICON1_X, ICON1_Y, ICON1_WIDTH, ICON1_HEIGHT);
}

