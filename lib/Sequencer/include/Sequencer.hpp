#ifndef __SEQUENCER_H_H
#define __SEQUENCER_H_H

#include <iostream>
#include <cstdint>
#include <functional>
#include <thread>
#include <vector>
#include <semaphore>
#include <condition_variable>

// The service class contains the service function and service parameters
// (priority, affinity, etc). It spawns a thread to run the service, configures
// the thread as required, and executes the service whenever it gets released.

class Service {
public:
    Service(uint8_t affinity, uint8_t priority, uint32_t period);
    virtual ~Service() = default;

    virtual void _doService() = 0;

    void run();
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

 
// The sequencer class contains the services set and manages
// starting/stopping the services. While the services are running,
// the sequencer releases each service at the requisite timepoint.
class Sequencer
{
public:
   Sequencer();
    ~Sequencer();

    void addService(Service* service);   // ✅ Accept raw pointer!

    void startServices();
    void stopServices();

private:
    std::vector<std::unique_ptr<Service>> _services;
    timer_t posix_timer;
};
#endif // __SEQUENCER_H_H
