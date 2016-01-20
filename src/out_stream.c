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

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "out_stream.h"
#include "utils.h"

#define OVERHEAD sizeof(char) + sizeof(int)

typedef struct 
{
    turbo_out_stream_t parent;
    int fd;
    size_t sent;
    char* body;
} turbo_out_stream_ex_t;

size_t send_all(int fd, const void* src, register size_t size)
{
    register size_t remained = size;
    register char* buffer = (char*)src;
    register ssize_t sent_size = 0;
    while (remained > 0)
    {
        sent_size = send(fd, &buffer[size - remained], remained, 0);
        if(sent_size <= 0)
        {
            if(errno == EAGAIN || errno == EWOULDBLOCK)
            {
                break;
            }
            return -1;
        }
        remained -= sent_size;
    }
    return size - remained;
}

/*
 * Creates a new stream object.
 */
turbo_out_stream_t* turbo_out_stream_create(int fd)
{
    turbo_out_stream_ex_t* stream = (turbo_out_stream_ex_t*)malloc(sizeof(turbo_out_stream_ex_t));
    int32_t* offset;
    stream->fd = fd;
    stream->sent = 0;
    stream->body = (char*)malloc(OVERHEAD);
    stream->body[0] = detect_host_endian();
    offset = (int32_t*)&stream->body[sizeof(char)];
    *offset = 0;
    return (turbo_out_stream_t*)stream;
}

/*
 * Destroys the given stream object.
 */
void turbo_out_stream_destroy(turbo_out_stream_t** base)
{
    turbo_out_stream_ex_t* ptr = (turbo_out_stream_ex_t*)*base;
    if(ptr != NULL)
    {
        if(ptr->body != NULL)
        {
            free(ptr->body);
        }
        free(ptr);
        *base = NULL;
    }
}

/*
 * Sends data to the peer client socket.
 */
size_t turbo_out_stream_send(turbo_out_stream_t* base)
{
    register ssize_t sent_size;
    register turbo_out_stream_ex_t* stream = (turbo_out_stream_ex_t*)base;
    int32_t* offset = (int32_t*)&stream->body[sizeof(char)];
    size_t total = *offset + OVERHEAD;

    if(stream->sent < total)
    {
        sent_size = send_all(stream->fd, 
                             &stream->body[stream->sent],
                             total - stream->sent);
        if(sent_size < 0)
        {
            return sent_size;
        }
        
        stream->sent += sent_size;
        
        if(stream->sent < total)
        {
            return 0;
        }
    }
    
    return stream->sent;
}

/*
 * Returns stream size in bytes.
 */
size_t turbo_out_stream_size(turbo_out_stream_t* base)
{
    register turbo_out_stream_ex_t* stream = (turbo_out_stream_ex_t*)base;
    register int32_t* offset = (int32_t*)&stream->body[sizeof(char)];
    return *offset;
}

/*
 * Appends the given data with size to the stream.
 */
int turbo_out_stream_append(turbo_out_stream_t* base, const void* src, size_t size)
{
    register turbo_out_stream_ex_t* stream = (turbo_out_stream_ex_t*)base;
    register int32_t* offset = (int32_t*)&stream->body[sizeof(char)];
    int32_t new_size = OVERHEAD + (*offset) + size;
    stream->body = (char*)realloc(stream->body, new_size);
    register char* buffer = &stream->body[OVERHEAD];
    offset = (int32_t*)&stream->body[sizeof(char)];
    memcpy(&buffer[*offset], src, size);
    *offset += size;
    return 0;
}

/*
 * Appends a short string to the given stream.
 */
int turbo_out_stream_append_str(turbo_out_stream_t* base, const char* src)
{
    int16_t len = strlen(src);
    if(turbo_out_stream_append(base, &len, sizeof(len)) == -1)
    {
        return -1;
    }
    return turbo_out_stream_append(base, src, len);
}

/*
 * Appends a message to stream.
 */
int turbo_out_stream_append_message(turbo_out_stream_t* base, const turbo_message_t* message)
{
    int32_t size;
    int8_t error;

    if(message == NULL)
    {
        error = 1;
        turbo_out_stream_append(base, &error, sizeof(error));
        return 0;
    }
    error = 0;
    if(turbo_out_stream_append(base, &error, sizeof(error)) == -1)
    {
        return -1;
    }
    if(turbo_out_stream_append(base, &message->address, sizeof(message->address)) == -1)
    {
        return -1;
    }
    size = (int32_t)message->size;
    if(turbo_out_stream_append(base, &size, sizeof(size)) == -1)
    {
        return -1;
    }
    if(turbo_out_stream_append(base, message->content, size) == -1)
    {
        return -1;
    }
    return 0;
}