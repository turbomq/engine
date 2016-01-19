

cdef class TurboMessage:
    def __cinit__(self, time_t creation_date, uint64_t address, bytes content):
        self.content = content
        self.address = bytes('127.0.0.1')
        self.creation_date = creation_date

cdef TurboMessage turbo_message_detach(turbo_message_t* message):
    cdef char* data = <char*>message.content
    cdef bytes content = data[:message.size]
    cdef TurboMessage result = TurboMessage(message.creation_date, message.address, content)
    with nogil:
        turbo_message_destroy(&message)
    return result;
