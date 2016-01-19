#
#    Copyright (C) 2015 abi <abisxir@gmail.com>
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.

include "common.pxi"

cdef class TurboClient:
    cdef object socket
    cdef dict queues
    cdef TurboWLock lock
    cdef public bytes url
    cdef int socket_fd

    cdef int push(self, char* qname, char* topic, bytes data) except -1
    cdef TurboMessage pop(self, char* qname, char* topic, int8_t timeout)
    cdef turbo_message_t* _pop(self, char* qname, char* topic, int8_t timeout)

cdef class TurboWQueue:
    cdef TurboClient client
    cdef bytes name
