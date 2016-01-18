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

import socket
import time
from turbomq.lock cimport TurboLock
from turbomq.message cimport TurboMessage, turbo_message_t
from turbomq.transponder cimport InStream, OutStream

cdef class TurboWQueue:
    def __cinit__(self, TurboClient client, bytes name):
        self.client = client
        self.name = name
    
    def push(self, topic, content):
        return self.client.push(self.name, topic, content)
        
    def pop(self, topic, timeout):
        return self.client.pop(self.name, topic, timeout)

cdef class TurboClient:    
    def __cinit__(self, bytes url):
        # Creates server socket.
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        
        host, port, = url[6:].split(':')
        self.socket.connect((host, int(port)))
        self.socket_fd = self.socket.fileno()
        
        self.lock = TurboWLock()
        self.queues = dict()
        
        self.PUSH_COMMAND = 0
        self.POP_COMMAND = 1
        
    def declare_queue(self, bytes name):
        return TurboWQueue(self, name)

    def close(self):
        self.socket.close()
        self.socket = None
        
    cdef int push(self, char* qname, char* topic, bytes data) except -1:
        cdef int32_t size = len(data)
        cdef char* content = data
        cdef OutStream out = OutStream(self.socket_fd)
        with self.lock:
            with nogil:
                out.append(&self.PUSH_COMMAND, sizeof(int64_t))
                out.append_short_string(qname, strlen(qname))
                out.append_short_string(topic, strlen(topic))
                out.append(&size, sizeof(size))
                out.append(content, size)
                return out.flush()
            
    cdef int _pop(self, char* qname, char* topic, int8_t timeout, turbo_message_t* message):
        cdef OutStream out = OutStream(self.socket_fd)
        cdef InStream input
        cdef int result

        with nogil:
            # Begin sending request
            out.append(&self.POP_COMMAND, sizeof(int64_t))
            out.append_short_string(qname, strlen(qname))
            out.append_short_string(topic, strlen(topic))
            out.append(&timeout, sizeof(timeout))
            if out.flush() == -1:
                return -1

        input = InStream(self.socket_fd)

        with nogil:
            return input.read_message(message)
        

    cdef TurboMessage pop(self, char* qname, char* topic, int8_t timeout):
        cdef turbo_message_t message
        cdef TurboMessage out
        with self.lock:
            if self._pop(qname, topic, timeout, &message) == -1:
                return None

        out = TurboMessage(message.ip, time.time(), message.content, message.size)
        free(message.content)
        return out
        # End receiving response
        
