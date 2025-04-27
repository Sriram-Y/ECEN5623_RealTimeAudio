/*
 * This is a C++ version of the canonical pthread service example. It intends
 * to abstract the service management functionality and sequencing for ease
 * of use. Much of the code is left to be implemented by the student.
 *
 * Build with g++ --std=c++23 -Wall -Werror -pedantic
 * Steve Rizor 3/16/2025
 */

#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <semaphore>
#include <thread>
#include <vector>
#include <pthread.h>
#include <sched.h>
#include <random>

// The service class contains the service function and service parameters
// (priority, affinity, etc). It spawns a thread to run the service, configures
// the thread as required, and executes the service whenever it gets released.
class Service
{
public:
    template <typename T>
    Service(T &&doService, uint8_t affinity, uint8_t priority, uint32_t period) : _doService(std::forward<T>(doService)),
                                                                                  _affinity(affinity),
                                                                                  _priority(priority),
                                                                                  _period(period),
                                                                                  _releaseSemaphore(0),
                                                                                  _running(true),
                                                                                  _serviceId(std::random_device{}())
    // todo (done): store service configuration values
    // todo (done): initialize release semaphore
    {
        // Start the service thread, which will begin running the given function immediately
        _service = std::jthread(&Service::_provideService, this);
    }

    void stop()
    {
        // todo (done): change state to "not running" using an atomic variable
        // (heads up: what if the service is waiting on the semaphore when this happens?)
        _running.store(false);
        _releaseSemaphore.release();
    }

    void release()
    {
        // todo (done): release the service using the semaphore
        _releaseSemaphore.release();
    }

    uint32_t getPeriod() const
    {
        return _period;
    }

private:
    std::function<void(void)> _doService;
    std::jthread _service;
    // set by constructor
    uint8_t _affinity;
    uint8_t _priority;
    uint32_t _period;
    std::binary_semaphore _releaseSemaphore;
    std::atomic<bool> _running;

    // logging stats
    uint32_t _serviceId = 0;
    std::chrono::steady_clock::time_point _lastReleaseTime;
    std::chrono::nanoseconds _minExecTime = std::chrono::nanoseconds::max();
    std::chrono::nanoseconds _maxExecTime = std::chrono::nanoseconds::min();
    std::chrono::nanoseconds _totalExecTime = std::chrono::nanoseconds::zero();
    size_t _executionCount = 0;
    std::chrono::nanoseconds _lastExecTime = std::chrono::nanoseconds::zero();
    std::chrono::nanoseconds _maxExecJitter = std::chrono::nanoseconds::zero();
    std::chrono::nanoseconds _maxStartJitter = std::chrono::nanoseconds::zero();

    void _initializeService()
    {
        // todo (done): set affinity
        // (heads up: the thread is already running and we're in its context right now)
        pthread_t native = pthread_self();

        // Set thread affinity
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(_affinity, &cpuset);
        int ret = pthread_setaffinity_np(native, sizeof(cpu_set_t), &cpuset);
        if (ret != 0)
        {
            std::cerr << "Error setting thread affinity\n";
        }

        // todo (done): set priority
        sched_param sch_params;
        sch_params.sched_priority = _priority;
        // todo (done): set sched policy
        ret = pthread_setschedparam(native, SCHED_FIFO, &sch_params);
        if (ret != 0)
        {
            std::cerr << "Failed to set thread scheduling: " << ret << "\n";
        }
    }

    void _provideService()
    {
        _initializeService();
        while (_running.load())
        {
            _releaseSemaphore.acquire();
            if (!_running.load())
            {
                break;
            }

            auto actualStartTime = std::chrono::steady_clock::now();
            auto startJitter = std::chrono::nanoseconds::zero();

            if (_executionCount > 0)
            {
                startJitter = (actualStartTime - _lastReleaseTime) - std::chrono::milliseconds(_period);
            }

            _maxStartJitter = std::max(_maxStartJitter, startJitter);

            auto execStart = std::chrono::steady_clock::now();
            _doService();
            auto execEnd = std::chrono::steady_clock::now();

            auto execTime = execEnd - execStart;
            _minExecTime = std::min(_minExecTime, execTime);
            _maxExecTime = std::max(_maxExecTime, execTime);
            _totalExecTime += execTime;

            if (_executionCount > 0)
            {
                auto execJitter = std::chrono::nanoseconds::zero();
                if (execTime > _lastExecTime)
                {
                    execJitter = execTime - _lastExecTime;
                }
                else
                {
                    execJitter = _lastExecTime - execTime;
                }

                _maxExecJitter = std::max(_maxExecJitter, execJitter);
            }

            _lastExecTime = execTime;
            _lastReleaseTime = actualStartTime;
            ++_executionCount;
        }

        _printStats();
    }

    void _printStats()
    {
        if (_executionCount == 0)
        {
            std::cout << "Service had no executions.\n";
            return;
        }

        auto avgExec = _totalExecTime / _executionCount;
        std::cout << "---- Service Stats ----\n";
        std::cout << "Service ID: " << _serviceId << "\n";
        std::cout << "Executions:       " << _executionCount << "\n";
        std::cout << "Min Exec Time:    " << std::chrono::duration_cast<std::chrono::microseconds>(_minExecTime).count() << " us\n";
        std::cout << "Max Exec Time:    " << std::chrono::duration_cast<std::chrono::microseconds>(_maxExecTime).count() << " us\n";
        std::cout << "Avg Exec Time:    " << std::chrono::duration_cast<std::chrono::microseconds>(avgExec).count() << " us\n";
        std::cout << "Max Exec Jitter:  " << std::chrono::duration_cast<std::chrono::microseconds>(_maxExecJitter).count() << " us\n";
        std::cout << "Max Start Jitter: " << std::chrono::duration_cast<std::chrono::microseconds>(_maxStartJitter).count() << " us\n";
        std::cout << "------------------------\n";
    }
};

// The sequencer class contains the services set and manages
// starting/stopping the services. While the services are running,
// the sequencer releases each service at the requisite timepoint.
class Sequencer
{
public:
    Sequencer() : _running(false) {}

    template <typename... Args>
    void addService(Args &&...args)
    {
        // Construct a new service in-place and store it as a unique_ptr.
        _services.push_back(std::make_unique<Service>(std::forward<Args>(args)...));
    }

    void startServices()
    {
        _running.store(true);
        // Start the sequencer timer thread that periodically releases services.
        _sequencerThread = std::thread([this]()
        {
            uint64_t tickCount = 1;
            constexpr auto tickDuration = std::chrono::microseconds(250);
            while (_running.load())
            {
                auto startTime = std::chrono::steady_clock::now();
                // For each service, if the tick count is a multiple of its period, release it.
                for (auto& service : _services)
                {
                    if (tickCount % service->getPeriod() == 0)
                    {
                        service->release();
                    }
                }
                ++tickCount;
                // Sleep until next tick
                std::this_thread::sleep_until(startTime + tickDuration);
            } 
        });
    }

    void stopServices()
    {
        // todo (done): start timer(s), release services
        _running.store(false);
        if (_sequencerThread.joinable())
        {
            _sequencerThread.join();
        }

        // Stop all the services
        for (auto &service : _services)
        {
            service->stop();
        }
    }

private:
    std::vector<std::unique_ptr<Service>> _services;
    std::atomic<bool> _running;
    std::thread _sequencerThread;
};
