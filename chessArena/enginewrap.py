#!/bin/python
from subprocess import Popen, PIPE, call

from fcntl import fcntl, F_GETFL, F_SETFL
from os import O_NONBLOCK, read, system



class Engine():
    def __init__(self, Arguments):
        self.engine = Popen(Arguments, stdin=PIPE, stdout=PIPE)
        
        flags = fcntl(self.engine.stdout, F_GETFL)
        fcntl(self.engine.stdout, F_SETFL, flags | O_NONBLOCK)
        
    def send(self, data):
        self.engine.stdin.write(bytearray("%s\n" % data, "utf-8"))
        self.engine.stdin.flush()

    def receive(self, method="lines"):

        if method == "lines":
            data = self.engine.stdout.readlines()
            data = [ x.decode('utf-8', 'ignore') for x in data ]
        else:
            data = self.engine.stdout.read().decode('utf-8', 'ignore')
            
        self.engine.stdout.flush()
        return data


    def readMove(self, data = None):
        if not data:
            data = self.receive()
            
        for line in data:
            if not "move" in line:
                continue
            line = line.replace('\n', '').split(" ")
            P = line.index("move")
            return line[P+1]

        return None
        
    def destroy(self):
        call(['kill', '-9', str(self.engine.pid)])

    def __del__(self):
        self.destroy()
        self.engine = None
