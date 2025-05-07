#ifndef VSENS_H
#define VSENS_H

#include <cstdint>


class VsysMonitor {
public:
    explicit VsysMonitor(uint8_t pin, uint8_t channel);
    ~VsysMonitor() = default;

    [[nodiscard]] float readOnce() const;
    [[nodiscard]] float readAvg(uint8_t num) const;

private:
    uint8_t m_Pin, m_Channel;

    void configure();
    [[nodiscard]] bool isADCConfigured() const;
};

#endif //VSENS_H
