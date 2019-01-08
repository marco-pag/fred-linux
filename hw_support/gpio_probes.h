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

#ifndef GPIO_PROBES_H_
#define GPIO_PROBES_H_

#define GPIO_PRB_ENABLED

int gpio_probes_init();

void gpio_probes_free();

void gpio_probes_set_pin(size_t pin_ixd);

void gpio_probes_clear_pin(size_t pin_ixd);

#endif /* GPIO_PROBES_H_ */
