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

#include <stdlib.h>
#include <stdint.h>
#include <time.h>

typedef struct
{
    time_t creation_date;
    uint64_t address;
    int32_t size;
    void* content;
} turbo_message_t;

/*
 * Creates a new message using the given stream as content.
 */
turbo_message_t* turbo_message_create_ex(uint64_t address, void* data, size_t size);

/*
 * Creates a new message using the given string as content.
 */
turbo_message_t* turbo_message_create(const char* content);

/*
 * Destroys the given message.
 */
void turbo_message_destroy(turbo_message_t** message);
