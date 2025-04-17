#include <stdio.h>
#include "realtimeaudio/main.h"
#include "AudioOutput.h"
#include "AudioCapture.h"
#include "Filters.h"

int main()
{
    printf("Hello from main!\n");
    AudioOutput_function();
    AudioCapture_function();
    Filters_function();
    return 0;
}
