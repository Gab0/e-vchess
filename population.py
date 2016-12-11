import sys

import os

from evchess_evolve.core import populate, setmachines


if not os.path.isdir("machines"):
    os.mkdir("machines")



for ARG in sys.argv:
    if ARG == "populate":
        try:
            Q = int(sys.argv[ARG+1])
        except:
            Q = 16
        print(Q)
        machines = populate([], Q, 1)
        print(len(machines))
        setmachines(machines, 'machines')
