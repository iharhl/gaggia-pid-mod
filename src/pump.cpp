#include "pump.h"
#include "time.h"

#include <hardware/gpio.h>
#include <pico/time.h>


Pump::Pump(const uint8_t brewpin, const uint8_t relaypin) :
            m_brewpin(brewpin),
            m_relaypin(relaypin) {
    // Set up input brew switch pin
    gpio_init(m_brewpin);
    gpio_set_dir(m_brewpin, GPIO_IN);
    gpio_pull_up(m_brewpin);
    // Set up output pump relay pin
    gpio_init(m_relaypin);
    gpio_set_dir(m_relaypin, GPIO_OUT);
    gpio_put(m_relaypin, true); // closed by default
}

void Pump::enablePreInfusion(const uint8_t duration) {
    m_preinfDuration = duration;
    m_preinfEnabled = true;
}

uint8_t Pump::updateState() {
    const bool brew_switch_pressed = gpio_get(m_brewpin);
    // State machine
    switch (m_state) {
        case STATE_IDLE:
            if (brew_switch_pressed) {
                m_starttime = Clock::now_sec();
                if (m_preinfEnabled)
                    m_state = STATE_PREINFUSION;
                else
                    m_state = STATE_BREWING;
            }
            break;
        case STATE_PREINFUSION:
            // note: relay already closed
            if (!brew_switch_pressed) {
                m_state = STATE_IDLE;
                break;
            }
            if (Clock::now_sec() >= m_starttime + m_preinfDuration) {
                gpio_put(m_relaypin, false);
                m_starttime = Clock::now_sec();
                m_state = STATE_TRANSIENT;
            }
            break;
        case STATE_TRANSIENT:
            if (!brew_switch_pressed) {
                gpio_put(m_relaypin, true);
                m_state = STATE_IDLE;
                break;
            }
            // wait for 5 sec
            if (Clock::now_sec() >= m_starttime + 5) {
                gpio_put(m_relaypin, true);
                m_starttime = Clock::now_sec();
                m_state = STATE_BREWING;
            }
            break;
        case STATE_BREWING:
            if (!brew_switch_pressed) {
                m_state = STATE_IDLE;
                break;
            }
            if (Clock::now_sec() >= m_starttime + brewtime) {
                gpio_put(m_relaypin, false); // open the relay until the brew switch is off
                m_state = STATE_WAITFORSWITCHOFF;
            }
            break;
        case STATE_WAITFORSWITCHOFF:
            // After brewing with automatic timer, return to idle only if brew
            // switch was switched off by the user
            if (!brew_switch_pressed) {
                sleep_ms(500); // todo: should wait a bit for solenoid to close?
                gpio_put(m_relaypin, true); // put relay back to closed
                m_state = STATE_IDLE;
            }
            break;
    }
    return m_state;
}
