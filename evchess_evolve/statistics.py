#!/bin/python

from evchess_evolve.core import loadmachines


def showBestParameterValues(DIR="machines"):
    pop = loadmachines(DIR=DIR)
    VALUES = {}
    for I in pop:
        for P in I.PARAMETERS:
            VNAME = "%s:    %.2f" % (P.name, P.value)
            if not VNAME in VALUES.keys():
                VALUES[VNAME] = [0,0]
            VALUES[VNAME][0] += I.ELO
            VALUES[VNAME][1] += 1

    TEXT = []
    for K in VALUES.keys():
        MEDIAN = VALUES[K][0]/VALUES[K][1]
        TEXT += ["%s = %.0f [%i]" % (K, MEDIAN, VALUES[K][1]) ]

    TEXT = sorted(TEXT)
    TEXT = "\n".join(TEXT)
    return TEXT
    
            

