#include <stdio.h>
#include <signal.h>
#include "AudioOutput.hpp"
#include "AudioCapture.hpp"
#include "Filters.hpp"
#include "Sequencer.hpp"

AudioCapture cap;
//const int rate = 44100;
const int rate = 18100;
const int channels = 1;
const int seconds = 1;
AudioData cap_data ;
extern Sequencer sequencer;
Sequencer sequencer{};

std::jthread _service;  // Global thread for service execution

void serviceCapture(){
    std::puts("Playback\n");
    //cap_data = start_capture(&cap, seconds);
}

void servicePlayback(){
    std::puts("Playback\n");
    //playback_audio(cap_data, rate, channels);
}

int main(){

    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sa.sa_flags = 0; // No special flags
    sigemptyset(&sa.sa_mask); // No blocked signals

    std::cout<<"Try to setup capture\n";
    if (setup_capture(&cap, rate, channels) != 0)
    {
        std::cout<<"Failed to setup capture\n";
        return 1;
    }
    // Example use of the sequencer/service classes:
    //sequencer.addService([]() {
        //std::puts("this is service 1 implemented in a lambda expression\n");
    //}, 1, 99, 5000);
    //std::cout<<"First pass recording\n"; 
    //cap_data = start_capture(&cap, seconds);
    //for(int i = 0; i<5; i++){
    //  //std::cout<<"Sample a bit\n";
    //  //serviceCapture();
    //  std::cout<<"Playback a bit\n";
    //  playback_audio(cap_data, rate, channels);
    //  //std::cout<<"Stop Capture\n";
    //  //stop_capture(&cap);
    //  //free(cap_data.data);
    //  //free(cap_data.data);
    //  //free(cap_data.size);
    //  //free(cap_data.offset);
    //}
    std::cout<<"Try to add services\n";
    sequencer.addService(serviceCapture, 1, 98, 5000);
    sequencer.addService(servicePlayback, 1, 99, 10000);
    
    // Register signal handler
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL); // Handle termination signal
    
    std::cout<<"Services added\n";

    //sequencer.startServices();
    // todo: wait for ctrl-c or some other terminating condition
    //while (sequencer.exit_flag == 0) {
    //    sleep(1); // Sleep to reduce CPU usage
    //}

    //while(sequencer.exit_flag <= 1) {
    //    sleep(1); // Sleep to reduce CPU usage
    //}
    _service.join();  // Wait for the service thread to finish
    free(cap_data.data);
    return 0;
}
