#!/bin/python

from copy import deepcopy
from random import choice, gauss, randrange
from evchess_evolve.core import machine

def crossover(pool):
    num_breaking_points = abs(round(gauss(3, 0.8)))

    newChromosome = ""
    lastBP =0 
    for k in range(num_breaking_points):
        
        nextBP = randrange(lastBP, len(pool[0]))
        if lastBP >= nextBP:
            break
        wInterval = [lastBP, nextBP]
        yInterval = [ x + round(gauss(0,1)) for x in wInterval ]

        one = choice(pool)
        two = choice(pool)
        Buffer = one[wInterval[0]:wInterval[1]]
        one = one[:wInterval[0]] +\
              two[yInterval[0]:yInterval[1]]+\
              one[wInterval[1]:]
        two = two[:yInterval[0]] +\
              one[wInterval[0]:wInterval[1]] +\
              two[yInterval[1]:]
        


def mate(parents, ploidy=2):
    #child = machine(NewMacName(Tail="#", ID=""))
    child=  machine("oias")

    chromossomepool = [ deepcopy(choice(P.Chromosomes)) for P in parents if P.Chromosomes ]

    crossover(chromossomepool)
    
    for k in range(2):
        poolSize = [x for x in range(len(chromossomepool))]
        if not poolSize:
            break
        child.Chromosomes = [ chromossomepool.pop(choice(poolSize))\
                              for n in range(ploidy)]

    child.readOwnChromosomes()

    return child


                            
