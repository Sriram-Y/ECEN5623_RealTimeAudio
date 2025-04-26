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

#include "Sequencer.hpp"
#include <atomic>

int _running = 1;  // Global variable to control service execution

std::jthread _service;  // Global thread for service execution

Sequencer sequencer{};
int exit_flag = 0;

// Service method definitions:

void Service::_initializeService(){

    // todo: set affinity, priority, sched policy
    // (heads up: the thread is already running and we're in its context right now)
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

void Service::stop(){
    // todo: change state to "not running" using an atomic variable
    // (heads up: what if the service is waiting on the semaphore when this happens?)
    _running = false;  // Mark the service as stopped
    this->release();  // Release semaphore if waiting
}

void Service::release(){

    // todo: release the service using the semaphore
    _semaphore.release();  // Allow service to run
}

void Service::_provideService(){
    _initializeService();

    // todo: call _doService() on releases (sem acquire) while the atomic running variable is true
    while (_running) {
        // _semaphore.acquire();  // Wait for release signal

        if(_semaphore.try_acquire()){  // Try to acquire the semaphore
            if (!_running) {
                std::cout<<"Service breaked\n\n";
                break;
            }
            _doService();
            std::cout<<"Service done\n";
        } 
        else {
            if (!_running) {
                std::cout<<"Service breaked\n\n";
                break;
            }
        }
    }
}

// Sequencer method definitions:

// void Sequencer::addService(std::function<void(void)> service, uint8_t affinity, uint8_t priority, uint32_t period)
// {
//     // Create a new service and add it to the list of services
//     _services.emplace_back(service, affinity, priority, period);
// }

// void Sequencer::addService(Args&&... args)
// {
//     // Add the new service to the services list,
//     // constructing it in-place with the given args
//     _services.emplace_back(std::forward<Args>(args)...);
// }


// Timer service that will launch based on set interval timer
void Sequencer::timer_irq_service(union sigval sv){
    
    static int count = 0;

    // auto *seq = static_cast<Sequencer *>(sv.sival_ptr);
        //  if (!seq->_seq_running)
        //      return;
    if(exit_flag > 0) {
        std::cout<<"Timer service stopped\n";
        return;
    }

    count++;
    
    for (auto& service : sequencer.getServices()){
    // for(auto& service : sequencer._services) {
    // for(auto it = std::begin(sequencer); it != std::end(sequencer); ++it) {
    //     auto& service = *it;
        // Check if the service is running and if the period has elapsed
        if (count % service->get_period() == 0) {
            service->release();  // Trigger service execution immediately (you can add periodic logic here)
        }
    }
    // auto *seq = static_cast<Sequencer *>(sv.sival_ptr);

    // std::cout<<"Got timer irq. \n";
}

void Sequencer::startServices(){
    _running = 1;  // Set the running flag to true
    // todo: start timer(s), release services
    

    // Set up the timer to call the timer_irq_service function
    struct sigevent sev{};
    sev.sigev_notify = SIGEV_THREAD;//run this in a seperate thread
    sev.sigev_value.sival_ptr = this; //store pointer of current object instance to pass to handler
    sev.sigev_notify_function = timer_irq_service;

    if (timer_create(CLOCK_MONOTONIC, &sev, &posix_timer) == -1)
    {
        perror("timer_create");
        return;
    }

  // Configure the timer to expire after 1 msec and then every 1 msec
    struct itimerspec its{};
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = 1000000; 
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 1000000;

    if (timer_settime(posix_timer, 0, &its, nullptr) == -1)
    {
        perror("timer_settime");
        return;
    }
}

void Sequencer::stopServices(){
    _running = 0;  // Set the running flag to false
    // todo: stop timer(s), stop services
    for (auto& service : _services) {
        service->stop();  // Stop the service
    }

    std::cout<<"Stop services\n";
    exit_flag = 2;
}

void handle_sigint(int sig) {
    printf("\nCaught signal %d (Ctrl+C). Exiting...\n", sig);
    sequencer.stopServices();
    exit_flag = 1; // Set exit flag to indicate termination
}

void service2(){
    std::puts("this is service 2 implemented as a function 22222222222222222\n");
}

int main(){

    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sa.sa_flags = 0; // No special flags
    sigemptyset(&sa.sa_mask); // No blocked signals

    // Example use of the sequencer/service classes:
    sequencer.addService([]() {
        std::puts("this is service 1 implemented in a lambda expression\n");
    }, 1, 99, 5000);
    
    sequencer.addService(service2, 1, 98, 1000);
    
    // Register signal handler
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL); // Handle termination signal
    
    std::cout<<"Services added\n";

    sequencer.startServices();
    // todo: wait for ctrl-c or some other terminating condition
    while (exit_flag == 0) {
        sleep(1); // Sleep to reduce CPU usage
    }

    while(exit_flag <= 1) {
        sleep(1); // Sleep to reduce CPU usage
    }
    _service.join();  // Wait for the service thread to finish
    return 0;
}