#ifndef LED_H
#define LED_H


class LED {
public:
    LED();
    ~LED() = default;
    static void init();
    static void turnOn();
    static void turnOff();
    static void blinkOnce(unsigned delay);
    static void blinkForDuration(unsigned delay, unsigned duration);
    static void blinkForCycles(unsigned delay, unsigned cycles);
    static void blinkInf(unsigned delay);
};


#endif //LED_H
