#ifndef __AUDIOOUTPUT_H_H
#define __AUDIOOUTPUT_H_H

// TODO (SY): Hardcoded for now, probably should have something to search for a device
#define PCM_PLAYBACK_DEVICE "default"

#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <syslog.h>

#include <DataStructures/DataStructures.hpp>

void playback_audio(AudioData audio, int rate, int channels);

#endif // __AUDIOOUTPUT_H_H
