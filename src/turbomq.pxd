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

cdef class TurboQueue
cdef class TurboEngine
cdef class TurboMessage

cdef extern from "time.h" nogil:
    ctypedef long time_t
    
cdef extern from "string.h" nogil:
    void *memcpy(void *dest, void *src, size_t n)

cdef extern from "stdlib.h" nogil:
    ctypedef unsigned long size_t
    void free(void *ptr)
    void *malloc(size_t size)
    void perror(const char*)

cdef extern from "stdint.h" nogil:
    ctypedef unsigned long long uint64_t
    ctypedef int int32_t
    #ctypedef unsigned short uint16_t
    #ctypedef unsigned char uint8_t

cdef extern from "message.h" nogil:
    ctypedef struct turbo_message_t:
        time_t creation_date
        uint64_t address
        int32_t size
        void* content
    
    turbo_message_t* turbo_message_create_ex(uint64_t address, void* data, size_t size)
    turbo_message_t* turbo_message_create(const char* content)
    void turbo_message_destroy(turbo_message_t** message)

cdef class TurboMessage:
    cdef public bytes content
    cdef public bytes address
    cdef public object creation_date

cdef extern from "queue.h" nogil:
    ctypedef struct turbo_queue_t:
        pass

    int turbo_queue_push(turbo_queue_t* queue, const char* topic, turbo_message_t* message)
    turbo_message_t* turbo_queue_pop(turbo_queue_t* queue, const char* topic, int timeout)
    #turbo_fifo_t* turbo_queue_remove_topic(turbo_queue_t* queue, const char* topic)
    
cdef extern from "engine.h" nogil:
    ctypedef struct turbo_engine_t:
        pass

    turbo_engine_t* turbo_engine_create(const char* protocol, const char* host, int port, int num_threads)
    int turbo_engine_run(turbo_engine_t* engine)
    int turbo_engine_stop(turbo_engine_t* engine)
    void turbo_engine_destroy(turbo_engine_t** engine)
    turbo_queue_t* turbo_engine_get_queue(turbo_engine_t* egnine, const char* name)

cdef class TurboEngine:
    cdef turbo_engine_t* handle
    
    cdef turbo_queue_t* _get_queue(self, bytes name)

cdef class TurboQueue:
    cdef turbo_queue_t* handle
    
