#
#    Copyright (C) 2015 abi <abi@singiro.com>
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

include "common.pyx"

import socket
import time
from threading import Lock

cdef int64_t PUSH_COMMAND = 0
cdef int64_t POP_COMMAND = 1

cdef class TurboWQueue:
    """
    It is a mirror of server queue in client side. When push or pop calls it sends or recieves data to or from server.

    Example:
    >>> client = TurboClient('tcp://127.0.0.1:33444')
    >>> q = client.declare_queue('test')
    >>> q.push('topic', 'hello')
    >>> print q.pop('topic', 1)
    """
    def __cinit__(self, TurboClient client, bytes name):
        self.client = client
        self.name = name

    def push(self, topic, content):
        """
        Pushes a content by the given topic to server-side queue.

        @param str topic: topic key
        @param str content: data
        """
        return self.client.push(self.name, topic, content)

    def pop(self, topic, timeout):
        """
        Pops from the given topic of the server-side queue and return the data.
        If timeout was exceeded, it would return None.

        @param str topic: topic key
        @param int timeout: timeout in seconds

        @rtype: str
        @raise TurboException
        """

        return self.client.pop(self.name, topic, timeout)

cdef class TurboClient:
    def __cinit__(self, bytes url):
        self.socket = None
        self.url = url
        self.lock = Lock()
        self.connect()

    def connect(self):
        if self.socket is None:
            # Creates server socket.
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

            protocol, address, = self.url.split('://')
            host, port, = address.split(':')
            self.socket.connect((host, int(port)))
            self.socket_fd = self.socket.fileno()

    def reconnect(self):
        self.close()
        self.connect()

    def get_queue(self, bytes name):
        return TurboWQueue(self, name)

    def close(self):
        self.socket.close()
        self.socket = None

    cdef int push(self, char* qname, char* topic, bytes data) except -1:
        cdef char endian = detect_host_endian()
        cdef int32_t size = sizeof(int64_t) + sizeof(int16_t) + sizeof(int16_t) + sizeof(int32_t) + len(data)
        cdef char* content = data
        cdef int32_t content_size = len(data)
        cdef turbo_out_stream_t* out = turbo_out_stream_create(self.socket_fd)
        cdef int result = 0
        cdef size_t sent = 0
        with nogil:
            turbo_out_stream_append(out, &PUSH_COMMAND, sizeof(int64_t))
            turbo_out_stream_append_str(out, qname)
            turbo_out_stream_append_str(out, topic)
            turbo_out_stream_append(out, &content_size, sizeof(int32_t))
            turbo_out_stream_append(out, content, content_size)
        with self.lock:
            with nogil:
                sent = turbo_out_stream_send(out)
                if sent <= 0:
                    result = -1

        with nogil:
            turbo_out_stream_destroy(&out)

        return result

    cdef turbo_message_t* _pop(self, char* qname, char* topic, int8_t timeout):
        cdef char endian = detect_host_endian()
        cdef int32_t size = sizeof(int64_t) + sizeof(int16_t) + sizeof(int16_t)+ 1
        cdef turbo_out_stream_t* out = turbo_out_stream_create(self.socket_fd)
        cdef int result = 0
        cdef turbo_in_stream_t* input;

        with nogil:
            turbo_out_stream_append(out, &POP_COMMAND, sizeof(int64_t))
            turbo_out_stream_append_str(out, qname)
            turbo_out_stream_append_str(out, topic)
            turbo_out_stream_append(out, &timeout, sizeof(int8_t))

        with self.lock:
            with nogil:
                if turbo_out_stream_send(out) <= 0:
                    result = -1

        with nogil:
            turbo_out_stream_destroy(&out)

        if result < 0:
            return NULL

        input = turbo_in_stream_create(self.socket_fd)

        with self.lock:
            with nogil:
                turbo_in_stream_recv(input);

        with nogil:
            return turbo_in_stream_read_message(input)

    cdef TurboMessage pop(self, char* qname, char* topic, int8_t timeout):
        cdef turbo_message_t* message
        cdef TurboMessage out
        cdef char* content;

        message = self._pop(qname, topic, timeout)

        if message == NULL:
            return None

        return turbo_message_detach(message)
