from turbomq import TurboEngine
from threading import Thread

def process(engine):
    q = engine.get_queue('world')
    while True:
        message = q.pop('hello', 60)
        if message is not None:
            name = message.content
            print(name + ' said hello :)')
            q.push(name, 'hello ' + name)
            print('Greetings backed to:' + name)
        else:
            print('Nobody said hello :(')

e = TurboEngine('tcp://127.0.0.1:33444')
e.run()
process(e)
