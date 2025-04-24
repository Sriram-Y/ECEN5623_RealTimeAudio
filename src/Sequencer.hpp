/*
 * This is a C++ version of the canonical pthread service example. It intends
 * to abstract the service management functionality and sequencing for ease
 * of use. Much of the code is left to be implemented by the student.
 *
 * Build with g++ --std=c++23 -Wall -Werror -pedantic
 * Steve Rizor 3/16/2025
 */

// /* V1

#pragma once

#include <cstdint>
#include <functional>
#include <thread>
#include <vector>
#include <semaphore>
#include <iostream>

// The service class contains the service function and service parameters
// (priority, affinity, etc). It spawns a thread to run the service, configures
// the thread as required, and executes the service whenever it gets released.
class Service
{
public:
    virtual void _doService() = 0;
    template<typename T>
    // Service(T&& doService, uint8_t affinity, uint8_t priority, uint32_t period) :
    //     _doService(doService)
    // {
    //     // todo: store service configuration values
    //     // todo: initialize release semaphore
    //     // Start the service thread, which will begin running the given function immediately
    //     _service = std::jthread(&Service::_provideService, this);
    // }
    
    Service(uint8_t affinity, uint8_t priority, uint32_t period);

 
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
    std::counting_semaphore<1> _semaphore{0};
    std::atomic<bool> _running;  // Atomic variable to control service execution

    std::jthread _service;
    
    // int some_int;
    // int* some_pointer;
    // std::atomic<bool> some_flag;   // Cannot be moved
    // std::counting_semaphore<1> sem; // Cannot be moved

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

// */

/* V0 cpy

// The service class contains the service function and service parameters
// (priority, affinity, etc). It spawns a thread to run the service, configures
// the thread as required, and executes the service whenever it gets released.
class Service
{
public:
    template<typename T>
    Service(T&& doService, uint8_t affinity, uint8_t priority, uint32_t period) :
        _doService(doService)
    {
        // todo: store service configuration values
        // todo: initialize release semaphore

        // Start the service thread, which will begin running the given function immediately
        _service = std::jthread(&Service::_provideService, this);
    }
 
    void stop(){
        // todo: change state to "not running" using an atomic variable
        // (heads up: what if the service is waiting on the semaphore when this happens?)
    }
 
    void release(){
        // todo: release the service using the semaphore
    }
 
private:
    std::function<void(void)> _doService;
    std::jthread _service;

    void _initializeService()
    {
        // todo: set affinity, priority, sched policy
        // (heads up: the thread is already running and we're in its context right now)
    }

    void _provideService()
    {
        _initializeService();
        // todo: call _doService() on releases (sem acquire) while the atomic running variable is true
    }
};
 
// The sequencer class contains the services set and manages
// starting/stopping the services. While the services are running,
// the sequencer releases each service at the requisite timepoint.
class Sequencer
{
public:
    template<typename... Args>
    void addService(Args&&... args)
    {
        // Add the new service to the services list,
        // constructing it in-place with the given args
        _services.emplace_back(std::forward<Args>(args)...);
    }

    void startServices()
    {
        // todo: start timer(s), release services
    }

    void stopServices()
    {
        // todo: stop timer(s), stop services
    }

private:
    std::vector<Service> _services;
};

*/

/*V2

//Sequencer.hpp

#pragma once

#include <cstdint>
#include <vector>
#include <functional>
#include <thread>
#include <semaphore.h>
#include <atomic>
#include <iostream>
#include <sched.h>
#include <pthread.h>

// The sequencer class contains the services set and manages
// starting/stopping the services. While the services are running,
// the sequencer releases each service at the requisite timepoint.
class Sequencer
{
public:
    // Add a new service to the sequencer with the given arguments
    template<typename... Args>
    void addService(Args&&... args)
    {
        _services.emplace_back(std::forward<Args>(args)...);
    }

    // Start all services and release them at the correct times
    void startServices()
    {
        // Timer-based mechanism to release services at their periods
        for (auto& service : _services) {
            service.release();  // Trigger service execution immediately (you can add periodic logic here)
        }
    }

    // Stop all services
    void stopServices()
    {
        for (auto& service : _services) {
            service.stop();  // Stop the service
        }
    }

private:
    std::vector<Service> _services;  // List of services managed by the sequencer
};


//Service.hpp

// The service class contains the service function and service parameters
// (priority, affinity, etc). It spawns a thread to run the service, configures
// the thread as required, and executes the service whenever it gets released.
class Service
{
public:
    // Constructor accepts the service function, affinity, priority, and period
    template<typename T>
    Service(T&& doService, uint8_t affinity, uint8_t priority, uint32_t period)
        : _doService(std::forward<T>(doService)),
          _affinity(affinity), _priority(priority), _period(period),
          _running(true)
    {
        // Initialize semaphore for service release
        _semaphore = std::make_shared<std::counting_semaphore<1>>(0);

        // Start the service thread, calling _provideService on the new thread
        _service = std::jthread(&Service::_provideService, this);
    }

    // Stop the service thread
    void stop()
    {
        _running = false;  // Mark the service as stopped
        _semaphore->release();  // Release semaphore if waiting
    }

    // Release the service (trigger the service execution)
    void release()
    {
        _semaphore->release();  // Allow service to run
    }

private:
    std::function<void(void)> _doService;
    std::jthread _service;
    std::shared_ptr<std::counting_semaphore<1>> _semaphore;
    uint8_t _affinity;
    uint8_t _priority;
    uint32_t _period;
    std::atomic<bool> _running;

    // Initialize service configuration (thread affinity, priority, etc.)
    void _initializeService()
    {
        // Set thread affinity for specific CPU core (example)
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(_affinity, &cpuset);
        if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0) {
            std::cerr << "Error setting thread affinity" << std::endl;
        }

        // Set thread priority and scheduling policy
        struct sched_param sched;
        sched.sched_priority = _priority;
        if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &sched) != 0) {
            std::cerr << "Error setting thread priority" << std::endl;
        }
    }

    // Main service thread function
    void _provideService()
    {
        _initializeService();

        // Run until service is stopped
        while (_running) {
            _semaphore->acquire();  // Wait for release signal

            if (_running) {
                _doService();  // Execute the service function
            }
        }
    }
};

*/
