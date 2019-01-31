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


#include <stdio.h>

//---------------------------------------------------------------------------------------------

#include "logger.h"
#include "../parameters.h"
#include "../utils/dbg_print.h"

//---------------------------------------------------------------------------------------------

logger fred_log;

//---------------------------------------------------------------------------------------------

int logger_init()
{
	fred_log.stream = fopen(LOG_FILE, "w");
	if (!fred_log.stream) {
		ERROR_PRINT("fred_log: error while opening log file");
		return -1;
	}

	fred_log.state = LOG_OPEN;

	return 0;
}

void logger_free()
{
	if (fred_log.state == LOG_CLOSE)
		return;

	fclose(fred_log.stream);
}
