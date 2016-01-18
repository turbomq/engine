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

#include <stdio.h>
#include <stdint.h>
#include "message.h"

#define LITTLE_ENDIANNESS 'L'
#define BIG_ENDIANNESS 'B'

int16_t convert_int16(char endian, int16_t value);
int32_t convert_int32(char endian, int32_t value);
int64_t convert_int64(char endian, int64_t value);

char detect_host_endian(void);
void print_system_error(void);

int make_socket_non_blocking(int fd);
int create_socket(const char* host, int port);

