#ifndef __SEQUENCER_H_H
#define __SEQUENCER_H_H

#include <cstdint>
#include <iostream>
#include <functional>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <atomic>

class Service {
public:
    Service(uint8_t affinity, uint8_t priority, uint32_t period);
    virtual ~Service() = default;

    virtual void _doService() = 0;  // Must be overridden by derived classes

    void stop();
    void release();

    uint32_t get_period() const;

protected:
    void _initializeService();
    void _provideService();

    uint8_t _affinity;
    uint8_t _priority;
    uint32_t _period;

    std::atomic<bool> _running{true};

    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> ready{false};
};
#endif // __SEQUENCER_H_H
