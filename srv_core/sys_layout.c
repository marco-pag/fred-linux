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
#include <string.h>

#include "sys_layout.h"
#include "hw_task.h"
#include "slot.h"
#include "../srv_support/pars.h"
#include "../parameters.h"
#include "../utils/dbg_print.h"

//---------------------------------------------------------------------------------------------

// TODO: add some range checks
static inline size_t str_to_size_(const char *string)
{
	if (!string)
		return 0;

	return (size_t)strtol(string, (char **)NULL, 10);
}

static inline uint32_t str_to_uint32_(const char *string)
{
	if (!string)
		return 0;

	return (uint32_t)strtol(string, (char **)NULL, 10);
}

//---------------------------------------------------------------------------------------------

static
int build_partitions_(struct sys_layout *self, const struct sys_hw_config *hw_config,
						const char *arch_file, struct scheduler *scheduler)
{
	int retval = 0;
	char arch_path[MAX_PATH];
	struct tokens *tokens = NULL;

	const char *part_name;
	int part_slots_count;

	struct slot *slot = NULL;
	char dev_name[MAX_NAMES];
	char dec_name[MAX_NAMES];

	DBG_PRINT("fred_sys: building partitions\n");

	// Read arch file tokens
	// FRED_PATH is not inserted by the user but chosen at compile time
	strcpy(arch_path, FRED_PATH);
	strncat(arch_path, arch_file, sizeof(arch_path) - strlen(arch_path) - 1);
	retval = pars_tokenize(&tokens, arch_path);
	if (retval < 0)
		return -1;

	// Partition file contains one line per partition
	self->partitions_count = pars_get_num_lines(tokens);
	if (self->partitions_count >= MAX_PARTITIONS) {
		ERROR_PRINT("fred_sys: maximum number of partitions exceeded\n");
		pars_free_tokens(tokens);
		return -1;
	}

	// Create partitions
	for (int p = 0; p < self->partitions_count; ++p) {
		// Partition name (first token in the line)
		part_name = pars_get_token(tokens, p, 0);
		// Number of slots (second token)
		part_slots_count = str_to_size_(pars_get_token(tokens, p, 1));

		// Initialize partition
		retval = partition_init(&self->partitions[p], part_name, p);
		if (retval < 0) {
			ERROR_PRINT("fred_sys: error: unable to initialize partition %s\n", part_name);
			pars_free_tokens(tokens);
			return -1;
		}

		// Populate each slot
		for (int s = 0; s < part_slots_count; ++s) {
			// Init UIO device driver for slot and decoupler
			// Names must match device tree names
			sprintf(dev_name, "slot_p%u_s%u", p, s);
			sprintf(dec_name, "pr_decoupler_p%u_s%u", p, s);

			// Create a new slot
			retval = slot_init(&slot, hw_config, s, dev_name, dec_name, scheduler);
			if (retval < 0) {
				ERROR_PRINT("fred_sys: error: unable to initialize slot %u of "
							"partition %s\n", s, part_name);
				pars_free_tokens(tokens);
				return -1;
			}

			// And add to the partition
			partition_add_slot(self->partitions[p], slot);
		}
	}
	return 0;
}

static int build_hw_tasks_(struct sys_layout *self, const char *hw_tasks_file)
{
	int retval = 0;
	struct tokens *tokens;

	char hw_tasks_path[MAX_PATH];

	struct partition *partition = NULL;
	const char *hw_task_name = NULL;
	uint32_t hw_task_id;
	const char *part_name = NULL;
	const char *bits_path = NULL;
	int data_buffs_count;
	unsigned int data_buff_size;

	DBG_PRINT("fred_sys: building hw-tasks\n");

	// Read hw tasks tokens
	// FRED_PATH is not inserted by the user but chosen at compile time
	strcpy(hw_tasks_path, FRED_PATH);
	strncat(hw_tasks_path, hw_tasks_file, sizeof(hw_tasks_path) - strlen(hw_tasks_path) - 1);
	retval = pars_tokenize(&tokens, hw_tasks_path);
	if (retval < 0)
		return -1;

	// One line for each hw-task
	self->hw_tasks_count = pars_get_num_lines(tokens);
	if (self->hw_tasks_count >= MAX_HW_TASKS) {
		ERROR_PRINT("fred_sys: maximum number of hw-task exceeded\n");
		pars_free_tokens(tokens);
		return -1;
	}

	// Populate hw tasks array
	for (int i = 0; i < self->hw_tasks_count; ++i) {

		// Get hw-task name (first token in the line)
		hw_task_name = pars_get_token(tokens, i, 0);
		// Get hw-task id (second token in the line),
		hw_task_id = str_to_uint32_(pars_get_token(tokens, i, 1));
		// Get hw-task partition (third token)
		part_name = pars_get_token(tokens, i, 2);
		// Get bitstreams sub-path path (fourth token)
		bits_path = pars_get_token(tokens, i, 3);

		// Find partition
		for (int j = 0; j < self->partitions_count; ++j) {
			if (!strncmp(part_name, partition_get_name(self->partitions[j]),MAX_NAMES)) {
				partition = self->partitions[j];
				break;
			}
		}

		// If no partition has been found
		if (!partition) {
			ERROR_PRINT("fred_sys: error: partition not found for HW-task %s\n", hw_task_name);
			pars_free_tokens(tokens);
			return -1;
		}

		// Initialize hw_task
		retval = hw_task_init(&self->hw_tasks[i], hw_task_id,
								hw_task_name, bits_path, partition, self->buffctl);
		if (retval) {
			ERROR_PRINT("fred_sys: error: unable to initialize HW-task %s\n", hw_task_name);
			pars_free_tokens(tokens);
			return -1;
		}

		// The reminder tokens (after fourth initial tokens) define the buffers
		data_buffs_count = pars_get_num_tokens(tokens, i) - 4;
		for (int b = 0; b < data_buffs_count; ++b) {
			data_buff_size = str_to_size_(pars_get_token(tokens, i, b + 4));
			retval = hw_task_add_buffer(self->hw_tasks[i], data_buff_size);
			if (retval)
				return -1;
		}
	}
	return 0;
}

