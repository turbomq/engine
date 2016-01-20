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

#include "command.h"

typedef struct 
{
    turbo_command_t parent;
    int64_t fixed_key;
    int64_t key;
    handle_func_t handle_func;
    void* context;
} turbo_command_ex_t;

/*
 * Creates an instance of command and returns it.
 */
turbo_command_t* turbo_command_create(int64_t fixed_key, handle_func_t handle_func, void* context)
{
    turbo_command_ex_t* command = (turbo_command_ex_t*)malloc(sizeof(turbo_command_ex_t));
    if(command == NULL)
    {
        return NULL;
    }
    command->fixed_key = fixed_key;
    command->key = fixed_key;
    command->handle_func = handle_func;
    command->context = context;
    return (turbo_command_t*)command;
}

/*
 * Returns fixed key of the given command.
 */
int turbo_command_get_fixed_key(turbo_command_t* base)
{
    turbo_command_ex_t* command = (turbo_command_ex_t*)base;
    return command->fixed_key;
}

/*
 * Sets dynamic key of the given command.
 */
int turbo_command_set_key(turbo_command_t* base, int64_t key)
{
    turbo_command_ex_t* command = (turbo_command_ex_t*)base;
    command->key = key;
    return 0;
}

/*
 * Returns dynamic key of the given command.
 */
int64_t turbo_command_get_key(turbo_command_t* base)
{
    turbo_command_ex_t* command = (turbo_command_ex_t*)base;
    return command->key;
}

/*
 * Returns the given context which was passed to the command in creation.
 */
void* turbo_command_get_context(turbo_command_t* base)
{
    turbo_command_ex_t* command = (turbo_command_ex_t*)base;
    return command->context;
}

/*
 * Executes the command and if it is required is going to return output stream.
 */
int turbo_command_handle(turbo_command_t* base, turbo_remote_client_t* client)
{
    turbo_command_ex_t* command = (turbo_command_ex_t*)base;
    return command->handle_func(base, client);
}

/*
 * Destroys the given command.
 */
int turbo_command_destroy(turbo_command_t** base)
{
    turbo_command_ex_t* command = (turbo_command_ex_t*)*base;
    if(command != NULL)
    {
        free(command);
        *base = NULL;
    }
    return 0;
}
