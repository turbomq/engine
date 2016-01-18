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

#include "message.h"

typedef struct 
{
} turbo_out_stream_t;

/*
 * Creates a new stream object.
 */
turbo_out_stream_t* turbo_out_stream_create(int fd);

/*
 * Destroys the given stream object.
 */
void turbo_out_stream_destroy(turbo_out_stream_t** stream);

/*
 * Sends data the peer socket.
 */
size_t turbo_out_stream_send(turbo_out_stream_t* stream);

/*
 * Returns stream size in bytes.
 */
size_t turbo_out_stream_size(turbo_out_stream_t* stream);

/*
 * Appends the given data using the size to the stream.
 */
int turbo_out_stream_append(turbo_out_stream_t* stream, const void* src, size_t size);

/*
 * Appends a short string to the given stream.
 */
int turbo_out_stream_append_str(turbo_out_stream_t* stream, const char* src);

/*
 * Appends a message to stream.
 */
int turbo_out_stream_append_message(turbo_out_stream_t* base, const turbo_message_t* message);

