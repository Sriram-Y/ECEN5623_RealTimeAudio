#include "AudioOutput.h"

void playback_audio(AudioData audio, int rate, int channels)
{
    snd_pcm_t *playback_handle;
    snd_pcm_hw_params_t *params;
    snd_pcm_uframes_t frames;

    if (snd_pcm_open(&playback_handle, PCM_PLAYBACK_DEVICE, SND_PCM_STREAM_PLAYBACK, 0) < 0)
    {
        syslog(LOG_ERR, "!!! ERROR: Can't open PCM device for playback. !!!\n");
        return;
    }

    snd_pcm_hw_params_malloc(&params);
    snd_pcm_hw_params_any(playback_handle, params);
    snd_pcm_hw_params_set_access(playback_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(playback_handle, params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(playback_handle, params, channels);
    snd_pcm_hw_params_set_rate(playback_handle, params, rate, 0);

    if (snd_pcm_hw_params(playback_handle, params) < 0)
    {
        syslog(LOG_ERR, "!!! ERROR: Can't set hardware parameters for playback. !!!\n");
        return;
    }

    snd_pcm_hw_params_get_period_size(params, &frames, 0);
    int buffer_size = frames * channels * 2;

    syslog(LOG_INFO, "### Playing back audio... ###\n");
    for (size_t i = 0; i < audio.size; i += buffer_size)
    {
        size_t to_write;

        if (i + buffer_size <= audio.size)
        {
            to_write = buffer_size;
        }
        else
        {
            to_write = audio.size - i;
        }

        int pcm = snd_pcm_writei(playback_handle, audio.data + i, to_write / (channels * 2));
        if (pcm < 0)
        {
            snd_pcm_prepare(playback_handle);
            syslog(LOG_ERR, "!!! ERROR: Playback write failed: %s !!!\n", snd_strerror(pcm));
        }
    }

    snd_pcm_drain(playback_handle);
    snd_pcm_close(playback_handle);
    snd_pcm_hw_params_free(params);

    syslog(LOG_INFO, "### Playback finished. ###\n");
}
