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

#include <stdlib.h>

#include "slot.h"
#include "../utils/logger.h"
#include "../utils/dbg_print.h"
#include "scheduler_fred.h"

//---------------------------------------------------------------------------------------------

static inline
int start_slot_after_rcfg_(struct scheduler_fred *self, struct accel_req *request_done);

static inline
int start_rcfg_(struct scheduler_fred *self, struct accel_req *request);

static inline
int push_req_fri_queue_(struct scheduler_fred *self, struct accel_req *request);

//---------------------------------------------------------------------------------------------

static inline
int start_slot_after_rcfg_(struct scheduler_fred *self, struct accel_req *request_done)
{
	int retval;
	struct slot *slot;
	struct accel_req *next_request;

	slot = accel_req_get_slot(request_done);
	assert(slot);

	// Start the hardware accelerator
	retval = slot_start_compute(slot, request_done);
	if (retval < 0)
		return -1;

	logger_log(LOG_LEV_FULL,"\tfred_sys: slot: %d of partition: %s"
							" started for hw-task: %s",
							slot_get_index(slot),
							partition_get_name(hw_task_get_partition(
								accel_req_get_hw_task(request_done))),
							hw_task_get_name(accel_req_get_hw_task(request_done)));

	// If the FRI queue is not empty start next reconfiguration
	if (!TAILQ_EMPTY(&self->fri_queue_head)) {

		// Get the head request from the FRI queue
		next_request = TAILQ_FIRST(&self->fri_queue_head);

		// Remove the head request from the FRI queue
		TAILQ_REMOVE(&self->fri_queue_head, next_request, queue_elem);

		logger_log(LOG_LEV_PEDANTIC,"\tfred_sys: FRI queue not empty");

		// Start reconfiguration
		retval = start_rcfg_(self, next_request);
		if (retval < 0)
			return -1;
	}

	return 0;
}


static inline
int start_rcfg_(struct scheduler_fred *self, struct accel_req *request)
{
	int retval;

	// If the slot already contains the hw-task
	if (accel_req_get_skip_rcfg(request)) {
		logger_log(LOG_LEV_FULL,"\tfred_sys: skipping rcfg of slot: %d"
								" of partition: %s for hw-task: %s",
								slot_get_index(accel_req_get_slot(request)),
								partition_get_name(hw_task_get_partition(
										accel_req_get_hw_task(request))),
								hw_task_get_name(accel_req_get_hw_task(request)));

		// Start the slot immediately
		retval = start_slot_after_rcfg_(self, request);

	} else {
		logger_log(LOG_LEV_FULL,"\tfred_sys: start rcfg of slot: %d"
								" of partition: %s for hw-task: %s",
								slot_get_index(accel_req_get_slot(request)),
								partition_get_name(hw_task_get_partition(
										accel_req_get_hw_task(request))),
								hw_task_get_name(accel_req_get_hw_task(request)));

		// Start FPGA reconfiguration and bind hw-task to the slot
		slot_prepare_for_rcfg(accel_req_get_slot(request));
		retval = devcfg_start_prog(self->devcfg, request);
	}

	return retval;
}

static inline
void ins_req_ordered_(struct accel_req_queue *queue_head, struct accel_req *new_request)
{
	struct accel_req *req = NULL;

	// If queue is empty
	if (TAILQ_EMPTY(queue_head)) {
		TAILQ_INSERT_HEAD(queue_head, new_request, queue_elem);
		return;
	}

	// Fri queue contains at least one element
	TAILQ_FOREACH(req, queue_head, queue_elem) {
		if (accel_req_compare_timestamps(req, new_request) == 1)
			break;
	}

	// Last
	if (req == NULL)
		TAILQ_INSERT_TAIL(queue_head, new_request, queue_elem);
	else
		TAILQ_INSERT_BEFORE(req, new_request, queue_elem);
}

//TODO: integrate the function above to avoid insert and remove
static inline
int push_req_fri_queue_(struct scheduler_fred *self, struct accel_req *request)
{
	// Insert the request directly into FRI queue
	ins_req_ordered_(&self->fri_queue_head, request);

	// If the inserted request is on top of FRI queue
	// and the DEVCFG is IDLE (not programming)
	if (devcfg_is_idle(self->devcfg) &&
		TAILQ_FIRST(&self->fri_queue_head) == request) {

		logger_log(LOG_LEV_PEDANTIC,"\tfred_sys: DevCfg idle & request on top");

		// Remove the head request from the FRI queue
		TAILQ_REMOVE(&self->fri_queue_head, request, queue_elem);

		// Start reconfiguration
		return start_rcfg_(self, request);
	}

	return 0;
}

// ------------------------ Functions to implement scheduler interface ------------------------

// Acceleration request from software tasks
static
int sched_fred_push_accel_req_(struct scheduler *self, struct accel_req *request)
{
	int retval;
	int rcfg;
	struct scheduler_fred *sched;
	struct hw_task *hw_task;
	struct slot *slot;
	struct partition *partition;

	assert(self);
	assert(request);

	sched = (struct scheduler_fred *)self;

	// Set request's time stamp
	accel_req_stamp_timestamp(request);

	// Search a free slot in the partition
	hw_task = accel_req_get_hw_task(request);
	partition = hw_task_get_partition(hw_task);
	rcfg = partition_search_slot(partition, &slot, hw_task);

	// If all slots in the partition are occupied
	if (!slot) {
		logger_log(LOG_LEV_FULL,"\tfred_sys: all slots are busy for hw-task %s"
								", insert into partition %s queue",
								hw_task_get_name(hw_task),
								partition_get_name(partition));

		// Insert the new request into the partition FIFO queue
		TAILQ_INSERT_TAIL(&sched->part_queues_heads[partition_get_index(partition)],
							request, queue_elem);

		retval = 0;

	// At least one free slot in the partition
	} else {
		// Reserve the slot for the HW-task
		slot_set_reserved(slot);
		accel_req_set_slot(request, slot);

		// If possible, set for skipping rcfg
		if (!rcfg && sched->mode != SCHED_FRED_ALWAYS_RCFG)
			accel_req_set_skip_rcfg(request);

		logger_log(LOG_LEV_FULL,"\tfred_sys: hw-task: %s got slot: %d of"
								" its partition: %s, inserted in fri queue",
								hw_task_get_name(hw_task),
								slot_get_index(slot),
								partition_get_name(partition));

		// Push request into FRI queue. If request goes on top of FRI queue
		// and devcfg is idle start reconfiguration immediately
		retval = push_req_fri_queue_(sched, request);
	}

	return retval;
}

