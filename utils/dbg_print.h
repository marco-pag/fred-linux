/*
 * Fred for Linux. Experimental support.
 *
 * Copyright (C) 2018-2021, Marco Pagani, ReTiS Lab.
 * <marco.pag(at)outlook.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
*/

#ifndef SRC_UTILS_DBG_PRINT_H_
#define SRC_UTILS_DBG_PRINT_H_

//---------------------------------------------------------------------------------------------

#include <stdio.h>

//---------------------------------------------------------------------------------------------

#define DBG_VERBOSE
#define ERROR_VERBOSE

//---------------------------------------------------------------------------------------------

#ifdef DBG_VERBOSE
#define DBG_PRINT(...) do { fprintf(stdout, __VA_ARGS__ ); } while (0)
#else
#define DBG_PRINT(...) do { } while (0)
#endif

#ifdef ERROR_VERBOSE
#define ERROR_PRINT(...) do { fprintf(stderr, __VA_ARGS__ ); } while (0)
#else
#define ERROR_PRINT(...) do { } while (0)
#endif

//---------------------------------------------------------------------------------------------

#endif /* SRC_UTILS_DBG_PRINT_H_ */
