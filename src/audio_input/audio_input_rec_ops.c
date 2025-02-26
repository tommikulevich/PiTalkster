#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <alsa/asoundlib.h>

#include "utils.h"

#include "audio_input_rec_ops.h"

#define WAV_HEADER_SIZE_BYTES   44

typedef struct {
    const char * device_name;
    unsigned int rate;
    unsigned int channels;
    snd_pcm_format_t format;
    snd_pcm_uframes_t period_size;
    snd_pcm_uframes_t buffer_size;
    uint16_t bits_per_sample;
} audio_settings_t;

static result_t setup_pcm_capture( snd_pcm_t ** handle, 
        snd_pcm_hw_params_t ** params, audio_settings_t * settings ) {
    ASSERT_NOT_NULL(handle);
    ASSERT_NOT_NULL(params);
    ASSERT_NOT_NULL(settings);

    int rc;
    snd_pcm_t * capture_handle = NULL;
    snd_pcm_hw_params_t * hw_params = NULL;

    rc = snd_pcm_open(&capture_handle, settings->device_name, 
        SND_PCM_STREAM_CAPTURE, 0);
    if( rc < 0 ) return RES_ERR_GENERIC;

    rc = snd_pcm_hw_params_malloc(&hw_params);
    if( rc < 0 ) {
        snd_pcm_close(capture_handle);
        return RES_ERR_GENERIC;
    }

    rc = snd_pcm_hw_params_any(capture_handle, hw_params);
    if( rc < 0 ) {
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(capture_handle);
        return RES_ERR_GENERIC;
    }

    rc = snd_pcm_hw_params_set_access(capture_handle, hw_params, 
        SND_PCM_ACCESS_RW_INTERLEAVED);
    if( rc < 0 ) {
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(capture_handle);
        return RES_ERR_GENERIC;
    }

    rc = snd_pcm_hw_params_set_format(capture_handle, hw_params, 
        settings->format);
    if( rc < 0 ) {
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(capture_handle);
        return RES_ERR_GENERIC;
    }

    rc = snd_pcm_hw_params_set_channels(capture_handle, hw_params, 
        settings->channels);
    if( rc < 0 ) {
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(capture_handle);
        return RES_ERR_GENERIC;
    }

    rc = snd_pcm_hw_params_set_rate_near(capture_handle, hw_params, 
        &(settings->rate), NULL);
    if( rc < 0 ) {
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(capture_handle);
        return RES_ERR_GENERIC;
    }

    rc = snd_pcm_hw_params_set_period_size_near(capture_handle, hw_params, 
        &(settings->period_size), NULL);
    if( rc < 0 ) {
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(capture_handle);
        return RES_ERR_GENERIC;
    }

    rc = snd_pcm_hw_params_set_buffer_size_near(capture_handle, hw_params, 
        &(settings->buffer_size));
    if( rc < 0 ) {
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(capture_handle);
        return RES_ERR_GENERIC;
    }

    rc = snd_pcm_hw_params(capture_handle, hw_params);
    if( rc < 0 ) {
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(capture_handle);
        return RES_ERR_GENERIC;
    }

    *handle = capture_handle;
    *params = hw_params;

    return RES_OK;
}

static void write_little_endian_16( FILE * fp, uint16_t value ) {
    ASSERT_NOT_NULL(fp);

    uint8_t data[2];
    for (int i = 0; i < 2; i++) {
        data[i] = (uint8_t)(value & 0xFF);
        value >>= 8;
    }
    fwrite(data, 1, 2, fp);
}

static void write_little_endian_32( FILE * fp, uint32_t value ) {
    ASSERT_NOT_NULL(fp);

    uint8_t data[4];
    for (int i = 0; i < 4; i++) {
        data[i] = (uint8_t)(value & 0xFF);
        value >>= 8;
    }
    fwrite(data, 1, 4, fp);
}

