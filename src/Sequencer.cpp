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
#include <iostream>
#include "Sequencer.hpp"

int abortTest = 0;
unsigned int fib10Cnt = 0, fib20Cnt = 0;

#define FIB_LIMIT_FOR_32_BIT 47

/* Macro for Fibonacci computation to simulate CPU load */
#define FIB_TEST(seqCnt, iterCnt)                           \
    do                                                      \
    {                                                       \
        unsigned int fib0 = 0, fib1 = 1, fib, idx, jdx = 1; \
        for (idx = 0; idx < iterCnt; idx++)                 \
        {                                                   \
            fib = fib0 + fib1;                              \
            while (jdx < seqCnt)                            \
            {                                               \
                fib0 = fib1;                                \
                fib1 = fib;                                 \
                fib = fib0 + fib1;                          \
                jdx++;                                      \
            }                                               \
        }                                                   \
    } while (0)


void service2()
{
    std::puts("this is service 2 implemented as a function\n");
}

//int main()
//{
//    Sequencer sequencer{};
//
//    // Schedule fib10 to run every 20ms and fib20 every 50ms.
//    sequencer.addService(fib10, 0, 97, 20);
//    sequencer.addService(fib20, 0, 96, 50);
//
//    std::cout << "Starting services..." << std::endl;
//    sequencer.startServices();
//
//    // todo (done): wait for ctrl-c or some other terminating condition
//    // implemented here as another thread that waits for ENTER key to be pressed
//    std::thread waitForEnterThread([]()
//    {
//        std::string input;
//        std::cout << "Press ENTER to stop services..." << std::endl;
//        std::getline(std::cin, input);
//    });
//    waitForEnterThread.join();
//
//    sequencer.stopServices();
//    std::cout << "Services stopped." << std::endl;
//    return 0;
//}
