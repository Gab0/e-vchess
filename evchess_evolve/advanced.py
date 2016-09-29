#!/usr/bin/python

import random
from deap import creator, base, tools, algorithms
import copy

from evchess_evolve.core import *
from evchess_evolve.std_parameters import *


def RectifyEOL(line, pos):
    if pos:
        if not line[-1] == '\n':
            return line + '\n'

    else:
        if line[-1] == '\n':
            return line[:-1]

    return line


def retrieve_machinelist():

    machinelist = "%s/machines.list" % machine_dir
    if not os.path.isfile(machinelist):
        print("ERROR: population list file not found.")

    Fo = open(machinelist, 'r')
    mLIST = Fo.readlines()

    for i in range(len(mLIST)):
        mLIST[i] = RectifyEOL(mLIST[i], 0)

    print("lenght of list file: %i" % len(mLIST))
    Fo.close()

    return mLIST


def set_machinelist(mLIST):

    machinelist = "%s/machines.list" % machine_dir

    Fo = open(machinelist, 'w+')

    for M in mLIST:
        M = RectifyEOL(M, 1)
        Fo.write(M)


def load_ADVpopulation():
    population = []
    ELOlist = []
    reference = STDPARAMETERS()
    print("lenght of reference machine: %i" % len(reference))

    mLIST = retrieve_machinelist()

    for file in mLIST:
        ind = []
        file = RectifyEOL(file, 0)

        Machine = open("%s/%s" % (machine_dir, file))

        for line in Machine.readlines():
            try:
                for R in reference:
                    if R.name in line:
                        line = line.split(' =  ')
                        try:
                            ind.append(float(line[1]))
                        except IndexError:
                            print("ERROR|%s" % line)
                            ind.append(0)

                if "stat_elo" in line:
                    line = line.split(' =  ')
                    ind.append(int(line[1]))
            except IndexError:
                print("ERROR|%s" % line)
                ind.append(0)

        if len(ind) == len(reference) + 1:
            population.append(ind)

    print(population)
    for I in range(len(population)):
        population[I] = creator.Individual(population[I])
        #population[I].fitness.values = (ELOlist[I],3.0)
        # ADVpopulation.append(IND)
    print(population)
    return population


def save_ADVpopulation(population):
    reference = STDPARAMETERS()

    mLIST = retrieve_machinelist()

    if len(mLIST) == len(population):
        print("good")
    while len(mLIST) < len(population):
        mLIST.append(NewMacName())
    while len(mLIST) > len(population):
        mLIST.pop(-1)

    for M in range(len(mLIST)):
        Fo = open("%s/%s" % (machine_dir, mLIST[M]), 'w+')
        for A in range(len(reference)):
            Fo.write("%s = %f\n" % (reference[A].name, population[M][A]))
        Fo.write("stat_elo = %i" % population[M][-1])

        Fo.close()


def Eval(individual):
    return (individual[-1] / 1000, 3.0)


def setfitness(individual):
    return (0, individual.ELO / 3000)




def initIndividual(ind):
    IND = creator.Individual(ind)

    return IND


toolbox = base.Toolbox()
toolbox.register("copy", copy.deepcopy)
toolbox.register("mate", tools.cxTwoPoint)
toolbox.register("evaluate", Eval)
toolbox.register("mutate", tools.mutFlipBit, indpb=0.05)
toolbox.register("select", tools.selTournament, tournsize=3)
toolbox.register("machine", initIndividual, machine)


def evolve_machines(POP):
    population = POP

    offspring = algorithms.varAnd(population, toolbox, cxpb=0.5, mutpb=0.1)

    fits = toolbox.map(toolbox.evaluate, population)
    for fit, ind in zip(fits, population):
        ind.fitness = fit
    population = toolbox.select(population, k=len(population))
    top10 = tools.selBest(population, k=10)

    return population


def ADVmanagement():
    creator.create("FitnessMax", base.Fitness, weights=(1.0,))
    creator.create("Individual", list, fitness=creator.FitnessMax)
    
    population = loadmachines()
    for IND in population:
        IND.fitness=creator.FitnessMax
        print("%s|%i" % (IND.filename, IND.ELO))
        IND.fitness.values=(IND.ELO,)
        print(IND.fitness.values)
    #print(population)
    #after = evolve_machines(population)
    #print(after)

    #save_ADVpopulation(after)
    #return after
