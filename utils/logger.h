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

#ifndef LOGGER_H_
#define LOGGER_H_

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>

#include "../parameters.h"

//---------------------------------------------------------------------------------------------

#define LOG_LEV_MUTE        0
#define LOG_LEV_SIMPLE      1
#define LOG_LEV_FULL        2
#define LOG_LEV_PEDANTIC    3

//---------------------------------------------------------------------------------------------

#define logger_log(level, format, ...) \
do {  \
    if (fred_log.state == LOG_OPEN && level <= LOG_GLOBAL_LEVEL) { \
        struct timespec log_ts_now; \
        uint64_t log_tstamp; \
        clock_gettime(CLOCK_MONOTONIC, &log_ts_now); \
        log_tstamp = (log_ts_now.tv_sec * 1000000 + log_ts_now.tv_nsec / 1000) \
                        - fred_log.t_begin; \
        fprintf(fred_log.stream, "%016"PRIu64": "format"\n", log_tstamp, ##__VA_ARGS__); \
        fflush(fred_log.stream); \
    } \
} while (0)

//---------------------------------------------------------------------------------------------

typedef struct logger_ {
    enum {LOG_CLOSE = 0, LOG_OPEN = 1} state;
    FILE *stream;
    uint64_t t_begin;
} logger;

extern logger fred_log;

//---------------------------------------------------------------------------------------------

int logger_init();

void logger_free();

//---------------------------------------------------------------------------------------------

#endif /* LOGGER_H_ */
