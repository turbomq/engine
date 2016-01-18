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

#include <stdlib.h>
#include <string.h>
#include "message.h"

/*
 * Creates a new message using the given stream as content.
 */
turbo_message_t* turbo_message_create_ex(uint64_t address, void* data, size_t size)
{
    turbo_message_t* message = (turbo_message_t*)malloc(sizeof(turbo_message_t));
    message->address = address;
    message->creation_date = time(NULL);
    message->size = size;
    message->content = data;
    return message;
}

/*
 * Creates a new message using the given string as content.
 */
turbo_message_t* turbo_message_create(const char* content)
{
    return turbo_message_create_ex(0, strdup(content), strlen(content) + 1);
}

/*
 * Destroys the given message.
 */
void turbo_message_destroy(turbo_message_t** message)
{
    turbo_message_t* ptr = *message;
    if(ptr != NULL)
    {
        if(ptr->size > 0 && ptr->content != NULL)
        {
            free(ptr->content);
        }
        free(ptr);
        *message = NULL;
    }
}