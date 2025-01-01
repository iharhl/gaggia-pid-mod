#ifndef LED_H
#define LED_H


class LED {
public:
    LED();
    ~LED() = default;
    static bool init();
    static void turnOn();
    static void turnOff();
    static void blinkOnce(unsigned delay);
    static void blinkForDuration(unsigned delay, unsigned duration);
    static void blinkForCycles(unsigned delay, unsigned cycles);
};


#endif //LED_H
