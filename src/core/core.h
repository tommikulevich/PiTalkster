/**
 *******************************************************************************
 * @file    core.h
 * @brief   Core header file.
 *******************************************************************************
 */

#ifndef CORE_H
#define CORE_H

#ifdef __cplusplus
extern "C" {
#endif

/************
 * INCLUDES *
 ************/

#include "utils.h"

/******************************
 * GLOBAL FUNCTION PROTOTYPES *
 ******************************/

extern result_t core_init( void );
extern void * core_thread( void * arg );

#ifdef __cplusplus
}
#endif

#endif /* CORE_H */
