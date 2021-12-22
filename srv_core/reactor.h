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

#ifndef REACTOR_H_
#define REACTOR_H_

#include "event_handler.h"

//--- Reactor opaque type ---------------------------------------------------------------------

struct reactor;

//--- Reactor interface -----------------------------------------------------------------------

enum react_handler_mode {
    REACT_NORMAL_HANDLER,
    REACT_PRI_HANDLER
};

enum react_handler_ownership {
    REACT_NOT_OWNED,
    REACT_OWNED
};

int reactor_init(struct reactor **self);

void reactor_free(struct reactor *self);

int reactor_add_event_handler(struct reactor *self, struct event_handler *event_handler,
                                enum react_handler_mode handler_mode,
                                enum react_handler_ownership handler_ownership);

void reactor_event_loop(struct reactor *self);

//---------------------------------------------------------------------------------------------

#endif /* REACTOR_H_ */
