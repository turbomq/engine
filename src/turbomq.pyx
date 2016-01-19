#    Copyright (C) 2015 abi <abisxir@gmail.com>
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.

include "client.pyx"

import multiprocessing

class TurboException(Exception):
    pass

class TurboEmptyException(TurboException):
    pass

cdef class TurboEngine:

    def __cinit__(self, bytes host, int port, int num_threads):
        if num_threads <= 0:
            num_threads = multiprocessing.cpu_count() * 4

        self.handle = turbo_engine_create('tcp', host, port, num_threads)

    cdef turbo_queue_t* _get_queue(self, bytes name):
        return turbo_engine_get_queue(self.handle, name)

    def run(self):
        cdef int result
        with nogil:
            result = turbo_engine_run(self.handle)
        return result

    def stop(self):
        with nogil:
            turbo_engine_stop(self.handle)

    def get_queue(self, bytes name):
        return TurboQueue(self, name)

    def destroy(self):
        with nogil:
            turbo_engine_destroy(&self.handle)

cdef class TurboQueue:
    def __cinit__(self, TurboEngine engine, bytes name):
        self.handle = engine._get_queue(name)


    def push(self, bytes topic, bytes content):
        cdef char* temp = content
        cdef size_t size = len(content)
        cdef char* data
        cdef int result
        cdef turbo_message_t* message
        cdef char* topic_cstr = topic
        with nogil:
            data = <char*>malloc(size)
            memcpy(data, temp, size)
            message = turbo_message_create_ex(0, data, size)
            result = turbo_queue_push(self.handle, topic_cstr, message)
            if result == -1:
                turbo_message_destroy(&message)
        if result == -1:
            raise TurboException('Queue push error.')


    def pop(self, bytes topic, int timeout):
        cdef turbo_message_t* message
        cdef bytes content
        cdef char* data
        cdef TurboMessage result
        cdef char* topic_cstr = topic
        with nogil:
            message = turbo_queue_pop(self.handle, topic_cstr, timeout)
        if message == NULL:
            raise TurboEmptyException('Queue is empty for topic [{}].'.format(topic))
        data = <char*>message.content
        content = data[:message.size]
        result = TurboMessage(message.creation_date, message.address, content)
        with nogil:
            turbo_message_destroy(&message)
        return result
