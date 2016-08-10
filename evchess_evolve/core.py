#!/usr/bin/python

import os
import random
import xml.etree.ElementTree as ET
import copy

from shutil import copyfile
#machine directory.
Fdir = "/home/gabs/Desktop/e-vchess/machines"
machine_dir = "/home/gabs/Desktop/e-vchess/machines"

from evchess_evolve.machine import machine


def populate(population,popsize, Randomize):
    NEWINDS = []
    for i in range(popsize):
        NEWINDS.append(machine(str(random.randrange(0,6489))+".mac")) 
    


    for I in NEWINDS:
        if Randomize:
            I.randomize()
        population.append(I)

        
    return population
       
def loadmachines():
        population = []
        k=0

        machinelist = "%s/machines.list" % machine_dir
        if not os.path.isfile(machinelist):
            return population
        Fo = open(machinelist,'r')
        mLIST = Fo.readlines()
        Fo.close()

        for file in mLIST:
            if file[-1] == '\n': file = file[:-1]

            if file.endswith(".mac"):
                Fo = open(machine_dir+'/'+file, "r+")
                population.append(machine(file))
                
                for line in Fo.readlines():
                    if line == "\n": continue
                    L = line.split()
                    if len(L) == 3:
                        L.append(0)

                    population[-1].read(L)


                
                

        return population


def recover_popfromfolder(N):#Linux only.
    entirety = []
    k=0
    X=0
    for file in os.listdir(machine_dir):
        if file.endswith(".mac"):
            X+=1
            Fo = open(machine_dir+'/'+file, "r+")
            entirety.append(machine(file))
            
            for line in Fo.readlines():
                if line == "\n": continue
                L = line.split()
                if len(L) == 3:
                    L.append(0)

                entirety[k].read(L)


            k+=1
    population = []
    
    if N>X: N=X
    for W in range(N):
        Best=[0,0]
        for M in range(len(entirety)):
            if entirety[M].ELO > Best[1]:
                Best[1] = entirety[M].ELO
                Best[0] = M

        population.append(copy.deepcopy(entirety[Best[0]]))
        entirety[Best[0]].ELO = 0

    return population
        
        
def p100():
    return random.randrange(0,100)


def mutatemachines(Aggro, population):

    ELOsum = 0
    for IND in population:
        ELOsum += IND.ELO

    averageELO = ELOsum // len(population)

    for i in range(len(population)):
        diff = population[i].ELO - averageELO
        
        MutateProbabilityDamper = diff//6
        
        population[i].mutate(MutateProbabilityDamper, Aggro)
                


    return population


def setTIMEweight(value):
    standards = []
    currentVAL = []

    standards.append([0.9,0.85,0.9,0.85,0.9,0.85,0.9,0.85,0.9,0.85])
    standards.append([0.9,0.85,0.8,0.75,0.68,0.68,0.75,0.65,0.58,0.55])
    standards.append([0.9,0.85,0.85,0.8,0.8,0.75,0.75,0.7,0.7,0.65])
    standards.append([0.9, 0.85, 0.9, 0.85, 0.81, 0.765, 0.825, 0.789, 0.844, 0.85])
    
    if random.randrange(0,100) < 66:
        currentVAL = value
    else:
        currentVAL = standards[random.randrange(0,len(standards)-1)]


        

    Ximpact = random.randrange(0,10)
    Yimpact = random.randrange(-3,3)


    currentVAL[Ximpact] *= 1+(Yimpact/10)

    Kdist = 0
    for forward in range(Ximpact+1,9):
        currentVAL[forward] *= 1 + (Yimpact/(10+2*Kdist))
        Kdist+=1

    Kdist = 0
    for backward in range(Ximpact-1,0):
        currentVAL[backward] *= 1 + (Yimpact/(10+2*Kdist))
        Kdist+=1
                               
                               


                               

    print("TIMEweight worked on.")


    for Y in range(len(currentVAL)):
        currentVAL[Y] = round(currentVAL[Y], 3)
    
    return currentVAL
        
def dump_all_paramstat(individual):


    individual.dump_parameter_stat()   




def read_param_dump(parameter):
    if not os.path.exists(Fdir + "/paramstats.xml"):
        return ["stats dump not found."]

    DUMP = ["reading " + parameter + ":  W D L G"]
    tree = ET.parse(Fdir + "/paramstats.xml")
    root = tree.getoot()

    for child in root:
        #print(child.tag)
        if child.tag == parameter:
  
            for stat in range(len(child)):
                DUMP.append([child[stat].tag])
                for score in child[stat]:
                    DUMP[stat+1].append(score.text)
                


    return DUMP

        




def setmachines(population):
    for i in range(len(population)):
        population[i].write()

    if os.path.isfile(Fdir+'/machines.list'):
        os.remove(Fdir+'/machines.list')
    Fo = open(Fdir+'/machines.list', "w+")
    for i in range(len(population)):
        Fo.write(population[i].filename+"\n")

    Fo.close





