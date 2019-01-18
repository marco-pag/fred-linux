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

#ifndef LOGGER_H_
#define LOGGER_H_

#include <stdio.h>

#define LOG_LEV_MUTE		0
#define LOG_LEV_SIMPLE		1
#define LOG_LEV_FULL		2
#define LOG_LEV_PEDANTIC	3

//---------------------------------------------------------------------------------------------

#ifndef LOG_GLOBAL_LEVEL
#define LOG_GLOBAL_LEVEL LOG_LEV_PEDANTIC
#endif

//---------------------------------------------------------------------------------------------

#define logger_log(level, ...) \
do {  \
	if (level <= LOG_GLOBAL_LEVEL) { \
		fprintf(fred_log.stream, __VA_ARGS__); \
		fflush(fred_log.stream); \
	} \
} while (0)

//---------------------------------------------------------------------------------------------

typedef struct logger_ {
	FILE *stream;
	enum {LOG_CLOSE, LOG_OPEN} state;
} logger;

extern logger fred_log;

//---------------------------------------------------------------------------------------------

int logger_init();

void logger_free();

//---------------------------------------------------------------------------------------------

#endif /* LOGGER_H_ */
