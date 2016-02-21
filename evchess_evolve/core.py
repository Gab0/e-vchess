#!/usr/bin/python

import os
import random
import xml.etree.ElementTree as ET

#machine directory.
Fdir = "/home/gabs/Desktop/e-vchess/machines"


class parameter():
    def __init__(self, name, dumpable, Cparam, value, aP=0, LIM = None, bLIM = None, INCR = 1, locked=0):
        
        self.name = name
        self.marks_dumpable = dumpable
        
        self.Cparam = 50

        self.stdvalue = value
     
        self.value = value
        self.dumpedvalue = 0

        self.alwaysPOSITIVE = aP
        self.LIM = LIM
        self.bLIM = bLIM
        self.INCR = INCR

        self.locked=locked
        
        
    def read(self, split_line):
        if self.locked: return
        try:
            if self.name in split_line[0]:
                if split_line[0] == "|": self.locked = 1
                

                if self.marks_dumpable:
                    self.value = int(split_line[2])
                    self.dumpedvalue = int(split_line[3])

                elif type(self.value) == int:
                    self.value = int(float(split_line[2]))

                elif type(self.value) == float:
                    self.value = round(float(split_line[2]),3)

                elif type(self.value) == list:

                    if type(self.value[0]) == int:
                        self.value = []
                        for z in range(2, len(split_line)):
                            self.value.append(int(split_line[z]))

                            
                    if type(self.value[0]) == float:
                        self.value = []
                        for z in range(2, len(split_line)):
                            self.value.append(float(split_line[z]))

        except ValueError:
            print('fail to read %s.' % self.name)

        
    def write(self):
        
        STR = self.name + " = "

        if not type(self.value) == list:
            STR += str(self.value)

        else:
            for i in range(len(self.value)):
                STR += str(self.value[i]) + " "


        if self.marks_dumpable:
            STR += " " + str(self.dumpedvalue)

        STR += "\n"
        if self.locked: STR = "|"+STR
        return STR



    def putonlimits(self, value):
        TYPE = type(value)
        if TYPE == list:
            for X in range(len(value)):
                value[X] = self.putonlimits(value[X])
            return value


        if self.LIM: LIM = self.LIM
        else: LIM = 0
        if self.bLIM: bLIM = self.bLIM
        else: bLIM = 0


        if type(self.stdvalue) == list:
            stdvalue = self.stdvalue[0]
        else: stdvalue = self.stdvalue

        
                
        if self.LIM != None:
            if value > LIM: value = random.uniform(stdvalue ,LIM)
        if self.bLIM != None:
            if value < bLIM: value = random.uniform(bLIM, stdvalue)


        if self.alwaysPOSITIVE: value = abs(value)

        if TYPE == float:
            value = round(value, 2)


        if not value % self.INCR == 0:
            if random.randrange(9) < 5:
                value -= value % self.INCR
            else:
                value += value % self.INCR

        if TYPE == float:
            value = round(value, 2)

        if TYPE == int:
            value = int(round(value))


        return value

    
    def mutate(self, eMOD, AGR):
        if (self.marks_dumpable) or (self.locked):
            return

        if self.name == 'param_TIMEweight':
            self.value = setTIMEweight(self.value)
            return

        

        
        elif not type(self.value) == list:
            self.value += getP_act(self.Cparam, eMOD) * random.randrange(0,AGR) * self.INCR

        else:
            for kk in range(len(self.value)):
                self.value[kk] += getP_act(self.Cparam, eMOD) * random.randrange(0,AGR) * self.INCR
                


        self.value = self.putonlimits(self.value)
        if type (self.value) == float:
            self.value = round(self.value, 3)



    def dump_parameter_stat(self, individual):
        parameter = self.name

        DUMP_games = individual.PARAMETERS[0].value - individual.PARAMETERS[0].dumpedvalue
        DUMP_wins = individual.PARAMETERS[1].value - individual.PARAMETERS[1].dumpedvalue
        DUMP_draws = individual.PARAMETERS[2].value - individual.PARAMETERS[2].dumpedvalue
        DUMP_loss = individual.PARAMETERS[3].value - individual.PARAMETERS[3].dumpedvalue
        DUMP_K = individual.PARAMETERS[4].value - individual.PARAMETERS[4].dumpedvalue

        value = self.value

        if type(self.value) == list:
            BUF = ""
            for ind in value:
                BUF += str(ind)+"x"
            value = BUF[:-1]
        value = 'x' + str(value)
        
        if not os.path.exists(Fdir + "/paramstats.xml"):
            
            root = ET.Element('root')
            tree = ET.ElementTree(root)

        else:
            tree = ET.parse(Fdir + "/paramstats.xml")
            root = tree.getroot()

        ISNEWPARAM = True
        ISNEWPARAMVAL = True

        if len(root) > 0:
            #print("searching for '" + parameter + "'.")
            for child in root:
                #print("match? '" +child.tag + "'.")
                if child.tag == parameter:
                    #print('match!')
                    ISNEWPARAM = False
                    
                    #print("searching for '" + value + "'.")
                    for PRchild in child:
                        #print("match? '" +PRchild.tag + "'.")
                        if PRchild.tag == value:
                            #print('match!')
                            ISNEWPARAMVAL = False
                            PRchild[0].text = str(int(PRchild[0].text) + DUMP_wins)
                            PRchild[1].text = str(int(PRchild[1].text) + DUMP_draws)
                            PRchild[2].text = str(int(PRchild[2].text) + DUMP_loss)
                            PRchild[3].text = str(int(PRchild[3].text) + DUMP_games)
                            PRchild[4].text = str(int(PRchild[4].text) + DUMP_K)


        if ISNEWPARAM == True:
            PARAM = ET.SubElement(root, parameter)

        else:
            PARAM = root.find(parameter)

        if ISNEWPARAMVAL == True:
            
            NEWPARAMVAL = ET.SubElement(PARAM, value)

            ET.SubElement(NEWPARAMVAL, "wins").text = str(DUMP_wins)
            ET.SubElement(NEWPARAMVAL, "draws").text = str(DUMP_draws)
            ET.SubElement(NEWPARAMVAL, "loss").text = str(DUMP_loss)
            ET.SubElement(NEWPARAMVAL, "games").text = str(DUMP_games)
            ET.SubElement(NEWPARAMVAL, "K").text = str(DUMP_K)


        #ET.dump(root)
        tree.write(Fdir + "/paramstats.xml")

    def randomize(self):
        if self.locked: return
        if not self.LIM: MAX = 2*self.stdvalue
        else: MAX = self.LIM
        
        if not self.bLIM: MIN = MAX - 2*self.stdvalue
        else: MIN = self.bLIM

        Grading = round((MAX-MIN)/self.INCR)

        VAL = random.randrange(Grading)

        self.value = VAL*self.INCR + MIN

        self.value = self.putonlimits(self.value)

    def lock(self, value):
        self.value = self.putonlimits(value)
        self.locked = 1
        
    def unlock(self, rnd):
        self.locked = 0
        if rnd: self.randomize()
        
        
