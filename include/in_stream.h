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

#include <stdlib.h>
#include "message.h"

typedef struct
{
} turbo_in_stream_t;

/*
 * Creates a new stream object.
 */
turbo_in_stream_t* turbo_in_stream_create(int fd);

/*
 * Destroys the given stream object.
 */
void turbo_in_stream_destroy(turbo_in_stream_t** stream);

/*
 * Receives data from the peer socket.
 */
size_t turbo_in_stream_recv(turbo_in_stream_t* stream);

/*
 * Returns stream size in bytes.
 */
size_t turbo_in_stream_size(turbo_in_stream_t* stream);

/*
 * Reads a specified amount of data from stream.
 */
int turbo_in_stream_read(turbo_in_stream_t* stream, void* dest, size_t size);

/*
 * Reads a short string from stream.
 */
int turbo_in_stream_read_str(turbo_in_stream_t* stream, char* dest);

/*
 * Reads a 16 bit int from stream.
 */
int turbo_in_stream_read_int16(turbo_in_stream_t* stream, int16_t* dest);

/*
 * Reads a 32 bit int from stream.
 */
int turbo_in_stream_read_int32(turbo_in_stream_t* stream, int32_t* dest);

/*
 * Reads a 64 bit int from stream.
 */
int turbo_in_stream_read_int64(turbo_in_stream_t* stream, int64_t* dest);

/*
 * Reads a message from stream
 */
turbo_message_t* turbo_in_stream_read_message(turbo_in_stream_t* stream);
