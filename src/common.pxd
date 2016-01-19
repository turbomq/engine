include "common.pxi"

cdef class TurboMessage:
    def __cinit__(self, time_t creation_date, uint64_t address, bytes content):
        self.content = content
        self.address = bytes('127.0.0.1')
        self.creation_date = creation_date
