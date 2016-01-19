from turbomq import TurboClient

client = TurboClient('tcp://127.0.0.1:33444')
q = client.get_queue('world')
q.push('hello', 'turbo')
print('Greetings sent to server.')
r = q.pop('turbo', 1)
if r is not None:
    print('Sever said:' + r.content + ' :)')
