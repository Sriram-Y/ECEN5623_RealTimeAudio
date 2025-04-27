#include "AudioCapture.hpp"
#include <stdio.h>
#include <iostream>

int setup_capture(AudioCapture *cap, int rate, int channels)
{
    cap->rate = rate;
    cap->channels = channels;

    if (snd_pcm_open(&cap->pcm_handle, PCM_DEVICE, SND_PCM_STREAM_CAPTURE, 0) < 0)
    {
        syslog(LOG_ERR, "!!! ERROR: Can't open PCM device. !!!\n");
        return -1;
    }

    snd_pcm_hw_params_malloc(&cap->params);
    snd_pcm_hw_params_any(cap->pcm_handle, cap->params);
    snd_pcm_hw_params_set_access(cap->pcm_handle, cap->params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(cap->pcm_handle, cap->params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(cap->pcm_handle, cap->params, channels);
    snd_pcm_hw_params_set_rate(cap->pcm_handle, cap->params, rate, 0);

    if (snd_pcm_hw_params(cap->pcm_handle, cap->params) < 0)
    {
        syslog(LOG_ERR, "!!! ERROR: Can't set hardware parameters. !!!\n");
        return -1;
    }

    snd_pcm_hw_params_get_period_size(cap->params, &cap->frames, 0);
    cap->frame_buffer_size = cap->frames * channels * 2; // 2 bytes/sample
    cap->frame_buffer = (char *)malloc(cap->frame_buffer_size);

    if (!cap->frame_buffer)
    {
        syslog(LOG_ERR, "!!! ERROR: Buffer allocation failed. !!!\n");
        return -1;
    }

    return 0;
}

AudioData start_capture(AudioCapture *cap, int seconds)
{
    int loops = (cap->rate / cap->frames) * seconds;

    size_t total_size = loops * cap->frame_buffer_size;
    char *buffer = (char *)malloc(total_size);
    size_t offset = 0;

    AudioData result;

    syslog(LOG_INFO, "### Recording for %d seconds... ###\n", seconds);

    for (int i = 0; i < loops; i++)
    {
        int pcm = snd_pcm_readi(cap->pcm_handle, cap->frame_buffer, cap->frames);
        if (pcm == -EPIPE)
        {
            syslog(LOG_ERR, "!!! Overrun occurred !!!\n");
            snd_pcm_prepare(cap->pcm_handle);
        }
        else if (pcm < 0)
        {
            syslog(LOG_ERR, "!!! ERROR: Read error %s !!!\n", snd_strerror(pcm));
        }

        if (offset + cap->frame_buffer_size <= total_size)
        {
            memcpy(buffer + offset, cap->frame_buffer, cap->frame_buffer_size);
            offset += cap->frame_buffer_size;
        }
    }

    snd_pcm_drain(cap->pcm_handle);
    snd_pcm_close(cap->pcm_handle);
    snd_pcm_hw_params_free(cap->params);
    free(cap->frame_buffer);

    // TODO (SY): Could be handled better
    result.data = buffer;
    result.size = total_size;
    result.offset = offset;

    return result;
}

void stop_capture(AudioCapture *cap)
{
    std::cout<<"A\n";
    snd_pcm_drain(cap->pcm_handle);
    std::cout<<"B\n";
    snd_pcm_close(cap->pcm_handle);
    std::cout<<"C\n";
    snd_pcm_hw_params_free(cap->params);
    free(cap->frame_buffer);
}

