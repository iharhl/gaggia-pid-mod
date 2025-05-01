#include "pump.h"
#include "clock.h"

#include <hardware/gpio.h>
#include <pico/time.h>


Pump::Pump(const uint8_t brewpin, const uint8_t relaypin) :
            m_brewpin(brewpin),
            m_relaypin(relaypin),
            m_autotimer(true) {
    // Set up input brew switch pin
    gpio_init(m_brewpin);
    gpio_set_dir(m_brewpin, GPIO_IN);
    gpio_pull_up(m_brewpin);
    // Set up output pump relay pin
    gpio_init(m_relaypin);
    gpio_set_dir(m_relaypin, GPIO_OUT);
    gpio_put(m_relaypin, false); // NC by default
}

Pump::Pump(const uint8_t brewpin) :
            m_brewpin(brewpin),
            m_relaypin(0), // assign to GP0 which is not used
            m_autotimer(false) {
    // Set up input brew switch pin
    gpio_init(m_brewpin);
    gpio_set_dir(m_brewpin, GPIO_IN);
    gpio_pull_up(m_brewpin);
}

uint8_t Pump::updateState() {
    const bool brew_switch_pressed = !gpio_get(m_brewpin);
    // State machine
    switch (m_state) {
        case STATE_IDLE:
            if (brew_switch_pressed) {
                m_starttime = Clock::now_sec();
                m_state = STATE_BREWING;
                // If auto timer enabled -> keep relay closed
                if (m_autotimer)
                    gpio_put(m_relaypin, false);
            }
            m_brewtime = 0;
            break;
        case STATE_BREWING:
            if (!brew_switch_pressed) {
                m_state = STATE_IDLE;
                break;
            }
            // Get the time since the brew button was pressed
            m_brewtime = Clock::now_sec() - m_starttime;
            // If auto shot time control enabled -> open the relay
            // until the brew switch is reset
            if (Clock::now_sec() >= m_starttime + brewtime and m_autotimer) {
                gpio_put(m_relaypin, true);
                m_state = STATE_WAITFORSWITCHOFF;
            }
            break;
        case STATE_WAITFORSWITCHOFF:
            // After brewing with auto timer, return to idle only if brew
            // switch was switched off by the user
            if (!brew_switch_pressed) {
                gpio_put(m_relaypin, false); // put relay back to closed
                m_state = STATE_IDLE;
            }
            break;
    }
    return m_brewtime;
}
