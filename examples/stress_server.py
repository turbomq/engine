from turbomq import TurboEngine
from threading import Thread
import time

def process(engine):
    q = engine.get_queue('stress')
    total_bytes = 0
    total_messages = 0
    start_time = 0
    unit = (1.0 / (1024 * 1024))
    total_sent_messages = 0

    while True:
        message = q.pop('in', 60)
        if message is not None:
            data = message.content
            if start_time == 0:
                start_time = time.time()
            if data.startswith('shutdown'):
                total_sent_messages = int(data.split('-')[1])
                break
            total_messages += 1
            total_bytes += len(data)

    elapsed_time = time.time() - start_time
    tps = total_messages / elapsed_time
    throughput = total_bytes / elapsed_time

    print('Total sent messages:     {}'.format(total_sent_messages))
    print('Total received messages: {}'.format(total_messages))
    print('Error:                   {}\n'.format(total_sent_messages - total_messages))

    print('Elapsed time:            {:.3f}s'.format(elapsed_time))
    print('Total transferred data:  {:.1f} MB'.format(unit * total_bytes))
    print('TPS:                     {:.0f} messages/second'.format(tps))
    print('Throughput:              {:.1f} mbps'.format(unit * throughput))

e = TurboEngine('tcp://127.0.0.1:33444')
e.run()
process(e)
