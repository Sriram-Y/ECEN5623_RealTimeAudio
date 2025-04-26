#include <stdio.h>
#include "AudioOutput.hpp"
#include "AudioCapture.hpp"
#include "Filters.hpp"
#include "Sequencer.hpp"

AudioCapture cap;
const int rate = 44100;
const int channels = 1;
const int seconds = 1;
AudioData cap_data ;

void serviceCapture(){
    cap_data = start_capture(&cap, seconds);
}

void servicePlayback(){
    playback_audio(cap_data, rate, channels);
}

int main(){

    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sa.sa_flags = 0; // No special flags
    sigemptyset(&sa.sa_mask); // No blocked signals

    if (setup_capture(&cap, rate, channels) != 0)
    {
        return 1;
    }
    // Example use of the sequencer/service classes:
    //sequencer.addService([]() {
        //std::puts("this is service 1 implemented in a lambda expression\n");
    //}, 1, 99, 5000);
  
    sequencer.addService(serviceCapture, 1, 98, 5);
    sequencer.addService(servicePlayback, 1, 99, 5);
    
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
    free(cap_data.data);
    return 0;
}
