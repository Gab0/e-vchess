#!/bin/python

from subprocess import call
from random import randrange

from sys import argv
from os import path, chdir
from json import dumps, loads
from chessArena import settings
settings.initialize()

from evchess_evolve.core import loadmachines, setmachines

# print(VerboseMove)

class DuelTable():

    def __init__(self):
        print("""
        loading machines from Hall of Fame.

          Choose your opponent:

                      """)
        self.AllMachines = loadmachines(DIR=settings.TOPmachineDIR)
                        
        print("zero")
        print("0 - random")
        for M in range(len(self.AllMachines)):
            view = '%i - %s' % (M+1, self.AllMachines[M].filename)
            view += "      %i :%i" %\
                (self.AllMachines[M].ELO,
                        self.AllMachines[M].getParameter("real_world_score"))
            print(view)
        print("")
        LOADED = 0
        userchoice = self.CollectInput(
            [x for x in range(len(self.AllMachines)+1)])

        if not userchoice:
            userchoice = randrange(len(self.AllMachines))+1
        else:
            pass
            #choices = userchoice.split(" ")
        Callargs = [settings.enginebin, "--deep", '4', '--xdeep', '3', '--showinfo']

        if userchoice != "zero":
            LOADED = 1
            userchoice -= 1
            print('Loading %s. glhf' % self.AllMachines[userchoice].filename)
            Callargs += ['--specific', self.AllMachines[userchoice].filename]
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
                result = 1
                print("logically.")
            else:
                result = -1
                print("a bad day for the computer age.")
            self.AllMachines[userchoice].getParameter("real_world_score",
                        toSUM=result)
            setmachines(self.AllMachines, DIR=settings.TOPmachineDIR)

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
