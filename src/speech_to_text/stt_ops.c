/**
 *******************************************************************************
 * @file    stt_ops.c
 * @brief   Speech-to-text operations source file. 
 *          Interaction with VOSK.
 *******************************************************************************
 */

/************
 * INCLUDES *
 ************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <vosk_api.h>
#include <cjson/cJSON.h>

#include "utils.h"

#include "stt_ops.h"

/******************************
 * PRIVATE MACROS AND DEFINES *
 ******************************/

#define DEFAULT_VOSK_ENG_MODEL      "models/vosk-model-small-en-us-0.15" // TODO: make it configurable
#define DEFAULT_VOSK_SAMPLE_RATE    16000.0
#define DEFAULT_VOSK_JSON_TEXT_KEY  "text"

#define READ_BUFFER_SIZE_BYTES      4096
#define WAV_HEADER_SIZE_BYTES       44

/********************
 * GLOBAL FUNCTIONS *
 ********************/

result_t perform_speech_to_text( const char * txt_filepath, const char * wav_filepath, 
        volatile int * stop_flag, volatile int * progress ) {
    RETURN_IF_NULL(txt_filepath);
    RETURN_IF_NULL(wav_filepath);
    RETURN_IF_NULL(stop_flag);
    RETURN_IF_NULL(progress);

    vosk_set_log_level(-1);

    VoskModel * model = vosk_model_new(DEFAULT_VOSK_ENG_MODEL);
    if( !model ) {
        return RES_ERR_GENERIC;
    }

    VoskRecognizer * recognizer = vosk_recognizer_new(model, 
        DEFAULT_VOSK_SAMPLE_RATE);
    if( !recognizer ) {
        vosk_model_free(model);
        return RES_ERR_GENERIC;
    }

    FILE * wav_file = fopen(wav_filepath, "rb");
    if( !wav_file ) {
        vosk_recognizer_free(recognizer);
        vosk_model_free(model);
        return RES_ERR_GENERIC;
    }

    fseek(wav_file, 0, SEEK_END);
    long file_size = ftell(wav_file);
    fseek(wav_file, WAV_HEADER_SIZE_BYTES, SEEK_SET);

    FILE * txt_file = fopen(txt_filepath, "w");
    if( !txt_file ) {
        fclose(wav_file);
        vosk_recognizer_free(recognizer);
        vosk_model_free(model);
        return RES_ERR_GENERIC;
    }

    int read_bytes = 0;
    int total_bytes_read = 0;
    char buffer[READ_BUFFER_SIZE_BYTES];
    while( (read_bytes = (int)fread(buffer, sizeof(char), READ_BUFFER_SIZE_BYTES, wav_file)) > 0 ) {
        if( *stop_flag ) {
            break;
        }

        total_bytes_read += read_bytes;
        *progress = (int)((total_bytes_read * 100) / file_size);

        if( vosk_recognizer_accept_waveform(recognizer, buffer, read_bytes) ) {
            const char * result_json = vosk_recognizer_result(recognizer);
            cJSON * json = cJSON_Parse(result_json);
            if( json ) {
                cJSON *text = cJSON_GetObjectItemCaseSensitive(json, 
                    DEFAULT_VOSK_JSON_TEXT_KEY);
                if( cJSON_IsString(text) && (text->valuestring != NULL) ) {
                    fprintf(txt_file, "%s\n", text->valuestring);
                }
                cJSON_Delete(json); 
            }
        }
    }

    const char * final_result_json = vosk_recognizer_final_result(recognizer);
    cJSON * final_json = cJSON_Parse(final_result_json);
    if( final_json ) {
        cJSON * final_text = cJSON_GetObjectItemCaseSensitive(final_json, 
            DEFAULT_VOSK_JSON_TEXT_KEY);
        if( cJSON_IsString(final_text) && (final_text->valuestring != NULL) ) {
            fprintf(txt_file, "%s\n", final_text->valuestring);
        }
        cJSON_Delete(final_json);
    }

    fclose(wav_file);
    fclose(txt_file);
    vosk_recognizer_free(recognizer);
    vosk_model_free(model);

    return RES_OK;
}
