#include "Sequencer.hpp"
#include <pthread.h>
#include <sched.h>
#include <unistd.h>

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

