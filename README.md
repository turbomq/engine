# TurboMQ
**TurboMQ** is a simple message queue system. I hope it is fast enough to merit the name. In our test it could provide and consume millions of messages in a second. But we delegate the final judgment to the developers who use the library. Consider that , currently, it is too experimental and there will be dramatical change in both functionality and protocols.

# Why TurboMQ is developed?
First, I want to explain why a new message queue system is developed. There are many message queue systems available and some of them are popular and stable like **RabbitMQ** or **ZMQ**. The most important reason behind this implementation is that most of message queue systems are designed to handle backend processing like distributing jobs between nodes to process huge amount of data or just complete the remained part of a business transaction. Certainly, TurboMQ can be used to distribute works between nodes. Moreover, it originally designed to support millions of providers and consumers working with millions of queues and topics.

The most close (as queue functionality) system is **Redis**. It has a remarkable IO mechanism to handle network connections. However, it can just utilize one core for one instance. Do we really want to use just one core of for example 8 available cores? Or do we want to configure clustering inside one machine to just use all available cores?

**ZMQ** is a good library. It is fast, stable and useful for many purposes. Nonetheless, there is a serious problem in topic-based PUB-SUB queues. The consumers (subscribers) has to be connected before providers [(missing message problem solver)](http://zguide.zeromq.org/page:all#Missing-Message-Problem-Solver) otherwise the message is going to be lost.

# Technical information
**TurboMQ** is a python module. To avoid GIL problems, it is developed using pure **C** and **Cython**. It uses its own event loop system. The benefit is that it is a real multi-threaded event loop and can exploit all available cores. The drawback is that it does not support windows. Are the bad news finished? No, kqueue has not implemented yet and it uses (slow) posix POLL in BSD families. Is there any other good news? Yes, windows and kqueue support is going to be implemented very soon.

# Installation
Installation is easy. The package can be installed by pip:

```bash
$ sudo pip install turbomq
```
 

You need to download or clone it and then type the python magic:

```bash
$ sudo python setup.py install
```

# Usage
To use **TurboMQ** just import and run the server. The following code runs a server for 10 minutes.

```python
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
```

This code sends a message to server and receives it again.

```python
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
```

It is possible to produce message in client-side and consume it in server-side and vice versa:

**Server code:**
```python
from turbomq import TurboEngine
import time

e = TurboEngine('tcp://127.0.0.1:33444')

# Makes engine ready to serve to remote clients.
e.run()

# Creates a queue in server side.
q = e.get_queue('test')

# Waits to consume client commands.
while True:
    # Waits a second for message.
    m = q.pop('hello', 1)
    if m is not None:
        print('Client says:'+ m.content)
        # Puts a new message for client in another topic.
        q.push('hello', 'Hi Client.')
        break

# Waits for example 2.0 seconds.
time.sleep(2.0)

# Cleanups and shuts engine down.
e.stop()
e.destroy()
```

**Client code:**
```python
from turbomq import TurboClient

# Connects to the server.
c = TurboClient('tcp://127.0.0.1:33444')

# Creates a mirror queue in client side.
q = c.get_queue('test')

# Sends the message to server.
q.push('hello', 'Hi Server.')

# Waits one seconds to pop server message
m = q.pop('hello', 1)
print('Server says:' + m.content)
```

# Future
First of all, I will fix bugs to make protocol and engine stable. The second step is to implement client libraries for other programming languages. Then we are going to extend the functionality and features.
