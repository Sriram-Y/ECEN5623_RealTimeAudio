#ifndef __AUDIOOUTPUT_H_H
#define __AUDIOOUTPUT_H_H


#ifdef __cplusplus
extern "C" {
#endif

// TODO (SY): Hardcoded for now, probably should have something to search for a device
#define PCM_PLAYBACK_DEVICE "default"

#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <syslog.h>

#include <DataStructures/DataStructures.h>

void playback_audio(AudioData audio, int rate, int channels);


#ifdef __cplusplus
}
#endif

#endif // __AUDIOOUTPUT_H_H