// Reconfiguration done
static
int sched_fred_rcfg_complete_(struct scheduler *self, struct accel_req *request_done)
{
	struct scheduler_fred *sched;
	struct slot *slot;
	int rcfg_time_us;

	assert(self);
	assert(request_done);

	sched = (struct scheduler_fred *)self;

	// Get the slot that has been reconfigured
	slot = accel_req_get_slot(request_done);
	assert(slot);

	// Clear devcfg event
	rcfg_time_us = (int)devcfg_clear_evt(sched->devcfg);
	if (rcfg_time_us <= 0)
		return -1;

	logger_log(LOG_LEV_FULL,"\tfred_sys: devcfg, slot: %d of partition: %s"
							" rcfg completed for hw-task: %s in %d us",
							slot_get_index(slot),
							partition_get_name(hw_task_get_partition(
								accel_req_get_hw_task(request_done))),
							hw_task_get_name(accel_req_get_hw_task(request_done)),
							rcfg_time_us);

	// Re-enable slot after it has been reconfigured
	slot_reinit_after_rcfg(slot);

#ifdef RCFG_CHECK
	// Only for testing
	// Check if the right hw-task has been reconfigured
	if (!slot_check_hw_task_consistency(slot)) {
		ERROR_PRINT("\tfred_sys: critical error: mismatch on slot %d"
					" of partition %s for hw-task %s",
					slot_get_index(slot),
					partition_get_name(hw_task_get_partition(
						accel_req_get_hw_task(request_done))),
					hw_task_get_name(accel_req_get_hw_task(request_done)));

		return -1;
	}
#endif

	// Start the hardware accelerator
	return start_slot_after_rcfg_(sched, request_done);
}

// Hardware task execution completed
static
int sched_fred_slot_complete_(struct scheduler *self, struct accel_req *request_done)
{
	int retval;
	struct scheduler_fred *sched;
	struct slot *slot;
	struct partition *partition;
	struct accel_req *request;
	struct accel_req_queue *part_queue_head;

	assert(self);
	assert(request_done);

	sched = (struct scheduler_fred *)self;

	slot = accel_req_get_slot(request_done);
	assert(slot);

	partition = hw_task_get_partition(accel_req_get_hw_task(request_done));
	assert(partition);

	logger_log(LOG_LEV_FULL,"\tfred_sys: slot: %d of partition: %s"
							" completed execution of hw-task: %s in %u us",
							slot_get_index(slot), partition_get_name(partition),
							hw_task_get_name(accel_req_get_hw_task(request_done)),
							slot_get_exec_time_us(slot));

	// Clear slot device
	slot_clear_after_compute(slot);

	// Notify the client through the notify action callback
	retval = accel_req_notify_action(request_done);
	if (retval)
		return retval;

	// Check if there are pending requests in the partition queue (queue not empty)
	part_queue_head = &sched->part_queues_heads[partition_get_index(partition)];
	if (!TAILQ_EMPTY(part_queue_head)) {

		// Get the head request from the partition FIFO queue
		request = TAILQ_FIRST(part_queue_head);

		// Remove the request from the partition FIFO queue
		TAILQ_REMOVE(part_queue_head, request, queue_elem);

		// Reserve the slot for the HW-task
		slot_set_reserved(slot);
		accel_req_set_slot(request, slot);

		// Push request into FRI queue. If request goes on top of FRI queue
		// and devcfg is idle start reconfiguration immediately
		retval = push_req_fri_queue_(sched, request);
	}

	return retval;
}

static
void sched_fred_free_(struct scheduler *self)
{
	struct scheduler_fred *sched;

	sched = (struct scheduler_fred *)self;

	if(sched)
		free(sched);
}

//---------------------------------------------------------------------------------------------

int sched_fred_init(struct scheduler **self, enum sched_fred_mode mode, struct devcfg *devcfg)
{
	struct scheduler_fred *sched;

	assert(devcfg);

	*self = NULL;

	// Allocate and set everything to 0
	sched = calloc(1, sizeof(*sched));
	if (!sched)
		return -1;

	// Set properties and methods
	sched->mode = mode;
	sched->devcfg = devcfg;

	// Scheduler interface
	sched->scheduler.push_accel_req = sched_fred_push_accel_req_;
	sched->scheduler.rcfg_complete = sched_fred_rcfg_complete_;
	sched->scheduler.slot_complete = sched_fred_slot_complete_;
	sched->scheduler.free = sched_fred_free_;

	// Initialize partition queues heads
	for (int i = 0; i < MAX_PARTITIONS; ++i)
		TAILQ_INIT(&(sched->part_queues_heads[i]));

	// And fri queue head
	TAILQ_INIT(&sched->fri_queue_head);

	*self = &sched->scheduler;

	return 0;
}
