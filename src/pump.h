#ifndef PUMP_H
#define PUMP_H

#include <cstdint>


typedef enum brew_state {
    STATE_IDLE,
    STATE_PREINFUSION,
    STATE_TRANSIENT,
    STATE_BREWING,
    STATE_WAITFORSWITCHOFF,
} brew_state_e;


class Pump {
public:
    explicit Pump(uint8_t brewpin, uint8_t relaypin);
    ~Pump() = default;

    uint8_t updateState();
    void enablePreInfusion(uint8_t duration);

    uint8_t brewtime = 5; // defaults to 28 sec

private:
    uint8_t m_brewpin, m_relaypin;
    uint8_t m_state = STATE_IDLE;

    bool m_preinfEnabled = false;
    uint8_t m_preinfDuration = 0;
    uint64_t m_starttime = 0;
};


#endif //PUMP_H
