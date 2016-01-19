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

include "common.pxi"

cdef class TurboQueue
cdef class TurboEngine

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
