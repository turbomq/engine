# TurboMQ
**TurboMQ** is a simple message queue system. I hope it is fast enough to merit the name. In our test it could provide and consume millions of messages in a second. But we delegate the final judgment to the developers who use the library.
# Why TurboMQ is developed?
First, I want to explain why a new message queue system is developed. There are many message queue systems available and some of them are popular and stable like **RabbitMQ** or **ZMQ**. The most important reason behind this implementation is that most of message queue systems are designed to handle backend processing like distributing jobs between nodes to process huge amount of data or just complete the remained part of a business transaction. Certainly, TurboMQ can be used to distribute works between nodes. Moreover, it originally designed to support millions of providers and consumers working with millions of queues and topics.

The most close (as queue functionality) system is **Redis**. It has a remarkable IO mechanism to handle network connections. However, it can just utilize one core for one instance. Do we really want to use just one core of for example 8 available cores? Or do we want to configure clustering inside one machine to just use all the available cores?
**ZMQ** is a good library. It is fast, stable and useful for many purposes. Nonetheless, there is a serious problem in topic-based PUB-SUB queues. The consumers (subscribers) has to be connected before providers [(missing message problem solver)](http://zguide.zeromq.org/page:all#Missing-Message-Problem-Solver) otherwise the message is going to be lost.
# Technical information
**TurboMQ** is a python module. To avoid GIL problems, it is developed using pure **C** and **Cython**. It uses its own event loop system. The benefit is that it is a real multi-threaded event loop and can exploit all available cores. The drawback is that it does not support windows. Are the bad news finished? No, kqueue has not implemented yet and it uses (slow) posix POLL in BSD families. Is there any other good news? Yes, windows and kqueue support is going to be implemented very soon.
# Installation
Installation is easy. You need to download or clone it and then type the python magic:
>sudo python setup.py install
# Usage
# Future
