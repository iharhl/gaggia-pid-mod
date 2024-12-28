#ifndef LED_H
#define LED_H


class LED {
public:
    LED() = default;
    ~LED() = default;
    static bool init();
    static void turnOn();
    static void turnOff();
    static void blink(unsigned delay);
};


#endif //LED_H
