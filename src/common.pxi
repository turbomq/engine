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
    ctypedef unsigned int uint32_t
    ctypedef unsigned short uint16_t
    ctypedef unsigned char uint8_t
    ctypedef long long int64_t
    ctypedef int int32_t
    ctypedef short int16_t
    ctypedef char int8_t

cdef extern from "message.h" nogil:
    ctypedef struct turbo_message_t:
        time_t creation_date
        uint64_t address
        int32_t size
        void* content

    turbo_message_t* turbo_message_create_ex(uint64_t address, void* data, size_t size)
    turbo_message_t* turbo_message_create(const char* content)
    void turbo_message_destroy(turbo_message_t** message)

cdef extern from "queue.h" nogil:
    ctypedef struct turbo_queue_t:
        pass

    int turbo_queue_push(turbo_queue_t* queue, const char* topic, turbo_message_t* message)
    turbo_message_t* turbo_queue_pop(turbo_queue_t* queue, const char* topic, int timeout)
    #turbo_fifo_t* turbo_queue_remove_topic(turbo_queue_t* queue, const char* topic)

    cdef extern from "utils.h" nogil:
        char detect_host_endian();

cdef extern from "out_stream.h" nogil:
    ctypedef struct turbo_out_stream_t:
        pass

    turbo_out_stream_t* turbo_out_stream_create(int fd);
    void turbo_out_stream_destroy(turbo_out_stream_t** stream);
    size_t turbo_out_stream_send(turbo_out_stream_t* stream);
    size_t turbo_out_stream_size(turbo_out_stream_t* stream);
    int turbo_out_stream_append(turbo_out_stream_t* stream, const void* src, size_t size);
    int turbo_out_stream_append_str(turbo_out_stream_t* stream, const char* src);
    int turbo_out_stream_append_message(turbo_out_stream_t* base, const turbo_message_t* message);

cdef extern from "in_stream.h" nogil:
    ctypedef struct turbo_in_stream_t:
        pass

    turbo_in_stream_t* turbo_in_stream_create(int fd);
    void turbo_in_stream_destroy(turbo_in_stream_t** stream);
    size_t turbo_in_stream_recv(turbo_in_stream_t* stream);
    size_t turbo_in_stream_size(turbo_in_stream_t* stream);
    int turbo_in_stream_read(turbo_in_stream_t* stream, void* dest, size_t size);
    int turbo_in_stream_read_str(turbo_in_stream_t* stream, char* dest);
    int turbo_in_stream_read_int16(turbo_in_stream_t* stream, int16_t* dest);
    int turbo_in_stream_read_int32(turbo_in_stream_t* stream, int32_t* dest);
    int turbo_in_stream_read_int64(turbo_in_stream_t* stream, int64_t* dest);
    turbo_message_t* turbo_in_stream_read_message(turbo_in_stream_t* stream);
