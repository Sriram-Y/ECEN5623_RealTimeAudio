/*
 * This is a C++ version of the canonical pthread service example. It intends
 * to abstract the service management functionality and sequencing for ease
 * of use. Much of the code is left to be implemented by the student.
 *
 * Build with g++ --std=c++23 -Wall -Werror -pedantic
 * Steve Rizor 3/16/2025
 */

#include <cstdint>
#include <cstdio>
#include <semaphore.h>
#include <cstdint>
#include <functional>
#include <thread>
#include <vector>
#include <iostream>
#include <sched.h>

#include <signal.h>
#include <unistd.h>
#include <cstring>
#include <chrono>

#include "../lib/Sequencer/include/Sequencer.hpp"
#include <atomic>

int _running = 1;  // Global variable to control service execution

std::jthread _service;  // Global thread for service execution

Sequencer sequencer{};
//int exit_flag = 0;
extern std::atomic<bool> exit_flag;



// Service method definitions:


// Sequencer method definitions:
Sequencer::Sequencer()
    : posix_timer(0) {
}

Sequencer::~Sequencer() {
    stopServices();
}

void Sequencer::addService(Service* service) {
    _services.emplace_back(service);  // Just wrap and take ownership
}

void Sequencer::startServices() {
    std::cout << "Starting services..." << std::endl;

    for (auto& service : _services) {
        std::thread([service = service.get()]() {
            service->run();
        }).detach();
    }

    // You could optionally create a timer here if you want periodic releases
}

void Sequencer::stopServices() {
    for (auto& service : _services) {
        service->stop();
    }
    std::cout << "Stopped all services." << std::endl;
}


Service::Service(uint8_t affinity, uint8_t priority, uint32_t period)
    : _affinity(affinity), _priority(priority), _period(period) {
    std::cout << "Service created with affinity: " << (int)_affinity << std::endl;
}

void Service::_initializeService() {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(_affinity, &cpuset);
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0) {
        std::cerr << "Error setting thread affinity" << std::endl;
    }

    struct sched_param sched;
    sched.sched_priority = _priority;
    if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &sched) != 0) {
        std::cerr << "Error setting thread priority" << std::endl;
    }
}

void Service::run() {
    _running = true;
    _provideService();
}

void Service::stop() {
    _running = false;
    release();  // Wake up service if waiting
}

void Service::release() {
    {
        std::lock_guard<std::mutex> lock(mtx);
        ready = true;
    }
    cv.notify_one();
}

void Service::_provideService() {
    _initializeService();

    while (_running) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]() { return ready.load() || !_running; });

        if (!_running) break;

        ready = false;
        _doService();
    }
}

uint32_t Service::get_period() const {
    return _period;
}

