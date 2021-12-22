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

#ifndef HW_TASK_H_
#define HW_TASK_H_

#include <assert.h>
#include <stdint.h>

#include "phy_bit.h"
#include "partition.h"
#include "../parameters.h"
#include "../srv_support/buffctl.h"
#include "../shared_kernel/fred_buffctl_shared.h"

//---------------------------------------------------------------------------------------------

struct hw_task {
    // Software ID should match the module ID
    // exported by the hardware module
    uint32_t hw_id;
    char name[MAX_NAMES];

    struct partition *partition;

    // Bistreams (one for each slot of the partition)
    struct fred_buff_if *bits_buffs[MAX_SLOTS];         // Side buffer allocator module interface
    struct phy_bit bits_phys[MAX_SLOTS];                // Bitstream in a contiguous buffer [1]

    // Data buffers info (to be used when new buffs for
    // a client must be allocated)
    unsigned int data_buffs_sizes[MAX_DATA_BUFFS];
    int data_buffs_count;

    // Hardware timeout
    uint64_t timeout_us;
    int banned;
};

// [1]  - The size maybe less than the buffer size due to proprietary bitstreams mangling
//      - Future releases will try to use the DMA scatter-gather mode (if available)
//        to avoid using contiguous buffers.
//---------------------------------------------------------------------------------------------

static inline
uint32_t hw_task_get_id(const struct hw_task *self)
{
    assert(self);

    return self->hw_id;
}

static inline
const char *hw_task_get_name(const struct hw_task *self)
{
    assert(self);

    return self->name;
}

static inline
struct partition *hw_task_get_partition(const struct hw_task *self)
{
    assert(self);

    return self->partition;
}

static inline
int hw_task_get_data_buffs_count(const struct hw_task *self)
{
    assert(self);

    return self->data_buffs_count;
}

static inline
const unsigned int *hw_task_get_data_buffs_sizes(const struct hw_task *self)
{
    assert(self);

    return self->data_buffs_sizes;
}

static inline
const struct phy_bit *hw_task_get_bit_phy(const struct hw_task *self,
                                                int slot_idx)
{
    assert(self);
    assert(slot_idx < partition_get_slots_count(self->partition));

    return &self->bits_phys[slot_idx];
}

static inline
uint64_t hw_task_get_timeout_us(const struct hw_task *self)
{
    assert(self);

    return self->timeout_us;
}

static inline
void hw_task_set_timeout_us(struct hw_task *self, uint64_t timeout_us)
{
    assert(self);

    self->timeout_us = timeout_us;
}

static inline
void hw_task_set_banned(struct hw_task *self)
{
    assert(self);

    self->banned = 1;
}

static inline
int hw_task_get_banned(const struct hw_task *self)
{
    assert(self);

    return self->banned;
}

//---------------------------------------------------------------------------------------------

int hw_task_init(struct hw_task **self, uint32_t hw_id, const char *name,
                    const char *bits_path, struct partition *partition,
                    buffctl_ft *buffctl);

void hw_task_free(struct hw_task *self, buffctl_ft *buffctl);

int hw_task_add_buffer(struct hw_task *self, unsigned int buff_size);

void hw_task_print(const struct hw_task *self, char *str, int str_size);

#endif /* HW_TASK_H_ */