class machine ():

    def __init__(self, fname):
        self.filename = fname
        self.wasmodified = 0


        self.TPARAMETERS = []
        self.PARAMETERS = []

        self.TPARAMETERS.append(parameter("stat_games", 1, 0, 0))
        self.TPARAMETERS.append(parameter("stat_wins", 1, 0, 0))
        self.TPARAMETERS.append(parameter("stat_draws", 1, 0, 0))
        self.TPARAMETERS.append(parameter("stat_loss", 1, 0, 0))
        self.TPARAMETERS.append(parameter("stat_K", 1, 0, 0))

        self.TPARAMETERS.append(parameter("stat_elo", 0,0,1000))
        


        self.PARAMETERS.append(parameter("param_DEEP", 0, 30, 2, bLIM=4, INCR=2, LIM=10))

        self.PARAMETERS.append(parameter("eval_randomness", 0, 30, 60, INCR=10, bLIM=1))
        self.PARAMETERS.append(parameter("param_seekpieces", 0, 30, 1, bLIM=0, INCR=0.25, LIM=3))
        #self.PARAMETERS.append(parameter("param_deviationcalc", 0, 30, 0.1, INCR=0.2))
        self.PARAMETERS.append(parameter("param_evalmethod", 0, 30, 0, aP=1, bLIM=0, LIM=0))
        self.PARAMETERS.append(parameter("param_seekatk", 0, 30, 0.5, bLIM=-1, INCR=0.25, LIM=3))
        self.PARAMETERS.append(parameter("param_seekmiddle", 0, 30, 3, LIM = 5))
        self.PARAMETERS.append(parameter("param_presumeOPPaggro", 0, 30, 1, LIM=1.1, bLIM=1, INCR=0.1))
        self.PARAMETERS.append(parameter("param_pawnrankMOD", 0, 30, 1, LIM=37))
        self.PARAMETERS.append(parameter("param_parallelcheck", 0, 80, 4,LIM=21, bLIM=0))
        self.PARAMETERS.append(parameter("param_balanceoffense", 0, 80, 1,LIM=5, bLIM=0))
        #self.PARAMETERS.append(parameter("param_cumulative", 0, 80, 0,LIM=1, bLIM=0, INCR=0.1))
        
        #self.PARAMETERS.append(parameter("param_pvalues", 0, 5, [100,500,300,300,900,2000], INCR=50, bLIM=70, LIM=2500, locked=1))
        #self.PARAMETERS.append(parameter("param_TIMEweight", 0, 30, [0.9, 0.85, 0.9, 0.85, 0.81, 0.765, 0.825, 0.789, 0.844, 0.85], LIM=1.3, bLIM=0.01, INCR = 0.05))


 

        self.onTOP = 0
        
        self.stat_games = 0
        self.stat_wins = 0
        self.stat_draws = 0
        self.stat_loss = 0
        self.K = 0
        
        self.dumped_games = 0
        self.dumped_wins = 0
        self.dumped_draws = 0
        self.dumped_loss = 0
        self.dumped_K = 0   

    def read(self, split_line):
        for parameter in self.TPARAMETERS+self.PARAMETERS:
            if len(split_line) < 2:
                if split_line[0] == 'W':
                    self.TPARAMETERS[1].value +=1
                    self.TPARAMETERS[0].value +=1
                elif split_line[0] == 'L':
                    self.TPARAMETERS[3].value +=1
                    self.TPARAMETERS[0].value +=1
                elif split_line[0] == 'D':
                    self.TPARAMETERS[2].value +=1
                    self.TPARAMETERS[0].value +=1
                    
            if parameter.name == split_line[0]:
                parameter.read(split_line)

        if 'TOP' in split_line:
            self.onTOP = 1
            
    def write(self):
        Fo = open(Fdir+'/'+self.filename, "w+")
        for parameter in self.PARAMETERS+self.TPARAMETERS:
            Fo.write(parameter.write())
            Fo.write('\n')

        if self.onTOP:
            Fo.write('TOP\n')
            
        Fo.close()


    def mutate(self, eMOD, AGR):
        print('mutating ' + self.filename)
        for parameter in self.PARAMETERS:
            parameter.mutate(eMOD,AGR)
            parameter.value = parameter.putonlimits(parameter.value)

    def dump_parameter_stat(self):
        for parameter in self.PARAMETERS:
            if parameter.marks_dumpable == 0:
                parameter.dump_parameter_stat(self)

        for i in range(4):
            self.PARAMETERS[i].dumpedvalue = self.PARAMETERS[i].value


    def randomize(self):
        for parameter in self.PARAMETERS:
            parameter.randomize
            

    def delete(self):
        os.remove(Fdir+'/'+self.filename)

    def resetscores(self):
        for P in self.TPARAMETERS:
            P.value = 0
            
