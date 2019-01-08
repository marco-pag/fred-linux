/*
 * Fred for Linux. Experimental support.
 *
 * Copyright (C) 2018, Marco Pagani, ReTiS Lab.
 * <marco.pag(at)outlook.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
*/

#ifndef __PARAMETERS_H__
#define __PARAMETERS_H__

//-------------------------------------------------------------------------------

#define LIST_SOCK_PATH			"/tmp/fred_sock"

#define FRED_PATH				"/fredsys/"

#define LOG_FILE				"/fredsys/log.txt"

//-------------------------------------------------------------------------------

#define MAX_SW_TASKS			1024

//-------------------------------------------------------------------------------

#define MAX_HW_TASKS			128

#define HW_OP_ARGS_SIZE			8

#define MAX_DATA_BUFFS			HW_OP_ARGS_SIZE

//-------------------------------------------------------------------------------

#define MAX_SLOTS				64

#define MAX_PARTITIONS			32

//-------------------------------------------------------------------------------

#define MAX_NAMES				128

#define MAX_PATH				1024

//-------------------------------------------------------------------------------

#define MAX_EVENTS_SRCS 		(MAX_SLOTS * MAX_PARTITIONS + MAX_SW_TASKS + 1)

//-------------------------------------------------------------------------------

#endif // __PARAMETERS_H__
