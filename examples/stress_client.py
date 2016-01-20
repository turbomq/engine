from turbomq import TurboClient
from threading import Thread
import time

thread_count = 64
message_size = 512
message_count_per_thread = 25000

def send(count, size):
    c = TurboClient('tcp://127.0.0.1:33444')
    q = c.get_queue('stress')

    message = 'M' * size;

    for i in xrange(count):
        q.push('in', message)

c = TurboClient('tcp://127.0.0.1:33444')
q = c.get_queue('stress')


start_time = time.time()
threads = []
for i in xrange(thread_count):
    t = Thread(target=send, args=(message_count_per_thread, message_size))
    t.start()
    threads.append(t)

for t in threads:
    t.join()

total_message_count = message_count_per_thread * thread_count
elapsed_time = time.time() - start_time
tps = total_message_count / elapsed_time

print('TPS:{}'.format(tps))

q.push('in', 'shutdown-{}'.format(total_message_count))
