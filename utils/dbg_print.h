/*
 * dbg_print.h
 *
 *  Created on: 29 dic 2018
 *      Author: marco
 */

#ifndef SRC_UTILS_DBG_PRINT_H_
#define SRC_UTILS_DBG_PRINT_H_

//---------------------------------------------------------------------------------------------

#include <stdio.h>

//---------------------------------------------------------------------------------------------

#define DBG_VERBOSE

//---------------------------------------------------------------------------------------------

#ifdef DBG_VERBOSE
#define DBG_PRINT(...) do { fprintf(stderr, __VA_ARGS__ ); } while (0)
#else
#define DBG_PRINT(...) do { } while (0)
#endif

//---------------------------------------------------------------------------------------------

#endif /* SRC_UTILS_DBG_PRINT_H_ */