static void write_wav_header( FILE * fp, uint32_t data_chunk_size, 
        const audio_settings_t * settings ) {
    ASSERT_NOT_NULL(fp);
    ASSERT_NOT_NULL(settings);

    uint32_t riff_chunk_size = data_chunk_size + (WAV_HEADER_SIZE_BYTES - 8);

    rewind(fp);
    fwrite("RIFF", 1, 4, fp);
    write_little_endian_32(fp, riff_chunk_size);
    fwrite("WAVE", 1, 4, fp);
    fwrite("fmt ", 1, 4, fp);
    write_little_endian_32(fp, 16);
    write_little_endian_16(fp, 1);
    write_little_endian_16(fp, (uint16_t)settings->channels); 
    write_little_endian_32(fp, settings->rate);  
    write_little_endian_32(fp, settings->rate * settings->channels 
        * settings->bits_per_sample / 8);  
    write_little_endian_16(fp, (uint16_t)(settings->channels 
        * settings->bits_per_sample / 8)); 
    write_little_endian_16(fp, settings->bits_per_sample);
    fwrite("data", 1, 4, fp);
    write_little_endian_32(fp, data_chunk_size);
}

result_t record_audio( const char * filepath OUTPUT, int duration_s, 
        volatile int * stop_flag ) {
    ASSERT_NOT_NULL(stop_flag);
    if( !filepath || duration_s <= 0 || !stop_flag ) {
        return RES_ERR_WRONG_ARGS;
    }

    audio_settings_t settings = {
        .device_name = "plughw:1",
        .rate = 48000,
        .channels = 2,
        .format = SND_PCM_FORMAT_S32_LE,
        .period_size = 6000,
        .buffer_size = 24000,
        .bits_per_sample = 32
    };

    snd_pcm_t * capture_handle = NULL;
    snd_pcm_hw_params_t * hw_params = NULL;
    char * buffer = NULL;
    FILE * fp = NULL;

    if( setup_pcm_capture(&capture_handle, &hw_params, &settings) < 0 ) {
        return RES_ERR_GENERIC;
    }

    size_t bytes_per_frame = (size_t)settings.channels *
                             (size_t)snd_pcm_format_physical_width(settings.format) / 8;
    snd_pcm_uframes_t buffer_frames = settings.period_size;
    size_t buffer_bytes = buffer_frames * bytes_per_frame;

    buffer = malloc(buffer_bytes);
    if( !buffer ) {
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(capture_handle);
        return RES_ERR_GENERIC;
    }

    fp = fopen(filepath, "wb");
    if( !fp ) {
        free(buffer);
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(capture_handle);
        return RES_ERR_GENERIC;
    }

    if( fseek(fp, WAV_HEADER_SIZE_BYTES, SEEK_SET) != 0 ) {
        fclose(fp);
        free(buffer);
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(capture_handle);
        return RES_ERR_GENERIC;
    }

    snd_pcm_uframes_t frames_recorded = 0;
    snd_pcm_uframes_t total_frames = (snd_pcm_uframes_t)duration_s * settings.rate;

    while( frames_recorded < total_frames && !(*stop_flag) ) {
        snd_pcm_uframes_t frames_to_read = buffer_frames;
        if( frames_recorded + frames_to_read > total_frames ) {
            frames_to_read = total_frames - frames_recorded;
        }

        snd_pcm_sframes_t rc = snd_pcm_readi(capture_handle, buffer, frames_to_read);
        if( rc == -EPIPE ) {
            snd_pcm_prepare(capture_handle);
            continue;
        } else if( rc < 0 ) {
            fclose(fp);
            free(buffer);
            snd_pcm_hw_params_free(hw_params);
            snd_pcm_close(capture_handle);
            return RES_ERR_GENERIC;
        } else if( rc > 0 ) {
            size_t bytes_to_write = (size_t)rc * bytes_per_frame;
            if( fwrite(buffer, 1, bytes_to_write, fp) != bytes_to_write ) {
                fclose(fp);
                free(buffer);
                snd_pcm_hw_params_free(hw_params);
                snd_pcm_close(capture_handle);
                return RES_ERR_GENERIC;
            }
            frames_recorded += (snd_pcm_uframes_t)rc;
        }
    }

    free(buffer);
    snd_pcm_drain(capture_handle);
    snd_pcm_close(capture_handle);
    snd_pcm_hw_params_free(hw_params);

    long file_size = ftell(fp);
    if( file_size < WAV_HEADER_SIZE_BYTES ) {
        fclose(fp);
        return RES_ERR_GENERIC;
    }

    uint32_t data_chunk_size = (uint32_t)(file_size - WAV_HEADER_SIZE_BYTES);
    write_wav_header(fp, data_chunk_size, &settings);
    fclose(fp);

    return RES_OK;
}
