#!/usr/bin/python

import os
import random
import xml.etree.ElementTree as ET
import copy

from shutil import copyfile

from evchess_evolve.machine import machine
from evchess_evolve.management import bareDeleteMachine
from evchess_evolve import chromossome
from os import listdir

from chessArena.settings import Settings
settings = Settings()

def populate(population, popsize, Randomize, ID=""):
    NEWINDS = []
    for i in range(popsize):
        NEWINDS.append(machine(NewMacName(ID=ID)))
        
    for I in NEWINDS:
        if Randomize:
            I.randomize()
        population.append(I)

    return population


def loadmachines(DIR=settings.machineDIR):
    population = []
    k = 0
    '''
    machinelist = "%s/machines.list" % DIR
    if not os.path.isfile(machinelist):
        return population
    Fo = open(machinelist, 'r')
    mLIST = Fo.readlines()
    Fo.close()'''
    
    mLIST = listdir(DIR)
    for File in mLIST:
        File = File.strip("\n")

        if File.endswith(".mac"):
            population.append(machine(File, DIR=DIR))
            population[-1].Load()

    return population


def recover_popfromfolder(N):  # Linux only.
    entirety = []
    k = 0
    X = 0
    for file in os.listdir(settings.machineDIR):
        if file.endswith(".mac"):
            X += 1
            Fo = open(settings.machineDIR + '/' + file, "r+")
            entirety.append(machine(file))

            for line in Fo.readlines():
                if line == "\n":
                    continue
                L = line.split()
                if len(L) == 3:
                    L.append(0)

                entirety[k].read(L)

            k += 1
    population = []

    if N > X:
        N = X
    for W in range(N):
        Best = [0, 0]
        for M in range(len(entirety)):
            if entirety[M].ELO > Best[1]:
                Best[1] = entirety[M].ELO
                Best[0] = M

        population.append(copy.deepcopy(entirety[Best[0]]))
        entirety[Best[0]].ELO = 0

    return population


def p100():
    return random.randrange(0, 100)


def mutatemachines(Aggro, population):

    ELOsum = 0
    for IND in population:
        ELOsum += IND.ELO

    averageELO = ELOsum // len(population)

    for i in range(len(population)):
        diff = population[i].ELO - averageELO

        MutateProbabilityDamper = diff // 6

        population[i].mutate(MutateProbabilityDamper, Aggro)

    return population

def setmachines(population, DIR=settings.machineDIR):
    for i in range(len(population)):
        if population[i].DIR != DIR:
            print("overriding %s directory." % population[i].filename)
            population[i].DIR = DIR
        population[i].write()


def deltheworst_clonethebest(population, action, MODlimit, ID=None):
    INFINITE = 6669990001
    POP_SCORETABLE = []
    MEDIUMSCORE = 0
    VALIDPOP = 0
    REMOVED = []
    for k in range(len(population)):
        if population[k].PARAMETERS[0].value == 0:
            POP_SCORETABLE.append(-1)
            continue
        SCORE = population[k].ELO

        POP_SCORETABLE.append(SCORE)
        MEDIUMSCORE += SCORE
        VALIDPOP += 1

    if VALIDPOP > 0:
        MEDIUMSCORE = MEDIUMSCORE / VALIDPOP
        print('mediumscore = %i' % MEDIUMSCORE)
        if action < 0:
            action = -action

            for D in range(action):
                CURRENT_SCORE = [0, INFINITE]
                for k in range(len(population)):
                    if population[k] != 0:
                        if (population[k].ELO > -1):
                            if (population[k].ELO < MEDIUMSCORE * MODlimit):
                                if population[k].ELO < CURRENT_SCORE[1]:
                                    CURRENT_SCORE[0] = k
                                    CURRENT_SCORE[1] = population[k].ELO

                try:
                    delname = population[CURRENT_SCORE[0]].filename
                    print("subject deleted. %s" % delname)
                    bareDeleteMachine(settings.machineDIR, delname)
                except AttributeError:
                    print("ERROR on Delete the Worst/Clone the Best")
                    pass

                population[CURRENT_SCORE[0]] = 0
                population = [x for x in population if x != 0]

        elif action > 0:
            NEWINDS = []
            for k in range(len(POP_SCORETABLE)):
                if (POP_SCORETABLE[k] > MEDIUMSCORE * 1.1):
                    print('subject cloned. ' + population[k].filename)
                    NEWINDS.append(population[k])
                    NEWINDS[-1].filename = NewMacName(ID=ID)

            mutatemachines(6, NEWINDS)
            for I in NEWINDS:
                population.append(I)

    return population


