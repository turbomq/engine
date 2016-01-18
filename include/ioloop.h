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

#include "engine.h"
#include "message.h"
#include "in_stream.h"
#include "out_stream.h"

typedef struct 
{
} turbo_ioloop_t;

typedef struct
{
    uint64_t ip;
    int socketfd;
    turbo_in_stream_t* in_stream;
    turbo_out_stream_t* out_stream;
} turbo_remote_client_t;

typedef int (*on_new_connection_func_t)(turbo_ioloop_t*, turbo_remote_client_t*);
typedef int (*on_read_func_t)(turbo_ioloop_t*, turbo_remote_client_t*);
typedef int (*on_write_func_t)(turbo_ioloop_t*, turbo_remote_client_t*);

/*
 * Creates an instance of IO loop.
 */
turbo_ioloop_t* turbo_ioloop_create(turbo_engine_t* engine, 
                                    const char* protocol,
                                    const char* host, 
                                    int port, 
                                    int num_threads);

/*
 * Runs the given IO loop.
 */
int turbo_ioloop_run(turbo_ioloop_t*);

/*
 * Stops the given IO loop.
 */
int turbo_ioloop_stop(turbo_ioloop_t*);

/*
 * Destroys the given IO loop.
 */
int turbo_ioloop_destroy(turbo_ioloop_t**);
