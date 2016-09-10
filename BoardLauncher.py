#!/bin/python

from subprocess import call
from random import randrange

from sys import argv
from os import path, chdir
from json import dumps, loads
from chessArena import settings
settings.initialize()

from chessArena.tournament import LoadMachineList

# print(VerboseMove)


def loadscores():
    try:
        F = open(settings.TOPmachineDIR + '/scoreData')
        Content = F.read()
        ScoreData = loads(Content)
        return ScoreData
    except:
        return {}
        pass


def savescores(DATA):
    F = open(settings.TOPmachineDIR + '/scoreData', 'w')

    F.write(dumps(DATA))
    F.close()


def ModifyScore(DATA, MacName, Operator):
    try:
        DATA[MacName] += Operator
    except:
        DATA[MacName] = Operator

    return DATA


class DuelTable():

    def __init__(self):
        ScoreData = loadscores()
        print("""
        loading machines from Hall of Fame.

          Choose your opponent:

                      """)

        LegalMachines = ["random"] + LoadMachineList()
        print("zero")
        for M in range(len(LegalMachines)):
            view = '%i - %s' % (M, LegalMachines[M])
            try:
                view += "   :%i" % ScoreData[LegalMachines[M]]
            except:
                pass
            print(view)
        print("")
        LOADED = 0
        userchoice = self.CollectInput(
            [x for x in range(len(LegalMachines))])

        if not userchoice:
            userchoice = randrange(1, len(LegalMachines))

        Callargs = [settings.enginebin, "--deep", '4', '--xdeep', '3', '--showinfo']

        if userchoice != "zero":
            LOADED = 1
            print('Loading %s. glhf' % LegalMachines[userchoice])
            Callargs += ['--specific', LegalMachines[userchoice]]
        engineCALL = " ".join(Callargs)

        Command = ['xboard', '-fcp', engineCALL]

        X = call(Command)
        if LOADED:
            try:
                print("Did this machine win the game? [y/n]")
                FeedBack = self.CollectInput(['y', 'n'])
                
            except KeyboardInterrupt:
                print("\n")
                exit()

            if FeedBack == 'y':
                ScoreData = ModifyScore(
                    ScoreData, LegalMachines[userchoice], 1)
                print("logically.")
            else:
                ScoreData = ModifyScore(
                    ScoreData, LegalMachines[userchoice], -1)
                print("a bad day for the computer age.")

            savescores(ScoreData)

    def CollectInput(self, ValidValues):
        userchoice = None
        while userchoice == None:
            userchoice = input(">>>")
            if userchoice == "zero":
                break
            try:
                userchoice = int(userchoice)
            except ValueError:
                userchoice = userchoice.lower()

            try:
                assert(userchoice in ValidValues)
            except:
                userchoice = None
                print("Invalid input. Try again.")

        return userchoice


chdir(path.dirname(path.realpath(__file__)))

x = DuelTable()
