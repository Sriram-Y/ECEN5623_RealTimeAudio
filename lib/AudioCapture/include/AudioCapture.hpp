#ifndef __AUDIOCAPTURE_H_H
#define __AUDIOCAPTURE_H_H

#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <syslog.h>

#include <DataStructures/DataStructures.hpp>

// TODO (SY): Hardcoded for now, probably should have something to search for a device (card#, device#)
#define PCM_DEVICE "hw:1,0"

int setup_capture(AudioCapture *cap, int rate, int channels);
AudioData start_capture(AudioCapture *cap, int seconds);
void stop_capture(AudioCapture *cap);

#endif // __AUDIOCAPTURE_H_H
