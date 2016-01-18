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

from distutils.core import setup
from Cython.Build import cythonize
from distutils.extension import Extension
import platform
import os

sources = ['src/turbomq.pyx',
           'src/fifo.c',
           'src/engine.c', 
           'src/queue.c', 
           'src/linked_list.c', 
           'src/utils.c', 
           'src/hashmap.c',
           'src/in_stream.c',
           'src/out_stream.c',
           'src/message.c',
           'src/array_list.c',
           'src/command.c']

system = platform.system()

if system == 'Linux':
    sources.append('src/ioloop_linux.c')
elif os.name == 'posix':
    sources.append('src/ioloop_posix.c')
else:
    print 'Unsupported OS :('

extensions = [
    Extension('turbomq', 
              sources, 
              include_dirs=['./include'])
]

setup(
    name = 'TurboMQ',
    ext_modules = cythonize(extensions),
)
