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
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "in_stream.h"
#include "utils.h"

typedef struct 
{
    turbo_in_stream_t parent;
    int fd;
    size_t offset;
    union
    {
        char all[5];
        char endian;
    } header;
    int32_t header_pos;
    int32_t body_pos;
    char* body;
} turbo_in_stream_ex_t;

size_t recv_all(int fd, void* dest, register size_t size)
{
    register size_t remained = size;
    register char* buffer = (char*)dest;
    register ssize_t read_size = 0;
    while (remained > 0)
    {
        read_size = recv(fd, &buffer[size - remained], remained, 0);
        if(read_size <= 0)
        {
            //print_system_error();
            if(read_size == 0)
            {
                return -1;
            }
            if(errno == EAGAIN || errno == EWOULDBLOCK)
            {
                break;
            }
            return -1;
        }
        remained -= read_size;
    }
    return size - remained;
}

/*
 * Creates a new stream object.
 */
turbo_in_stream_t* turbo_in_stream_create(int fd)
{
    turbo_in_stream_ex_t* stream = (turbo_in_stream_ex_t*)malloc(sizeof(turbo_in_stream_ex_t));
    int32_t* size = (int32_t*)&stream->header.all[1];
    stream->fd = fd;
    stream->offset = 0;
    stream->header.endian = 0;
    *size = 0;
    stream->header_pos = 0;
    stream->body_pos = 0;
    stream->body = NULL;
    return (turbo_in_stream_t*)stream;
}

/*
 * Destroys the given stream object.
 */
void turbo_in_stream_destroy(turbo_in_stream_t** base)
{
    turbo_in_stream_ex_t* ptr = (turbo_in_stream_ex_t*)*base;
    if(ptr != NULL)
    {
        if(ptr->body != NULL)
        {
            free(ptr->body);
            ptr->body = NULL;
        }
        free(ptr);
        *base = NULL;
    }
}

/*
 * Receives data from network.
 */
size_t turbo_in_stream_recv(turbo_in_stream_t* base)
{
    register ssize_t read_size;
    register turbo_in_stream_ex_t* stream = (turbo_in_stream_ex_t*)base;
    register int32_t* size = (int32_t*)&stream->header.all[1];
    if(stream->header_pos < sizeof(stream->header.all))
    {
        read_size = recv_all(stream->fd, 
                             &stream->header.all[stream->header_pos],
                             sizeof(stream->header.all) - stream->header_pos);
        if(read_size < 0)
        {
            return read_size;
        }
        stream->header_pos += read_size;
        if(stream->header_pos < sizeof(stream->header.all))
        {
            printf("partial:%ld\n", read_size);
            return -1;
        }
        *size = convert_int32(stream->header.endian, *size);
    }
    if(stream->body == NULL)
    {
        stream->body = (char*)malloc(*size);
        if(stream->body == NULL)
        {
            print_system_error();
            return -1;
        }
    }
    if(stream->body_pos < *size)
    {
        read_size = recv_all(stream->fd, 
                             &stream->body[stream->body_pos], 
                             *size - stream->body_pos);
        if(read_size < 0)
        {
            return -1;
        }
        stream->body_pos += read_size;
        if(stream->body_pos < *size)
        {
            return 0;
        }
    }
    return *size;
}

/*
 * Returns stream size in bytes.
 */
size_t turbo_in_stream_size(turbo_in_stream_t* base)
{
    register turbo_in_stream_ex_t* stream = (turbo_in_stream_ex_t*)base;
    register int32_t* size = (int32_t*)&stream->header.all[1];
    return *size;
}

/*
 * Reads a specified amount of data from stream.
 */
int turbo_in_stream_read(turbo_in_stream_t* base, void* dest, size_t size)
{
    register turbo_in_stream_ex_t* stream = (turbo_in_stream_ex_t*)base;
    register int32_t* total = (int32_t*)&stream->header.all[1];
    if(stream->body_pos != *total)
    {
        return -1;
    }
    if(size + stream->offset > *total)
    {
        return -1;
    }
    memcpy(dest, &stream->body[stream->offset], size);
    stream->offset += size;
    return 0;
}

/*
 * Reads a short string from stream.
 */
int turbo_in_stream_read_str(turbo_in_stream_t* base, char* dest)
{
    int16_t len;
    if(turbo_in_stream_read_int16(base, &len) == -1)
    {
        return -1;
    }
    dest[len] = 0;
    return turbo_in_stream_read(base, dest, len);
}

/*
 * Reads a 16 bit int from stream.
 */
int turbo_in_stream_read_int16(turbo_in_stream_t* base, int16_t* dest)
{
    register turbo_in_stream_ex_t* stream = (turbo_in_stream_ex_t*)base;
    if(turbo_in_stream_read(base, dest, sizeof(int16_t)) == -1)
    {
        return -1;
    }
    *dest = convert_int16(stream->header.endian, *dest);
    return 0;
}

/*
 * Reads a 32 bit int from stream.
 */
int turbo_in_stream_read_int32(turbo_in_stream_t* base, int32_t* dest)
{
    register turbo_in_stream_ex_t* stream = (turbo_in_stream_ex_t*)base;
    if(turbo_in_stream_read(base, dest, sizeof(int32_t)) == -1)
    {
        return -1;
    }
    *dest = convert_int32(stream->header.endian, *dest);
    return 0;
}

/*
 * Reads a 64 bit int from stream.
 */
int turbo_in_stream_read_int64(turbo_in_stream_t* base, int64_t* dest)
{
    register turbo_in_stream_ex_t* stream = (turbo_in_stream_ex_t*)base;
    if(turbo_in_stream_read(base, dest, sizeof(int64_t)) == -1)
    {
        return -1;
    }
    *dest = convert_int64(stream->header.endian, *dest);
    return 0;
}
