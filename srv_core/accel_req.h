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

#ifndef ACCEL_REQ_H_
#define ACCEL_REQ_H_

#include <stdint.h>
#include <assert.h>
#include <time.h>
#include <sys/queue.h>

#include "../parameters.h"

//---------------------------------------------------------------------------------------------
// Forward declarations to avoid circular dependencies
// from including slot.h and hw_task.h

struct slot;
struct hw_task;
struct sw_task_client;

//---------------------------------------------------------------------------------------------

struct accel_req {

	// Hw-task parameters array
	uintptr_t args[HW_OP_ARGS_SIZE];
	int args_size;

	struct hw_task *hw_task;
	struct slot *slot;

	// Parent client
	struct sw_task_client *sw_task_client;

	// Issuing time stamp
	struct timespec tstamp;

	// Optimization
	int skip_rcfg;

	// Notify when the request has been executed
	// (whole acceleration process has been completed)
	int (*notify_action)(void *self, int notify_sock);
	int notify_sock;

	// List element (next and previous element pointers)
	TAILQ_ENTRY(accel_req) queue_elem;
};

// Define "accel_req_queue" struct type for a tailq list of "accel_req"
TAILQ_HEAD(accel_req_queue, accel_req);


//---------------------------------------------------------------------------------------------

static inline
void accel_req_unbind(struct accel_req *self)
{
	assert(self);

	self->hw_task = NULL;
	self->slot = NULL;
	self->skip_rcfg = 0;
}

static inline
const uintptr_t *accel_req_get_args(const struct accel_req *self)
{
	assert(self);

	return self->args;
}

static inline
void accel_req_set_args(struct accel_req *self, int index, uintptr_t value)
{
	assert(self);
	assert(index < HW_OP_ARGS_SIZE);

	self->args[index] = value;
}

static inline
int accel_req_get_args_size(const struct accel_req *self)
{
	assert(self);

	return self->args_size;
}

static inline
void accel_req_set_args_size(struct accel_req *self, int args_size)
{
	assert(self);

	self->args_size = args_size;
}

static inline
int accel_req_get_skip_rcfg(const struct accel_req *self)
{
	assert(self);

	return self->skip_rcfg;
}

static inline
void accel_req_set_skip_rcfg(struct accel_req *self)
{
	assert(self);

	self->skip_rcfg = 1;
}

static inline
struct slot *accel_req_get_slot(const struct accel_req *self)
{
	assert(self);

	return self->slot;
}

static inline
void accel_req_set_slot(struct accel_req *self, struct slot *slot)
{
	assert(self);

	self->slot = slot;
}

static inline
struct hw_task *accel_req_get_hw_task(const struct accel_req *self)
{
	assert(self);

	return self->hw_task;
}

static inline
void accel_req_set_hw_task(struct accel_req *self, struct hw_task *hw_task)
{
	assert(self);

	self->hw_task = hw_task;
}

static inline
void accel_req_set_sw_task_client(struct accel_req *self,
									struct sw_task_client *sw_task_client)
{
	assert(self);

	self->sw_task_client = sw_task_client;
}

static inline
void accel_req_stamp_timestamp(struct accel_req *self)
{
	assert(self);

	clock_gettime(CLOCK_MONOTONIC, &self->tstamp);
}

static inline
int accel_req_compare_timestamps(const struct accel_req *self,
									const struct accel_req *other)
{
	int retval;

	assert(self);
	assert(other);

	if (self->tstamp.tv_sec > other->tstamp.tv_sec) {
		retval = 1;
	} else if (self->tstamp.tv_sec < other->tstamp.tv_sec) {
		retval = -1;
	} else if (self->tstamp.tv_nsec > other->tstamp.tv_nsec) {
		retval = 1;
	} else if (self->tstamp.tv_nsec < other->tstamp.tv_nsec) {
		retval = -1;
	} else {
		retval = 0;
	}

	return retval;
}

//---------------------------------------------------------------------------------------------

int accel_req_notify_action(struct accel_req *self);

const struct phy_bitstream *accel_req_get_phy_bit(const struct accel_req *self);

//---------------------------------------------------------------------------------------------

#endif /* ACCEL_REQ_H_ */