def create_hybrid(population):
    K = random.randrange(len(population))
    K_ = range(population[K].ELO - 20,  population[K].ELO + 20)
    for I in range(len(population)):
        if population[I].ELO in K_:
            if random.randrange(100) < 60:
                CHILD = (machine(NewMacName()))

                for P in range(len(population[I].PARAMETERS)):
                    chance = random.randrange(100)

                    if chance < 50:
                        CHILD.PARAMETERS[P].value = population[
                            I].PARAMETERS[P].value
                    else:
                        CHILD.PARAMETERS[P].value = population[
                            K].PARAMETERS[P].value
                print('new hybrid created. son of %i & %i' % (I, K))
                return CHILD


def select_best_inds(population, NUMBER):
    HoF = [None for x in range(NUMBER)]

    SCORE = 0
    for i in range(NUMBER):
        SCORE = 0
        for individual in population:
            SCR = individual.ELO
            if SCR > SCORE:
                if i and SCR > HoF[i-1].ELO:
                    continue
                if i and individual.filename in [x.filename for x in HoF[:i]]:
                    continue
                HoF[i] = individual
                SCORE = SCR

    HoF = [x for x in HoF if x]

    return HoF


def crossover(population, indexA, indexB):
    pA = random.randrange(len(population[indexA].PARAMETERS))
    pB = random.randrange(len(population[indexB].PARAMETERS))

    Buffer = population[indexA].PARAMETERS[pA].value

    population[indexA].PARAMETERS[pA].value = population[
        indexB].PARAMETERS[pB].value
    population[indexB].PARAMETERS[pB].value = Buffer

    return population


def Mate(individuals, nchild, ID=None):
    Children = []
    for N in range(nchild):
        Child = machine(NewMacName(ID=ID, Tail="#"))
        for P in range(len(Child.PARAMETERS)):
            Child.PARAMETERS[P] = copy.deepcopy(
                random.choice(individuals).PARAMETERS[P])
        Children.append(Child)

    return Children


def replicate_best_inds(population, NUMBER, ID=None):
    TOP = select_best_inds(population, NUMBER)

    for IND in TOP:
        for N in range(NUMBER):
            IND.filename = NewMacName(ID=ID)
            population.append(copy.deepcopy(IND))

    return population


def clone_from_template(ID=None):
    TemplatePool = os.listdir("%s/halloffame" % settings.machineDIR)
    POOL = []

    for line in TemplatePool.readlines():
        if '.mac' in line:
            POOL.append(line.replace('\n', ''))

    X = random.randrange(len(POOL))

    model = open('%s/%s' % (settings.HoFmachineDIR, POOL[X]), 'r')

    CHILD = machine(NewMacName(ID=ID, Tail="&"))
    for line in model.readlines():
        CHILD.read(line)
    CHILD.onTOP = 0

    # for N in range(NUMBER):
    CHILD.mutate(3, 3)

    return CHILD


def NewMacName(Tail="", ID=""):
    # used tail coding:
    # &   stands for machine cloned from hall of fame template.
    # #   stands for machine created by mating.
    letters = ""
    for k in range(3):
        x = random.randrange(65, 91)
        letters += chr(x)

    numbers = random.randrange(0, 9999)
    numbers = str(numbers).zfill(4)
    if ID:
        ID += "_"
    return "%s%s%s%s.mac" % (ID, letters, numbers, Tail)


def IsEqual(model, against):
    EQUAL = True
    for P in range(len(model.PARAMETERS)):
        if not model.PARAMETERS[P].value == against.PARAMETERS[P].value:
            EQUAL = False
    return EQUAL


def EliminateEquals(population, Range, delete=True):
    blacklist = []
    for I in range(Range, len(population)):
        if population[I] == 0:
            continue
        for T in range(I + 1, len(population)):
            if population[T] == 0:
                continue
            if IsEqual(population[I], population[T]):
                blacklist.append(population[T].filename)
                population[T] = 0
    if delete:
        for Name in blacklist:
            bareDeleteMachine(settings.machineDIR, Name)
                                 
    return list(filter((0).__ne__, population))

def EvaluateSimilarityTwoMachines(mac1, mac2):
    TotalScore = 0
    for I in range(len(mac1.PARAMETERS)):
        if mac1.PARAMETERS[I].value > 10:
            continue

        a = mac1.PARAMETERS[I].value
        b = mac2.PARAMETERS[I].value
        TotalScore += abs(a-b)

    return TotalScore

def Triangulate_value(values):
    if len(values) == 0:
        return 0

    BUFFER = 0
    #for value in values: BUFFER+=value

    #value = BUFFER/len(values)

    X = random.randrange(len(values))

    return values[X]


def sendtoHallOfFame(MACHINE):
    MACHINE.DIR = settings.HoFmachineDIR
    print('machine %s sent to hall of fame.' % MACHINE.filename)
    MACHINE.onHoF = 1
    MACHINE.write()


