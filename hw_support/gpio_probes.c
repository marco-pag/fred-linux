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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "gpio_probes.h"
#include "../utils/dbg_print.h"

// Zybo PmodJF pins
#define JF1_PIN     13
#define JF2_PIN     10
#define JF3_PIN     11
#define JF4_PIN     12

#define JF7_PIN     0
#define JF8_PIN     9
#define JF9_PIN     14
#define JF10_PIN    15

#define GPIO_BASE   906
#define PINS_LEN    8

#define GPIO_PATH "/sys/class/gpio/"

struct pin_ {
    int fd;
    uint32_t idx;
};

struct probes_ {
    enum {ERROR, OPEN} state;
    int exp_fd;
    struct pin_ pins[PINS_LEN];
};

static struct probes_ probes = {
        .pins = {
            {0, JF1_PIN + GPIO_BASE},
            {0, JF2_PIN + GPIO_BASE},
            {0, JF3_PIN + GPIO_BASE},
            {0, JF4_PIN + GPIO_BASE},

            {0, JF7_PIN + GPIO_BASE},
            {0, JF8_PIN + GPIO_BASE},
            {0, JF9_PIN + GPIO_BASE},
            {0, JF10_PIN + GPIO_BASE}
        },
};

int gpio_probes_init()
{
    char str[1024];
    int fd;
    int retval;

    strcpy(str, GPIO_PATH);
    strcat(str, "export");

    // Open export file
    probes.exp_fd = open(str, O_WRONLY);
    if (probes.exp_fd < 0) {
        ERROR_PRINT("fred_probes: error while opening GPIO export file\n");
        return -1;
    }

    for (size_t p = 0; p < PINS_LEN; ++p) {

        // Enable pin
        sprintf(str, "%d", probes.pins[p].idx);
        retval = write(probes.exp_fd, str, 4);
        if (retval < 0) {
            ERROR_PRINT("fred_probes: error while enabling pin\n");
            return -1;
        }

        // Open direction file
        sprintf(str, "%sgpio%d/direction", GPIO_PATH, probes.pins[p].idx);
        fd = open(str, O_RDWR);
        if (fd < 0) {
            ERROR_PRINT("fred_probes: error while opening GPIO direction file\n");
            return -1;
        }

        // Set for output
        retval = write(fd, "out", 4);
        if (retval < 0) {
            ERROR_PRINT("fred_probes: error while setting GPIO pin for output\n");
            return -1;
        }

        close(fd);

        // Open value file
        sprintf(str, "%sgpio%d/value", GPIO_PATH, probes.pins[p].idx);
        fd = open(str, O_RDWR);
        if (fd < 0) {
            ERROR_PRINT("fred_probes: error while opening GPIO value file\n");
            return -1;
        }

        // Set zero and keep fd open
        retval = write(fd, "0", 2);
        if (retval < 0) {
            ERROR_PRINT("fred_probes: error while setting GPIO pin to zero\n");
            return -1;
        }

        probes.pins[p].fd = fd;
    }

    return 0;
}

void gpio_probes_free()
{
    for (size_t p = 0; p < PINS_LEN; ++p)
        close(probes.pins[p].fd);
}


void gpio_probes_set_pin(size_t pin_ixd)
{
#ifdef GPIO_PRB_ENABLED
    int retval;

    if (pin_ixd >= PINS_LEN)
        return;

    retval = write(probes.pins[pin_ixd].fd, "1", 2);
    if (retval < 0) {
        ERROR_PRINT("fred_probes: error while setting GPIO pin\n");
    }
#endif
}

void gpio_probes_clear_pin(size_t pin_ixd)
{
#ifdef GPIO_PRB_ENABLED
    int retval;

    if (pin_ixd >= PINS_LEN)
        return;

    retval = write(probes.pins[pin_ixd].fd, "0", 2);
    if (retval < 0) {
        ERROR_PRINT("fred_probes: error while clearing GPIO pin\n");
    }
#endif
}


