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

#include "ioloop.h"

typedef struct 
{
} turbo_command_t;

typedef int (*handle_func_t)(turbo_command_t* command, turbo_remote_client_t* client);

/*
 * Creates an instance of command and returns it.
 */
turbo_command_t* turbo_command_create(int64_t fixed_key, handle_func_t handle_func, void* context);

/*
 * Returns fixed key of the given command.
 */
int turbo_command_get_fixed_key(turbo_command_t* command);

/*
 * Sets dynamic key of the given command.
 */
int turbo_command_set_key(turbo_command_t* command, int64_t key);

/*
 * Returns dynamic key of the given command.
 */
int64_t turbo_command_get_key(turbo_command_t* command);

/*
 * Returns the given context which was passed to the command in creation.
 */
void* turbo_command_get_context(turbo_command_t* command);

/*
 * Executes the command and if it is required is going to return output stream.
 */
int turbo_command_handle(turbo_command_t* command, turbo_remote_client_t* client);

/*
 * Destroys the given command.
 */
int turbo_command_destroy(turbo_command_t** command);