k=0
population=[]


def populate(population,popsize):
    NEWINDS = []
    for i in range(popsize):
        NEWINDS.append(machine(str(random.randrange(0,6489))+".mac")) 
        


    for I in NEWINDS:
        I.randomize()
        population.append(I)

        
    return population
       
def loadmachines():
        population = []
        k=0
        for file in os.listdir(Fdir):
        
            if file.endswith(".mac"):
                Fo = open(Fdir+'/'+file, "r+")
                population.append(machine(file))
                
                for line in Fo.readlines():
                    if line == "\n": continue
                    L = line.split()
                    if len(L) == 3:
                        L.append(0)

                    population[k].read(L)


                k+=1

        return population
def p100():
    return random.randrange(0,100)

def getP_act(chance, eMOD):
    result = random.randrange(0,100)+(50-eMOD)
    if result > chance: return 0
    elif result < chance/2:
        print('mutated-')
        return -1
    else:
        print('mutated+')
        return 1
        
def mutatemachines(AGR, population):


    for i in range(len(population)):
            eMOD = 50-population[i].TPARAMETERS[1].value + population[i].TPARAMETERS[2].value/2 /(population[i].TPARAMETERS[0].value+1)

            population[i].mutate(eMOD, AGR)
                


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
    root = tree.getroot()

    for child in root:
        #print(child.tag)
        if child.tag == parameter:
  
            for stat in range(len(child)):
                DUMP.append([child[stat].tag])
                for score in child[stat]:
                    DUMP[stat+1].append(score.text)
                


    return DUMP

        




def setmachines(population, includeall):
    for i in range(len(population)):
        

        if os.path.isfile(Fdir+'/'+population[i].filename):
            os.remove(Fdir+'/'+population[i].filename)
            
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
        SCORE = population[k].TPARAMETERS[5].value  #population[k].PARAMETERS[1].value + (population[k].PARAMETERS[2].value/2) / (population[k].PARAMETERS[0].value)

            
        POP_SCORETABLE.append(SCORE)
        MEDIUMSCORE += SCORE
        VALIDPOP += 1
    
    if VALIDPOP > 0:    
        MEDIUMSCORE = MEDIUMSCORE/VALIDPOP
        print('mediumscore = %i' % MEDIUMSCORE)
        if action < 0:
            action = -action
            N_removed = 0
            for k in range(len(POP_SCORETABLE)):
                if (POP_SCORETABLE[k] > -1):
                    if (POP_SCORETABLE[k] < MEDIUMSCORE*MODlimit):
                        if N_removed < action:
                            print('subject deleted. ' + population[k].filename)
                            try:
                                os.remove(Fdir+'/'+population[k].filename)
                            except FileNotFoundError:
                                print("can't find machine file, but it's ok.")
                            population[k] = 0
                            REMOVED.append(k)
                            N_removed+=1


            for ind in reversed(REMOVED):
                population.pop(ind)


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
    K_ = range(population[K].TPARAMETERS[5].value-20,  population[K].TPARAMETERS[5].value+20) 
    for I in range(len(population)):
        if population[I].TPARAMETERS[5].value in K_:
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
            SCR = individual.TPARAMETERS[5].value
            if (SCR > SCORE) and (SCR < LASTSCORE):
                TOP[i]=individual
                SCORE = SCR
                LASTSCORE = SCR

    for i in reversed(range(len(TOP))):
        if TOP[i] == 0: TOP.pop(i)

    return TOP


