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

long_description = """
TurboMQ
=======

**TurboMQ** is a simple message queue system. I hope it is fast enough
to merit the name. In our test it could provide and consume millions of
messages in a second. But we delegate the final judgment to the
developers who use the library. Consider that , currently, it is too
experimental and there will be dramatical change in both functionality
and protocols.

Why TurboMQ is developed?
=========================

First, I want to explain why a new message queue system is developed.
There are many message queue systems available and some of them are
popular and stable like **RabbitMQ** or **ZMQ**. The most important
reason behind this implementation is that most of message queue systems
are designed to handle backend processing like distributing jobs between
nodes to process huge amount of data or just complete the remained part
of a business transaction. Certainly, TurboMQ can be used to distribute
works between nodes. Moreover, it originally designed to support
millions of providers and consumers working with millions of queues and
topics.

The most close (as queue functionality) system is **Redis**. It has a
remarkable IO mechanism to handle network connections. However, it can
just utilize one core for one instance. Do we really want to use just
one core of for example 8 available cores? Or do we want to configure
clustering inside one machine to just use all available cores?

**ZMQ** is a good library. It is fast, stable and useful for many
purposes. Nonetheless, there is a serious problem in topic-based PUB-SUB
queues. The consumers (subscribers) has to be connected before providers
`(missing message problem solver)`_ otherwise the message is going to be
lost.

Technical information
=====================

**TurboMQ** is a python module. To avoid GIL problems, it is developed
using pure **C** and **Cython**. It uses its own event loop system. The
benefit is that it is a real multi-threaded event loop and can exploit
all available cores. The drawback is that it does not support windows.
Are the bad news finished? No, kqueue has not implemented yet and it
uses (slow) posix POLL in BSD families. Is there any other good news?
Yes, windows and kqueue support is going to be implemented very soon.

Installation
============

Installation is easy. The package can be installed by pip:

$ sudo pip install turbomq

Alternatively, you can download it or clone it directly from github and then type the python magic:

$ sudo python setup.py install

Usage
============

To use **TurboMQ** just import and run the server. The following code runs a server for 10 minutes.

.. code-block:: python
    from turbomq import TurboEngine
    import time

    # You can pass the thread count as a second parameter.
    # Otherwise, it will automatically selects 4 threads per core.
    e = TurboEngine('tcp://127.0.0.1:33444')

    e.run()
    # "run" method will not block the main thread.
    # So you need to simply wait or run your own loop as you want.
    time.sleep(10.0 * 60)

    # "stop" method just shuts TCP sockets down.
    e.stop()

    # After destroy all resources will be freed.
    # Then you can not use this instance anymore.
    e.destroy()

This code sends a message to server and receives it again.

.. code-block:: python
    from turbomq import TurboClient

    # Connects to the server.
    c = TurboClient('tcp://127.0.0.1:33444')

    # Creates a mirror queue in client side.
    q = c.get_queue('test')

    # Both topic key and data is mandatory in push.
    q.push('hello', 'turbo')

    # In pop you need to determine a timeout.
    # So this will wait two seconds. If timeout is exceeded, it will return None.
    print(q.pop('hello', 2))
"""

setup(
    name='turbomq',
    ext_modules=cythonize(extensions),
    version='0.1.7',
    description='TurboMQ - Message Queue System',
    long_description=long_description,
    author='Abi M.Sangarab',
    author_email='abi@singiro.com',
    url='https://github.com/turbomq/engine',
    download_url='https://github.com/turbomq/engine/archive/0.1.7.tar.gz',
    classifiers=[
    'Development Status :: 3 - Alpha',
    'Programming Language :: Python :: 2.7',
    'Programming Language :: C'
    ],
    keywords='turbomq message queue amqp',
    license='GNU General Public',
    install_requires=[
      'cython',
    ],
    include_package_data=True,
    zip_safe=False
)
