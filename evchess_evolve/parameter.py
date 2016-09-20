#!/usr/bin/python
import random


class parameter():

    def __init__(self, name, dumpable, chanceMutate, value, aP=0,
                 LIM=None, bLIM=None, INCR=1, locked=0, stdvalue=0):

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

    def dump_parameter_stat(self, individual):
        parameter = self.name

        DUMP_games = individual.PARAMETERS[
            0].value - individual.PARAMETERS[0].dumpedvalue
        DUMP_wins = individual.PARAMETERS[
            1].value - individual.PARAMETERS[1].dumpedvalue
        DUMP_draws = individual.PARAMETERS[
            2].value - individual.PARAMETERS[2].dumpedvalue
        DUMP_loss = individual.PARAMETERS[
            3].value - individual.PARAMETERS[3].dumpedvalue
        DUMP_K = individual.PARAMETERS[4].value - \
            individual.PARAMETERS[4].dumpedvalue

        value = self.value

        if type(self.value) == list:
            BUF = ""
            for ind in value:
                BUF += str(ind) + "x"
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
                    # print('match!')
                    ISNEWPARAM = False

                    #print("searching for '" + value + "'.")
                    for PRchild in child:
                        #print("match? '" +PRchild.tag + "'.")
                        if PRchild.tag == value:
                            # print('match!')
                            ISNEWPARAMVAL = False
                            PRchild[0].text = str(
                                int(PRchild[0].text) + DUMP_wins)
                            PRchild[1].text = str(
                                int(PRchild[1].text) + DUMP_draws)
                            PRchild[2].text = str(
                                int(PRchild[2].text) + DUMP_loss)
                            PRchild[3].text = str(
                                int(PRchild[3].text) + DUMP_games)
                            PRchild[4].text = str(
                                int(PRchild[4].text) + DUMP_K)

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

        # ET.dump(root)
        tree.write(Fdir + "/paramstats.xml")

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