def deltheworst_clonethebest(population, action, MODlimit):
    POP_SCORETABLE=[]
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
        MEDIUMSCORE = MEDIUMSCORE/VALIDPOP
        print('mediumscore = %i' % MEDIUMSCORE)
        if action < 0:
            action = -action
   
            for D in range (action):
                CURRENT_SCORE = [0,66666]
                for k in range(len(population)):
                    if population[k] != 0:
                        if (population[k].ELO > -1):
                            if (population[k].ELO < MEDIUMSCORE*MODlimit):
                                if population[k].ELO < CURRENT_SCORE[1]:
                                    CURRENT_SCORE[0] = k
                                    CURRENT_SCORE[1] = population[k].ELO

                                    
                try:
                    print('subject deleted. ' + population[ CURRENT_SCORE[0] ].filename)
                except AttributeError:
                    print("ERROR on Delete the Worst/Clone the Best")
                    pass
                """try:
                    os.remove(Fdir+'/'+population[CURRENT_SCORE[0]].filename)
                except FileNotFoundError:
                    print("can't find machine file, but it's ok.")"""
                population[CURRENT_SCORE[0]] = 0
                population = [x for x in population if x != 0]



        elif action > 0:
            NEWINDS = []
            for k in range(len(POP_SCORETABLE)):
                if (POP_SCORETABLE[k] > MEDIUMSCORE*1.1):
                    print('subject cloned. ' + population[k].filename)
                    NEWINDS.append(population[k])
                    NEWINDS[-1].filename = NewMacName()

            mutatemachines(6,NEWINDS)
            for I in NEWINDS:
                population.append(I)

            
                
    return population

def create_hybrid(population):
    K = random.randrange(len(population))
    K_ = range(population[K].ELO-20,  population[K].ELO+20) 
    for I in range(len(population)):
        if population[I].ELO in K_:
            if random.randrange(100) < 60:
                CHILD = (machine(NewMacName()))

                for P in range(len(population[I].PARAMETERS)):
                    chance = random.randrange(100)
                    
                    if chance < 50:
                        CHILD.PARAMETERS[P].value = population[I].PARAMETERS[P].value
                    else:
                        CHILD.PARAMETERS[P].value = population[K].PARAMETERS[P].value
                print('new hybrid created. son of %i & %i' % (I,K))
                return CHILD                
    


def select_best_inds(population, NUMBER):
    TOP = []
    for i in range(NUMBER): TOP.append(0)
    
    SCORE = 0
    LASTSCORE = 66666
    for i in range(NUMBER):
        SCORE = 0
        for individual in population:
            SCR = individual.ELO
            if (SCR > SCORE) and (SCR < LASTSCORE):
                TOP[i]=individual
                SCORE = SCR
                LASTSCORE = SCR

    TOP = [x for x in TOP if x]

    return TOP


def crossover(population, indexA, indexB):
    pA = random.randrange(len(population[indexA].PARAMETERS))
    pB = random.randrange(len(population[indexB].PARAMETERS))


    Buffer = population[indexA].PARAMETERS[pA].value

    population[indexA].PARAMETERS[pA].value = population[indexB].PARAMETERS[pB].value
    population[indexB].PARAMETERS[pB].value = Buffer

    return population


def Mate(individuals, nchild):
    Children = []
    for N in range(nchild):
        Child = machine(NewMacName())
        for P in range(len(Child.PARAMETERS)):
            Child.PARAMETERS[P] = copy.deepcopy(random.choice(individuals).PARAMETERS[P])
        Children.append(Child)

    return Children

def replicate_best_inds(population, NUMBER):
    TOP = select_best_inds(population, NUMBER)


    for IND in TOP:
        for N in range(NUMBER):
            IND.filename = NewMacName()
            population.append(copy.deepcopy(IND))

    return population


def clone_from_template():
    Tpool = open('%s/top_machines/machines.list' % Fdir, 'r')
    POOL = []
    
    for line in Tpool.readlines():
        if '.mac' in line:
            POOL.append(line[:-1])

    X = random.randrange(len(POOL))

    model = open('%s/top_machines/%s' % (Fdir,POOL[X]), 'r')

    CHILD = machine(NewMacName())
    for line in model.readlines():
        CHILD.read(line)
    CHILD.onTOP = 0
                  
    #for N in range(NUMBER):
    CHILD.mutate(10,6)    

    return CHILD


def NewMacName():
    letters = ""
    for k in range(3):
        x=random.randrange(65,91)
        letters += chr(x)
        
    numbers = random.randrange(0,6489)
    return "%s%i.mac" % (letters, numbers)

def IsEqual(model, against):
    for P in range(len(model.PARAMETERS)):
        if not model.PARAMETERS[P].value == against.PARAMETERS[P].value:
            return 0

    return 1


def EliminateEquals(population, Range):
    for I in range(Range, len(population)):
        if population[I] == 0:
            continue
        for T in range(I + 1, len(population)):
            if population[T] == 0:
                continue
            if IsEqual(population[I], population[T]):
                population[T] = 0

    return list(filter((0).__ne__, population))


def Triangulate_value(values):
    if len(values) == 0:
        return 0
    
    BUFFER=0
    #for value in values: BUFFER+=value

    #value = BUFFER/len(values)

    X = random.randrange(len(values))
    

    return values[X]


def sendtoHallOfFame(MACHINE):
    
    copyfile(machine_dir + '/' + MACHINE.filename, machine_dir + '/top_machines/' + MACHINE.filename)
    Fo = open(machine_dir + '/top_machines/machines.list', 'a+')
    Fo.write(MACHINE.filename+'\n')
    print('machine %s sent to top.' % MACHINE.filename)
    MACHINE.onTOP=1
    MACHINE.write()
        

            
