
#ifndef __AUDIOCAPTURE_H_H
#define __AUDIOCAPTURE_H_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <syslog.h>

#include <DataStructures/DataStructures.h>

// TODO (SY): Hardcoded for now, probably should have something to search for a device (card#, device#)
#define PCM_DEVICE "hw:0,0"

int setup_capture(AudioCapture *cap, int rate, int channels);
AudioData start_capture(AudioCapture *cap, int seconds);
void stop_capture(AudioCapture *cap);

#ifdef __cplusplus
}
#endif

#endif // __AUDIOCAPTURE_H_H

