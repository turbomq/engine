/*
 *    Copyright (C) 2015 abi <abi@singiro.com>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "types.h"

typedef struct 
{
} turbo_fifo_t;

/*
 * Creates a instance of a fifo.
 */
turbo_fifo_t* turbo_fifo_create(release_func_t release_func);

/*
 * Destroys the given fifo.
 */
void turbo_fifo_destroy(turbo_fifo_t** fifo);

/*
 * Pushs the given data into fifo.
 */
int turbo_fifo_push(turbo_fifo_t* fifo, void* data);

/*
 * Pops one element from fifo. If the timeout exceeded, it would return NULL. 
 * Zero for timeout means wait forever.
 */
void* turbo_fifo_pop(turbo_fifo_t* fifo, int timeout);
