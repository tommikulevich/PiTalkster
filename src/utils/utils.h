#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>

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
    RES_ERR_INVALID_SIZE
} result_t;

#define RETURN_ON_ERROR(result) \
    do { \
        if ((result) != RES_OK) { \
            return (result); \
        } \
    } while (0)

// ======
// = IO =
// ======

typedef enum {
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG
} log_level_t;

#ifndef CURRENT_LOG_LEVEL
#define CURRENT_LOG_LEVEL LOG_LEVEL_INFO
#endif

#ifndef LOG_STREAM
#define LOG_STREAM stdout
#endif

#define LOG(level, ...) \
    do { \
        if (level <= CURRENT_LOG_LEVEL) { \
            const char *level_str; \
            switch (level) { \
                case LOG_LEVEL_ERROR: level_str = "[ERROR]"; break; \
                case LOG_LEVEL_WARN:  level_str = "[WARN]";  break; \
                case LOG_LEVEL_INFO:  level_str = "[INFO]";  break; \
                case LOG_LEVEL_DEBUG: level_str = "[DEBUG]"; break; \
                default: level_str = ""; break; \
            } \
            fprintf(LOG_STREAM, "%s ", level_str); \
            fprintf(LOG_STREAM, __VA_ARGS__); \
            fprintf(LOG_STREAM, "\n"); \
        } \
    } while (0)

#define ERROR(...) LOG(LOG_LEVEL_ERROR, __VA_ARGS__)
#define WARN(...)  LOG(LOG_LEVEL_WARN, __VA_ARGS__)
#define INFO(...)  LOG(LOG_LEVEL_INFO, __VA_ARGS__)
#define DEBUG(...) LOG(LOG_LEVEL_DEBUG, __VA_ARGS__)

// ===========
// = Asserts =
// ===========

#define STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)

#define ASSERT(condition)                        \
    do {                                    \
        if (!(condition)) {                      \
            ERROR("Assertion failed: %s\n", #condition); \
            exit(EXIT_FAILURE);             \
        }                                   \
    } while (0)

#define ASSERT_NOT_NULL(ptr)  ASSERT((ptr) != NULL)

#endif
