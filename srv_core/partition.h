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

#ifndef PARTITION_H_
#define PARTITION_H_

#include <assert.h>

#include "../parameters.h"
#include "slot_timer.h"
#include "reactor.h"

//---------------------------------------------------------------------------------------------
// Forward declarations to avoid circular dependencies
// between this header slot.h, and hw_task.h

struct hw_task;
struct slot;

//---------------------------------------------------------------------------------------------

struct partition {
    char name[MAX_NAMES];
    int index;

    struct slot *slots[MAX_SLOTS];
    int slots_count;

    // One execution timer per slot
    struct slot_timer *timers[MAX_SLOTS];
};

//---------------------------------------------------------------------------------------------

static inline
const char* partition_get_name(const struct partition *self)
{
    assert(self);

    return self->name;
}

static inline
int partition_get_index(const struct partition *self)
{
    assert(self);

    return self->index;
}

static inline
int partition_get_slots_count(const struct partition *self)
{
    assert(self);

    return self->slots_count;
}

//---------------------------------------------------------------------------------------------

int partition_init(struct partition **self, const char *name, int index);

void partition_free(struct partition *self);

int partition_add_slot(struct partition *self, struct slot *slot, struct slot_timer *timer);

// hw_task is passed for reconfiguration optimization.
// Set slot pointer to the the first free slot.
// If all slots are occupied set slot pointer to null
// if no reconfiguration required return 0, otherwise 1
int partition_search_slot(struct partition *self, struct slot **slot,
                            struct slot_timer **timer, const struct hw_task *hw_task);

// Pick a random slot among the slots that are free (for testing proposes)
int partition_search_random_slot(struct partition *self, struct slot **slot,
                                    struct slot_timer **timer, const struct hw_task *hw_task);

int partiton_register_slots(struct partition *self, struct reactor *reactor);

void partiton_print(const struct partition *self, char *str, int str_size);

//---------------------------------------------------------------------------------------------

#endif /* PARTITION_H_ */
