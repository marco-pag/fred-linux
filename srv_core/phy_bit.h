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

#ifndef PHY_BIT_H_
#define PHY_BIT_H_

#include <stddef.h>
#include <assert.h>

struct phy_bit {
    uintptr_t addr;
    size_t size;
};

static inline
void phy_bit_set(struct phy_bit *self, uintptr_t addr, size_t size)
{
    assert(self);

    self->addr = addr;
    self->size = size;
}

static inline
uintptr_t phy_bit_get_addr(const struct phy_bit *self)
{
    assert(self);

    return self->addr;
}

static inline
size_t phy_bit_get_size(const struct phy_bit *self)
{
    assert(self);

    return self->size;
}

#endif /* PHY_BIT_H_ */
