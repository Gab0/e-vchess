#!/usr/bin/python
import random


class parameter():

    def __init__(self, name, dumpable, chanceMutate, value, aP=0,
                 LIM=None, bLIM=None, INCR=1, locked=0, stdvalue=0, promoter=None):

        self.name = name
        self.marks_dumpable = dumpable

        self.chanceMutate = chanceMutate

        self.stdvalue = stdvalue

        self.value = value
        self.dumpedvalue = 0

        self.alwaysPOSITIVE = aP
        self.LIM = LIM
        self.bLIM = bLIM
        self.INCR = INCR
        
        self.promoter = promoter
        self.locked = locked

    def read(self, split_line):
        if self.locked:
            return
        try:
            if self.name in split_line[0]:
                if split_line[0] == "|":
                    self.locked = 1
                    print("locked.")

                if self.marks_dumpable:
                    self.value = int(split_line[2])
                    self.dumpedvalue = int(split_line[3])

                elif type(self.value) == list:

                    if type(self.value[0]) == int:
                        self.value = []
                        for z in range(2, len(split_line)):
                            self.value.append(int(split_line[z]))

                    if type(self.value[0]) == float:
                        self.value = []
                        for z in range(2, len(split_line)):
                            self.value.append(float(split_line[z]))

                else:
                    self.value = round(float(split_line[2]), 3)
                    #print(split_line)


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
        if self.locked:
            STR = "|" + STR
        return STR

    def putonlimits(self):

        if self.LIM != None:
            if self.value > self.LIM:
                self.value = self.LIM

        if self.bLIM != None:
            if self.value < self.bLIM:
                self.value = self.bLIM

        if self.alwaysPOSITIVE:
            self.value = abs(self.value)

        if type(self.value) == float:
            self.value = round(self.value, 2)

        elif type(self.value) == int:
            self.value = int(round(self.value))



        

    def mutate(self, MutateProbabilityDamper, Aggro):
        if (self.marks_dumpable) or (self.locked):
            return

        if self.name == 'param_TIMEweight':
            self.value = setTIMEweight(self.value)
            return

        AggroModifier = random.randrange(1, Aggro) if Aggro > 1 else 1
        if type(self.value) == list:
            for V in range(len(self.value)):

                self.value[V] += self.mutateDecideIfProceed\
                    (self.chanceMutate, MutateProbabilityDamper)\
                    * AggroModifier * self.INCR

        else:

            self.value += self.mutateDecideIfProceed\
                (self.chanceMutate, MutateProbabilityDamper)\
                * AggroModifier * self.INCR

        self.putonlimits()

    def mutateDecideIfProceed(self, chance, MutateProbabilityDamper):

        result = random.randrange(100) + MutateProbabilityDamper
        if result > chance:
            return 0

        GRAPHIC = ['-', '+']
        VALUE = [-1, 1]

        x = 1 if self.value >= 0 else 0

        if MutateProbabilityDamper < 0:
            x = 1 - x

        #print("mutated %c" % GRAPHIC[x])
        return VALUE[x]

    def randomize(self):
        if self.locked:
            print('locked')
            return
        if not self.LIM:
            MAX = 2 * self.stdvalue
        else:
            MAX = self.LIM

        if not self.bLIM:
            MIN = MAX - 2 * self.stdvalue
        else:
            MIN = self.bLIM

        Grading = round((MAX - MIN) / self.INCR)
        if Grading == 0:
            self.value = 0
            return

        VAL = random.randrange(Grading)

        self.value = VAL * self.INCR + MIN

        self.putonlimits()

    def lock(self, value):
        self.value = value
        self.putonlimits()
        self.locked = 1

    def unlock(self, rnd):
        self.locked = 0
        if rnd:
            self.randomize()
    def getValidValues(self):
        lower = self.bLIM if self.bLIM != None else 0
        upper = self.LIM if self.LIM != None else 0

        values = []
        v = lower
        while v <=upper:
            values += [v]
            v += self.INCR
            
        return values

    def toGene(self):
        Range = self.LIM - self.bLIM
        NumberOfSteps = round(self.value/self.INCR)

        b = str(int(bin(NumberOfSteps)[2:]))

        b = "%s%s" % ((8 - len(b)) * "0", b)
        return b

    def fromGene(self, Gene):
        NumberOfSteps = int(Gene, 2)
        self.value = self.bLIM + NumberOfSteps * self.INCR
        
        
    
