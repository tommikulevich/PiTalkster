/**
 *******************************************************************************
 * @file    ollama_api_ops.c
 * @brief   Ollama API operations source file.
 *          Interaction (curl) with DeepSeek using Ollama API.
 *******************************************************************************
 */

/************
 * INCLUDES *
 ************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

#include "utils.h"

#include "ollama_api_ops.h"

/******************************
 * PRIVATE MACROS AND DEFINES *
 ******************************/

#define READ_BUFFER_SIZE_BYTES      4096
#define DEFAULT_DEEPSEEK_MODEL      "deepseek-r1:1.5b"
#define DEFAULT_OLLAMA_URL          "http://localhost:11434/api/generate"

/********************
 * PRIVATE TYPEDEFS *
 ********************/

typedef struct {
    char * data;
    size_t size;
    volatile int * stop_flag;
    void * user_data;

    response_callback_t callback;
} ollama_response_data_t;

/********************
 * STATIC FUNCTIONS *
 ********************/

static size_t ollama_write_callback( char * ptr, size_t size, size_t nmemb, 
        void * user_data ) {
    size_t total_size = size * nmemb;
    ollama_response_data_t *resp = (ollama_response_data_t *)user_data;
    if( *(resp->stop_flag) ) {
        return 0;
    }

    char * new_data = realloc(resp->data, resp->size + total_size + 1);
    if( new_data == NULL ) {
        return 0;
    }

    resp->data = new_data;
    memcpy(&(resp->data[resp->size]), ptr, total_size);
    resp->size += total_size;
    resp->data[resp->size] = '\0';

    char * line_start = resp->data;
    char * newline_pos = NULL;
    while( (newline_pos = strchr(line_start, '\n')) != NULL ) {
        *newline_pos = '\0';
        cJSON * json = cJSON_Parse(line_start);
        if( json ) {
            cJSON * response = cJSON_GetObjectItemCaseSensitive(json, "response");
            if( cJSON_IsString(response) && (response->valuestring != NULL) ) {
                resp->callback(response->valuestring, strlen(response->valuestring), 
                    resp->user_data);
            }
            cJSON_Delete(json);
        }
        line_start = newline_pos + 1;
    }

    size_t remaining_size = resp->size - (size_t)(line_start - resp->data);
    memmove(resp->data, line_start, remaining_size);
    resp->size = remaining_size;
    resp->data[resp->size] = '\0';

    return total_size;
}

/********************
 * GLOBAL FUNCTIONS *
 ********************/

result_t ollama_ask_deepseek_model( 
        const char * answer_filepath, const char * prompt_filepath, 
        volatile int * stop_flag, response_callback_t callback ) {
    RETURN_IF_NULL(answer_filepath);
    RETURN_IF_NULL(prompt_filepath);
    RETURN_IF_NULL(stop_flag);

    char * prompt = NULL;
    long prompt_length = 0;
    FILE * prompt_file = fopen(prompt_filepath, "rb");
    if( !prompt_file ) {
        return RES_ERR_GENERIC;
    }
    fseek(prompt_file, 0, SEEK_END);
    prompt_length = ftell(prompt_file);
    fseek(prompt_file, 0, SEEK_SET);
    prompt = malloc((size_t)prompt_length + 1);
    if( !prompt ) {
        fclose(prompt_file);
        return RES_ERR_GENERIC;
    }
    size_t read_size = fread(prompt, 1, (size_t)prompt_length, prompt_file);
    prompt[read_size] = '\0';
    fclose(prompt_file);

    cJSON * root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "model", DEFAULT_DEEPSEEK_MODEL);
    cJSON_AddStringToObject(root, "prompt", prompt);
    char * post_data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    free(prompt);

    CURL * curl = curl_easy_init();
    if( !curl ) {
        free(post_data);
        return RES_ERR_GENERIC;
    }

    FILE * output_file = fopen(answer_filepath, "a+");
    if( !output_file ) {
        free(post_data);
        curl_easy_cleanup(curl);
        return RES_ERR_GENERIC;
    }
    
    ollama_response_data_t resp;
    memset(&resp, 0, sizeof(ollama_response_data_t));
    resp.callback = callback;
    resp.user_data = output_file;
    resp.stop_flag = stop_flag;
    curl_easy_setopt(curl, CURLOPT_URL, DEFAULT_OLLAMA_URL);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ollama_write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&resp);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);
    if( res != CURLE_OK ) {
        fclose(output_file);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        free(post_data);
        free(resp.data);
        return RES_ERR_GENERIC;
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    free(post_data);
    free(resp.data);
    fclose(output_file);

    return RES_OK;
}
