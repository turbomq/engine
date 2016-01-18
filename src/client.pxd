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

from turbomq.lock cimport TurboWLock
from turbomq.message cimport TurboMessage, turbo_message_t

cdef class TurboClient:
    cdef object socket
    cdef dict queues
    cdef TurboWLock lock
    cdef public bytes url
    cdef int socket_fd
    cdef public int64_t PUSH_COMMAND
    cdef public int64_t POP_COMMAND
    
    cdef int push(self, char* qname, char* topic, bytes data) except -1
    cdef TurboMessage pop(self, char* qname, char* topic, int8_t timeout)
    cdef int _pop(self, char* qname, char* topic, int8_t timeout, turbo_message_t* message)

cdef class TurboWQueue:
    cdef TurboClient client
    cdef bytes name