#ifndef __SEQUENCER_H_H
#define __SEQUENCER_H_H

#include <iostream>
#include <cstdint>
#include <functional>
#include <thread>
#include <vector>
#include <semaphore>

// The service class contains the service function and service parameters
// (priority, affinity, etc). It spawns a thread to run the service, configures
// the thread as required, and executes the service whenever it gets released.
class Service
{
public:
    //template<typename T>
    
    Service(uint8_t affinity, uint8_t priority, uint32_t period) 
    // : _doService(std::forward<T>(doService)), 
    //: _doService(doService), 
    _affinity(affinity), 
    _priority(priority), 
    _period(period), 
    _semaphore(0), 
    _running(true)
    {
        // Store configuration values
        std::cout << "Service created with affinity: " << (int)_affinity 
        << ", priority: " << (int)_priority 
        << ", period: " << _period << "ms\n";
        
        // Start the service thread, which will begin running the given function immediately
        _service = std::jthread(&Service::_provideService, this);
    }

 
    void stop();
 
    void release();
    int get_period() { return _period; } // Added getter for period
    // Service() = default;
    // Service(const Service&) = delete;  // Disable copy
    // Service& operator=(const Service&) = delete;
    // Service(Service&&) noexcept = default; // Allow move
    // Service& operator=(Service&&) noexcept = default;
    // Service(Service&& other) noexcept
    //     : some_int(other.some_int),
    //       some_pointer(std::exchange(other.some_pointer, nullptr)) // Properly handle pointers
    // {
    //     // Handle non-movable members like atomic and semaphore
    // }
    // Service& operator=(Service&& other) noexcept {
    //     if (this != &other) {
    //         some_int = other.some_int;
    //         some_pointer = std::exchange(other.some_pointer, nullptr);
    //         // Handle non-movable members carefully
    //     }
    //     return *this;
    // }


private:
    //std::function<void(void)> _doService;
    
    uint8_t _affinity;
    uint8_t _priority;  
    uint32_t _period;
    std::binary_semaphore _semaphore;
    std::atomic<bool> _running;  // Atomic variable to control service execution

    std::jthread _service;
    
    // int some_int;
    // int* some_pointer;
    // std::atomic<bool> some_flag;   // Cannot be moved
    // std::counting_semaphore<1> sem; // Cannot be moved
    virtual void _doService() = 0;

    void _initializeService();

    void _provideService();
};
 
// The sequencer class contains the services set and manages
// starting/stopping the services. While the services are running,
// the sequencer releases each service at the requisite timepoint.
class Sequencer
{
public:
    template<typename... Args>
    void addService(Args &&... args)
    {
        // Add the new service to the services list,
        // constructing it in-place with the given args
        // _services.emplace_back(std::forward<Args>(args)...);
        
        _services.emplace_back(std::make_unique<Service>(std::forward<Args>(args)...));
    }

    void startServices();

    void stopServices();
    
    const std::vector<std::unique_ptr<Service>>& getServices() const {
        return _services;
    }
    private:
    // std::vector<std::make_unique<Service>> _services;
    std::vector<std::unique_ptr<Service>> _services; // Store services as unique_ptrs
    std::atomic<bool> _seq_running{false};
    std::jthread timer_thread;
    timer_t posix_timer;

    static void timer_irq_service(union sigval sv);

    void timer_service();
};

#endif // __SEQUENCER_H_H
