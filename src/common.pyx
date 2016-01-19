#    Copyright (C) 2016 abi <abisxir@gmail.com>
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


cdef class TurboMessage:
    """
    It contains message data like creation date, address and the content.

    Example:
    >>> message = queue.pop('topic', 1)
    >>> print message.creation_date, message.address, message.content
    """
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