def replicate_best_inds(population, NUMBER):
    TOP = select_best_inds(population, NUMBER)


    
    for IND in TOP:
        for N in range(NUMBER):
            IND.filename = NewMacName()
            population.append(IND)

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
    return "%i.mac" % random.randrange(0,6489)


def Triangulate_value(values):
    if len(values) == 0: return 0
    
    BUFFER=0
    #for value in values: BUFFER+=value

    #value = BUFFER/len(values)

    X = random.randrange(len(values))
    

    return values[X]



def CyclingStatLock(population):
    DB = "%s/CyclingLockDB" % Fdir
    if not os.path.isfile(DB): return population

    ACTIVE = 0
    NEXT = 0
    CYCLE_PARAMETERS = []
    DataBase = open(DB, 'r').readlines()
    for line in DataBase:
        L = line.split(" = ")
        if len(L) > 1:
            CYCLE_PARAMETERS.append([L[0], float(L[1])])
    for P in range(len(CYCLE_PARAMETERS)):
        if CYCLE_PARAMETERS[P][0][0] == ">":
            CYCLE_PARAMETERS[P][0] = CYCLE_PARAMETERS[P][0][1:]
            ACTIVE = P
            
            
    if ACTIVE+1<len(CYCLE_PARAMETERS): NEXT = ACTIVE+1

    
    TOP = select_best_inds(population,3)
    
    print(TOP)#TEMP
    
    for P in range(len(TOP[0].PARAMETERS)):
        if TOP[0].PARAMETERS[P].name in CYCLE_PARAMETERS[ACTIVE][0]:
            ACTIVE_index = P

        if CYCLE_PARAMETERS[NEXT][0] in TOP[0].PARAMETERS[P].name:
            NEXT_index = P

    APPLIEDvalue = []
    for T in TOP:
        #print(T)#TEMP
        APPLIEDvalue.append(T.PARAMETERS[ACTIVE_index].value)


    APPLIEDvalue = Triangulate_value(APPLIEDvalue)
    CYCLE_PARAMETERS[ACTIVE][1] = APPLIEDvalue

    

    
    for IND in population:
                IND.PARAMETERS[NEXT_index].unlock(1)
                IND.PARAMETERS[ACTIVE_index].lock(APPLIEDvalue)
                

    DataBase = open(DB, 'w+')
    for P in range(len(CYCLE_PARAMETERS)):
        string = "%s = %f\n" % (CYCLE_PARAMETERS[P][0], CYCLE_PARAMETERS[P][1])
        if P == NEXT: string = ">"+string
        DataBase.write(string)
    
    
    
    return population   
    

def PrepareCyclingStatLock(population):
    DB = "%s/CyclingLockDB" % Fdir
    if not os.path.isfile(DB): return population

    ACTIVE = 0
    NEXT = 0
    CYCLE_PARAMETERS = []
    DataBase = open(DB, 'r').readlines()
    for line in DataBase:
        L = line.split(" = ")
        if len(L) > 1:
            CYCLE_PARAMETERS.append([L[0],L[1][:-1]])

    if len(CYCLE_PARAMETERS) < 1: return population

    #print(CYCLE_PARAMETERS) #TEMP
    for C in range(len(CYCLE_PARAMETERS)):
        for P in range(len(population[0].PARAMETERS)):
            #print(P)
            #print(C)
            try:
                if population[0].PARAMETERS[P].name in CYCLE_PARAMETERS[C][0]:
                    CYCLE_PARAMETERS[C][0] = P
            except:
                print('>>>>>'+population[0].PARAMETERS[P].name)
                

    for P in range(len(CYCLE_PARAMETERS),-1):
        if type(CYCLE_PARAMETERS[P][0]) != int:
            CYCLE_PARAMETERS.pop(P)
       


    for IND in population:
        IND.resetscores()
        for CP in CYCLE_PARAMETERS:
            IND.PARAMETERS[CP[0]].lock(float(CP[1]))
            
    return population
