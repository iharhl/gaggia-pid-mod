#ifndef PUMP_H
#define PUMP_H

#include <cstdint>


typedef enum brew_state {
    STATE_IDLE,
    STATE_BREWING,
    STATE_WAITFORSWITCHOFF,
} brew_state_e;


class Pump {
public:
    explicit Pump(uint8_t brewpin, uint8_t relaypin);
    ~Pump() = default;

    uint8_t updateState();
    uint8_t brewtime = 28; // defaults to 28 sec

private:
    uint8_t m_brewpin, m_relaypin;
    uint8_t m_state = STATE_IDLE;
    uint64_t m_starttime = 0;
};


#endif //PUMP_H
