#!/bin/python
import sys

import os

from evchess_evolve.core import populate, setmachines

for ARG in sys.argv:
    if ARG == "populate":
        try:
            Q = int(sys.argv[sys.argv.index(ARG)+1])
        except:
            Q = 16
        if "quadra" in sys.argv:
            MACDIRS = ['_machines_%i' % W for W in range(1,5) ]
        elif "duo" in sys.argv:
            MACDIRS = ['_machines_%i' % W for W in range(1,3) ]
        else:
            MACDIRS = ['machines']

        for MD in MACDIRS:
            if not os.path.isdir(MD):
                os.mkdir(MD)
            if not os.path.isdir("%s/top_machines" % MD):
                os.mkdir("%s/top_machines" % MD)
                
            machines = populate([], Q, 1)
            for M in machines:
                M.DIR = MD
            print("creating %i machines under dir %s." % (len(machines), MD))
            setmachines(machines, MD)
