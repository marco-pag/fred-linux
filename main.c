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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "srv_core/fred_sys.h"
#include "parameters.h"

//---------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
    int retval;
    int opts;
    struct fred_sys *fred_sys;
    enum fred_sys_mode mode;

    opterr = 0;
    opts = getopt(argc, argv, "hre");

    switch (opts) {
        case 'h':
            printf("Use -r for reconfiguration test, -e for execution test\n");
            return 0;
            break;
        case 'r':
            mode = FRED_SYS_RCFG_TEST_MODE;
            break;
        case 'e':
            mode = FRED_SYS_HW_TASKS_TEST_MODE;
            break;
        default:
            mode = FRED_SYS_NORMAL_MODE;
            break;
    }

    retval = fred_sys_init(&fred_sys, ARCH_FILE, HW_TASKS_FILE, mode);
    if (retval < 0)
        return -1;

    // Event loop
    fred_sys_run(fred_sys);

    fred_sys_free(fred_sys);

    return 0;
}

