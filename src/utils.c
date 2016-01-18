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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <endian.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "utils.h"

char detect_host_endian(void)
{
    if(be32toh(1) == 1)
    {
        return BIG_ENDIANNESS;
    }
    return LITTLE_ENDIANNESS;
}

void print_system_error(void)
{
    char buf[1024];
    if(errno != 0)
    {
        memset(buf, 0, sizeof(buf));
        perror(buf);
        printf("System error:%d:%s\n", errno, buf);
    }
}

int16_t convert_int16(char endian, int16_t value)
{
    if(endian == BIG_ENDIANNESS)
    {
        return be16toh(value);
    }
    return le16toh(value);
}

int32_t convert_int32(char endian, int32_t value)
{
    if(endian == BIG_ENDIANNESS)
    {
        return be32toh(value);
    }
    return le32toh(value);
}

int64_t convert_int64(char endian, int64_t value)
{
    if(endian == BIG_ENDIANNESS)
    {
        return be64toh((uint64_t)value);
    }
    return le64toh((uint64_t)value);
}

int make_socket_non_blocking(int fd)
{
    int flags;

    flags = fcntl(fd, F_GETFL, NULL);

    if(flags == -1)
    {
        return -1;
    }

    flags |= O_NONBLOCK;

    return fcntl(fd, F_SETFL, flags);
}

int create_socket(const char* host, int port)
{
    struct sockaddr_in server_addr;
    int socketfd;
    int optval = 1;
    

    /*
     * Create server socket. Specify the nonblocking socket option.
     *
     */
    socketfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof optval);

    if(socketfd == -1)
    {
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(host);

    /*
     * Bind the server socket to the required ip-address and port.
     *
     */
    if(bind(socketfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
    {
        return -1;
    }

    /*
     * Mark the server socket has a socket that will be used to .
     * accept incoming connections.
     */
    if(listen(socketfd, 5) == -1)
    {
        return -1;
    }
    
    return socketfd;
}
