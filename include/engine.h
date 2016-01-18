/*
 *    Copyright (C) 2015 abi <abisxir@gmail.com>
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

#include "queue.h"

typedef struct 
{
} turbo_engine_t;

/*
 * Creates a instance of an engine.
 */
turbo_engine_t* turbo_engine_create(const char* protocol, const char* host, int port, int num_threads);

/*
 * Runs the queue engine.
 */
int turbo_engine_run(turbo_engine_t* engine);

/*
 * Executes a command.
 */
int turbo_engine_execute(turbo_engine_t* engine, int64_t key, void* client);

/*
 * Stops turbo engine.
 */
int turbo_engine_stop(turbo_engine_t* engine);

/*
 * Destroys the given engine.
 */
void turbo_engine_destroy(turbo_engine_t** engine);

/*
 * Declares a queue.
 */
turbo_queue_t* turbo_engine_get_queue(turbo_engine_t* egnine, const char* name);
