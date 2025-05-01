#include "gui.h"
#include "clock.h"


DisplayManager::DisplayManager(SSD1327 *display) : m_display(display)
{
    displayHome();
}

void DisplayManager::updateTemperature(const uint8_t temp) {
    // Update not faster than once a second
    const uint64_t now = Clock::now_ms();
    if (now - m_prevTempUpdateTime < 1000)
        return;
    m_prevTempUpdateTime = now;
    // Split value into 3 digits
    const uint8_t hundreds = temp / 100;
    const uint8_t tens = (temp / 10) % 10;
    const uint8_t ones = temp % 10;
    // Update each digit if changed
    if (hundreds != m_temp1) {
        m_temp1 = hundreds;
        m_display->drawRegion(digit_map[hundreds], NUMBER_FIELD1_X, NUMBER_FIELD1_Y,
            NUMBER_FIELD_WIDTH, NUMBER_FIELD_HEIGHT);
    }
    if (tens != m_temp2) {
        m_temp2 = tens;
        m_display->drawRegion(digit_map[tens], NUMBER_FIELD2_X, NUMBER_FIELD2_Y,
            NUMBER_FIELD_WIDTH, NUMBER_FIELD_HEIGHT);
    }
    if (ones != m_temp3) {
        m_temp3 = ones;
        m_display->drawRegion(digit_map[ones], NUMBER_FIELD3_X, NUMBER_FIELD3_Y,
            NUMBER_FIELD_WIDTH, NUMBER_FIELD_HEIGHT);
    }
}

void DisplayManager::updateStatus(const uint8_t context, const uint8_t code) {
    // Return if status is already displayed
    if (context == m_status1 and code == m_status2)
        return;
    // Update not faster than once in two second
    const uint64_t now = Clock::now_ms();
    if (now - m_prevStatusUpdateTime < 2000)
        return;
    m_prevStatusUpdateTime = now;
    // Draw letter and digit corresponding to the context and code received
    m_display->drawRegion(letter_map[context], TEXT_FIELD1_X, TEXT_FIELD1_Y,
            TEXT_FIELD_WIDTH, TEXT_FIELD_HEIGHT);
    m_display->drawRegion(digit_map[code], NUMBER_FIELD4_X, NUMBER_FIELD4_Y,
            NUMBER_FIELD_WIDTH, NUMBER_FIELD_HEIGHT);
    // Store displayed status
    m_status1 = context;
    m_status2 = code;
}

void DisplayManager::resetStatus(const uint8_t context, const uint8_t code) {
    // Return if status to be reset is not displayed
    if (context != m_status1 or code != m_status2)
        return;
    // Update not faster than once in two second
    const uint64_t now = Clock::now_ms();
    if (now - m_prevStatusUpdateTime < 2000)
        return;
    m_prevStatusUpdateTime = now;
    // Reset status to OK
    m_display->drawRegion(NUMBER0, TEXT_FIELD1_X, TEXT_FIELD1_Y,
            TEXT_FIELD_WIDTH, TEXT_FIELD_HEIGHT);
    m_display->drawRegion(LETTERK, NUMBER_FIELD4_X, NUMBER_FIELD4_Y,
            NUMBER_FIELD_WIDTH, NUMBER_FIELD_HEIGHT);
    // Store displayed status (0xFF is default)
    m_status1 = m_status2 = 0xFF;
}

void DisplayManager::updateShotTime(const uint8_t time) {
    // Update not faster than once in half a second
    const uint64_t now = Clock::now_ms();
    if (now - m_prevShotUpdateTime < 950)
        return;
    m_prevShotUpdateTime = now;
    // Handle time > 99 sec
    uint8_t tens, ones;
    if (time <= 99) {
        tens = (time / 10) % 10;
        ones = time % 10;
    }
    else {
        tens = 9;
        ones = 9;
    }
    // After brew switch is reset, start a time to hold the brew time on the
    // display for 5 sec (5000 ms)
    if (time == 0 and (m_shottime1 != 0 or m_shottime2 != 0) and m_brewHoldTimeStart == 0) {
        m_brewHoldTimeStart = now;
        return;
    }
    // Update each digit if one of them is changed and if brew hold time is expired
    if (tens != m_shottime1 and now - m_brewHoldTimeStart > 5000) {
        m_shottime1 = tens;
        m_brewHoldTimeStart = 0;
        m_display->drawRegion(digit_map[tens], NUMBER_FIELD5_X, NUMBER_FIELD5_Y,
            NUMBER_FIELD_WIDTH, NUMBER_FIELD_HEIGHT);
    }
    if (ones != m_shottime2 and now - m_brewHoldTimeStart > 5000) {
        m_shottime2 = ones;
        m_brewHoldTimeStart = 0;
        m_display->drawRegion(digit_map[ones], NUMBER_FIELD6_X, NUMBER_FIELD6_Y,
            NUMBER_FIELD_WIDTH, NUMBER_FIELD_HEIGHT);
    }
}

void DisplayManager::displayHome() {
    // Draw thermometer icon
    m_display->drawRegion(THERM, ICON1_X, ICON1_Y, ICON1_WIDTH, ICON1_HEIGHT);
    // Draw 3 digits as 0
    m_display->drawRegion(NUMBER0, NUMBER_FIELD1_X, NUMBER_FIELD1_Y,
            NUMBER_FIELD_WIDTH, NUMBER_FIELD_HEIGHT);
    m_display->drawRegion(NUMBER0, NUMBER_FIELD2_X, NUMBER_FIELD2_Y,
            NUMBER_FIELD_WIDTH, NUMBER_FIELD_HEIGHT);
    m_display->drawRegion(NUMBER0, NUMBER_FIELD3_X, NUMBER_FIELD3_Y,
            NUMBER_FIELD_WIDTH, NUMBER_FIELD_HEIGHT);
    // Store displayed digits
    m_temp1 = m_temp2 = m_temp3 = 0;
    // Draw status icon
    m_display->drawRegion(STATUS, ICON2_X, ICON2_Y, ICON2_WIDTH, ICON2_HEIGHT);
    // Draw OK status
    m_display->drawRegion(NUMBER0, TEXT_FIELD1_X, TEXT_FIELD1_Y,
            TEXT_FIELD_WIDTH, TEXT_FIELD_HEIGHT);
    m_display->drawRegion(LETTERK, NUMBER_FIELD4_X, NUMBER_FIELD4_Y,
            NUMBER_FIELD_WIDTH, NUMBER_FIELD_HEIGHT);
    // Store displayed status (0xFF is default)
    m_status1 = m_status2 = 0xFF;
    // Draw cup icon
    m_display->drawRegion(CUP, ICON3_X, ICON3_Y, ICON3_WIDTH, ICON3_HEIGHT);
    // Draw 2 digits as 0
    m_display->drawRegion(NUMBER0, NUMBER_FIELD5_X, NUMBER_FIELD5_Y,
            NUMBER_FIELD_WIDTH, NUMBER_FIELD_HEIGHT);
    m_display->drawRegion(NUMBER0, NUMBER_FIELD6_X, NUMBER_FIELD6_Y,
            NUMBER_FIELD_WIDTH, NUMBER_FIELD_HEIGHT);
    // Store displayed digits
    m_shottime1 = m_shottime2 = 0;
}