//---------------------------------------------------------------------------------------------

struct hw_task *sys_layout_get_hw_task(const struct sys_layout *self, uint32_t hw_task_id)
{
	if (!self)
		return NULL;

	for (int i = 0; i < self->hw_tasks_count; ++i) {
		if (hw_task_get_id(self->hw_tasks[i]) == hw_task_id)
			return self->hw_tasks[i];
	}

	return NULL;
}

int sys_layout_get_hw_tasks(const struct sys_layout *self, struct hw_task **hw_tasks)
{
	if (!self || !hw_tasks)
		return -1;

	for (int i = 0; i < self->hw_tasks_count; ++i)
		hw_tasks[i] = self->hw_tasks[i];

	return self->hw_tasks_count;
}

int sys_layout_register_slots(struct sys_layout *self, struct reactor *reactor)
{
	int retval;

	assert(self);
	assert(reactor);

	for (int i = 0; i < self->partitions_count; ++i) {
		retval = partiton_register_slots(self->partitions[i], reactor);
		if (retval)
			return -1;
	}

	return 0;
}

void sys_layout_print(const struct sys_layout *self)
{
	char name[MAX_NAMES];

	DBG_PRINT("------------------------------------ Layout "
				"------------------------------------\n");


	DBG_PRINT("Partitions:\n");
	for (int i = 0; i < self->partitions_count; ++i) {
		partiton_print(self->partitions[i], name, MAX_NAMES);
		DBG_PRINT("\t%s\n", name);
	}

	DBG_PRINT("Hw-tasks:\n");
	for (int i = 0; i < self->hw_tasks_count; ++i) {
		hw_task_print(self->hw_tasks[i], name, MAX_NAMES);
		DBG_PRINT("\t%s\n", name);
	}

	DBG_PRINT("--------------------------------------------"
				"------------------------------------\n");
}

int sys_layout_init(struct sys_layout **self, const struct sys_hw_config *hw_config,
					const char *arch_file, const char *hw_tasks_file,
					struct scheduler *scheduler, buffctl_ft *buffctl)
{
	int retval;

	assert(buffctl);
	assert(scheduler);
	assert(arch_file);
	assert(hw_tasks_file);

	DBG_PRINT("fred_sys: building system\n");

	// Zero everything including partition and HW-tasks arrays
	*self = calloc(1, sizeof(**self));
	if (!(*self))
		return -1;

	(*self)->buffctl = buffctl;

	retval = build_partitions_(*self, hw_config, arch_file, scheduler);
	if (retval) {
		ERROR_PRINT("fred_sys: error while building partitions\n");
		goto error_clean;
	}

	retval = build_hw_tasks_(*self, hw_tasks_file);
	if (retval) {
		ERROR_PRINT("fred_sys: error while building HW-tasks\n");
		goto error_clean;
	}

	return 0;

error_clean:
	sys_layout_free(*self);
	return -1;
}

void sys_layout_free(struct sys_layout *self)
{
	if (!self)
		return;

	for (int i = 0; i < MAX_PARTITIONS; ++i) {
		if (self->partitions[i])
			partition_free(self->partitions[i]);
	}

	for (int i = 0; i < MAX_HW_TASKS; ++i) {
		if (self->hw_tasks[i])
			hw_task_free(self->hw_tasks[i], self->buffctl);
	}

	free(self);
}
