#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/time.h>

// ===========
// = General =
// ===========

#define OUTPUT
#define UNUSED_PARAM __attribute__((unused))
#define NELEMS(x) (sizeof(x)/sizeof((x)[0]))
#define STRUCT_INIT_ALL_ZEROS {0}

typedef enum {
    RES_OK,
    RES_ERR_GENERIC,
    RES_ERR_NOT_READY,
    RES_ERR_INVALID_SIZE,
    RES_ERR_WRONG_ARGS,
    RES_ERR_NULL_PTR
} result_t;

#define RETURN_ON_ERROR(result) \
    do { \
        if ((result) != RES_OK) { \
            return (result); \
        } \
    } while (0)

#define RETURN_IF_NULL(ptr) \
    do { \
        if ((ptr) == NULL) { \
            return RES_ERR_NULL_PTR; \
        } \
    } while (0)

#define RETURN_ERROR_IF(condition, error) \
    do { \
        if ((condition)) { \
            return (error); \
        } \
    } while (0)

// ======
// = IO =
// ======

typedef enum {
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_DEBUG_
} log_level_t;

#ifndef CURRENT_LOG_LEVEL
#define CURRENT_LOG_LEVEL LOG_LEVEL_DEBUG_
#endif

#ifndef LOG_STREAM
#define LOG_STREAM stdout
#endif

#define RST     "\033[0m"
#define RED     "\033[31m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define GREEN   "\033[32m"
#define CYAN    "\033[36m"
#define MAGENTA "\033[35m"

#define LOG(level, is_endline, ...) \
    do { \
        if (level <= CURRENT_LOG_LEVEL) { \
            const char *level_str; \
            switch (level) { \
                case LOG_LEVEL_ERROR: level_str = RED"[ERROR]"RST; break; \
                case LOG_LEVEL_WARN:  level_str = YELLOW"[WARN]"RST;  break; \
                case LOG_LEVEL_INFO:  level_str = BLUE"[INFO]"RST;  break; \
                case LOG_LEVEL_DEBUG: level_str = "[DEBUG]"; break; \
                case LOG_LEVEL_DEBUG_: \
                default: level_str = ""; break; \
            } \
            fprintf(LOG_STREAM, "%s ", level_str); \
            fprintf(LOG_STREAM, __VA_ARGS__); \
            if( is_endline ) { fprintf(LOG_STREAM, "\n"); } \
        } \
    } while (0)

#define ERROR(...) LOG(LOG_LEVEL_ERROR, true, __VA_ARGS__)
#define WARN(...)  LOG(LOG_LEVEL_WARN, true, __VA_ARGS__)
#define INFO(...)  LOG(LOG_LEVEL_INFO, true, __VA_ARGS__)
#define DEBUG(...) LOG(LOG_LEVEL_DEBUG, true, __VA_ARGS__)
#define DEBUG_(...) LOG(LOG_LEVEL_DEBUG_, false, __VA_ARGS__)

// ===========
// = Asserts =
// ===========

#define SLEEP_TIME_ASSERT_FAILED_S  5

#define STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)

#define ASSERT(condition)                        \
    do {                                    \
        if (!(condition)) {                      \
            ERROR("Assertion failed: %s\n", #condition); \
            sleep(SLEEP_TIME_ASSERT_FAILED_S); \
            exit(EXIT_FAILURE);             \
        }                                   \
    } while (0)

#define ASSERT_NOT_NULL(ptr)  ASSERT((ptr) != NULL)

// ========
// = Time =
// ========

static inline uint64_t get_current_time_us( void ) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((uint64_t)tv.tv_sec) * 1000000ULL + (uint64_t)tv.tv_usec;
}

#endif
