#    Copyright (C) 2015 abi <abi@singiro.com>
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

__version__ = '0.1.3'

include "client.pyx"

import multiprocessing


class TurboException(Exception):
    pass


cdef class TurboEngine:
    """
    It is responsible to create queues and manipulate them.

    Example:
    >>> from turbomq import TurboEngine
    >>> egnine = TurboEngine('tcp://127.0.0.1:33444')
    >>> engine.run()
    """

    def __cinit__(self, bytes url, int num_threads=0):
        if num_threads <= 0:
            num_threads = multiprocessing.cpu_count() * 4

        if url.find('://') < 0:
            raise TurboException('Url [{}] is invalid. Example: tcp://host:port'.format(url))
        protocol, address, = url.split('://')

        if address.find(':') < 0:
            raise TurboException('Url [{}] is invalid. Example: tcp://host:port'.format(url))

        host, port, = address.split(':')
        self.handle = turbo_engine_create(protocol, host, int(port), num_threads)

    cdef turbo_queue_t* _get_queue(self, bytes name):
        return turbo_engine_get_queue(self.handle, name)

    def run(self):
        """
        Runs the engine forever. So remote producers and consumers are able to work with queues.
        If the user just wants to use the engine locally then no need to run the engine.
        Consider that "run" does not block the main thread.
        """
        cdef int result
        with nogil:
            result = turbo_engine_run(self.handle)
        return result

    def stop(self):
        """
        Stops the engine and then it is not serves to the remote clients.
        However, the local threads inside the process can use the engine.
        """

        with nogil:
            turbo_engine_stop(self.handle)

    def get_queue(self, bytes name):
        """
        Creates a new or returns an existed queue using the given name.

        @param str name: queue name

        @rtype: TurboQueue
        @return: instance of the created queue
        """

        return TurboQueue(self, name)

    def destroy(self):
        """
        Destroys the engine so all the queues are going to be destroyed and all resources will be freed.
        After that nobody can use this engine.
        """

        with nogil:
            turbo_engine_destroy(&self.handle)

cdef class TurboQueue:
    """
    This class is responsible to push data to the queue or pop from it.

    Example:
    >>> from turbomq import TurboEngine
    >>> egnine = TurboEngine('tcp://127.0.0.1:33444')
    >>> queue = engine.get_queue('test')
    >>> queue.push('topic-0', 'data-0')
    >>> queue.pop('topic-0', 2)
    """

    def __cinit__(self, TurboEngine engine, bytes name):
        self.handle = engine._get_queue(name)

    def push(self, bytes topic, bytes content):
        """
        Pushes a new content into the queue categorizing by the topic.

        @param str topic: topic key
        @param str content: content as bytes
        """

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
        """
        Pops data from the queue categorizing by the given topic. You need to specify timeout in seconds.
        If after the given timeout it could not acquire a data it would return None.

        @param str topic: topic key
        @param int timeout: timeout in seconds

        @rtype: str
        @raise TurboException
        """

        cdef turbo_message_t* message
        cdef bytes content
        cdef char* data
        cdef TurboMessage result
        cdef char* topic_cstr = topic
        with nogil:
            message = turbo_queue_pop(self.handle, topic_cstr, timeout)
        if message == NULL:
            return None
        data = <char*>message.content
        content = data[:message.size]
        result = TurboMessage(message.creation_date, message.address, content)
        with nogil:
            turbo_message_destroy(&message)
        return result
