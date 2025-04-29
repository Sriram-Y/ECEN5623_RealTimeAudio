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
const int seconds = 5;
AudioData cap_data ;
AudioData cap_data2 ;
extern Sequencer sequencer;

std::jthread _service;  // Global thread for service execution

void serviceCapture(){
    std::puts("Capture\n");
    //cap_data = start_capture(&cap, seconds);
}

void servicePlayback(){
    std::puts("Playback\n");
    //playback_audio(cap_data, rate, channels);
}

int main(){
    Sequencer sequencer{};

    //struct sigaction sa;
    //sa.sa_handler = handle_sigint;
    //sa.sa_flags = 0; // No special flags
    //sigemptyset(&sa.sa_mask); // No blocked signals

    std::cout<<"Try to setup capture\n";
    if (setup_capture(&cap, rate, channels) != 0)
    {
        std::cout<<"Failed to setup capture\n";
        return 1;
    }
    std::cout<<"Success of setup capture\n";
    // Example use of the sequencer/service classes:
    //sequencer.addService([]() {
        //std::puts("this is service 1 implemented in a lambda expression\n");
    //}, 1, 99, 5000);
    //std::cout<<"First pass recording\n"; 
    //cap_data = start_capture(&cap, seconds);


    std::cout<<"Test Audio Service\n";
    for(int i = 0; i<5; i++){
      std::cout<<"Sample a bit\n";
      cap_data = start_capture(&cap, seconds);
      std::cout<<"Playback a bit\n";
      playback_audio(cap_data, rate, channels);
      std::cout<<"Sample a bit again\n";
      cap_data2 = start_capture(&cap, seconds);
      std::cout<<"Playback a bit\n";
      playback_audio(cap_data, rate, channels);
      //std::cout<<"Stop Capture\n";
      //stop_capture(&cap);
      //free(cap_data.data);
      //free(cap_data.data);
      //free(cap_data.size);
      //free(cap_data.offset);
    }
  

    //std::cout<<"Try to add services\n";
    //sequencer.addService(serviceCapture, 1, 97, 5000);
    //sequencer.addService(servicePlayback, 1, 96, 1000);
    
    // Register signal handler
    //sigaction(SIGINT, &sa, NULL);
    //sigaction(SIGTERM, &sa, NULL); // Handle termination signal
    
    std::cout<<"Services added\n";

    sequencer.startServices();
    // todo: wait for ctrl-c or some other terminating condition
    //while (sequencer.exit_flag == 0) {
    //    sleep(1); // Sleep to reduce CPU usage
    //}

    //while(sequencer.exit_flag <= 1) {
    //    sleep(1); // Sleep to reduce CPU usage
    //}
    std::thread waitForEnterThread([]()
    {
        std::string input;
        std::cout << "Press ENTER to stop services..." << std::endl;
        std::getline(std::cin, input);
    });
    waitForEnterThread.join();

    sequencer.stopServices();
    std::cout << "Services stopped." << std::endl;
}
