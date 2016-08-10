#!/bin/python

from subprocess import call
from random import randrange

from sys import argv

from chessArena import settings
settings.initialize()

from chessArena.tournament import loadscores, savescores,\
    ModifyScore, LoadMachineList

#print(VerboseMove)


class DuelTable():
    def __init__(self):
        ScoreData = loadscores()
        print("""
        loading machines from Hall of Fame.

          Choose your opponent:
                      """)

        LegalMachines = ['random'] + LoadMachineList()

        for M in range(len(LegalMachines)):
            print('%i - %s' % (M, LegalMachines[M]))
        print("")

        userchoice = self.CollectInput(
            [ x for x in range(len(LegalMachines)) ] )

        if not userchoice:
            userchoice = randrange(1,len(LegalMachines))

        print('Loading %s. glhf' % LegalMachines[userchoice])
        Callargs =[ settings.enginebin, "--deep", '4', '--xdeep', '1',
                    '--specific', LegalMachines[userchoice] ] 
        engineCALL = " ".join(Callargs)

        Command = ['xboard', '-fcp', engineCALL]

        X = call(Command)

        print("Did this machine win the game? [y/n]")

        FeedBack = self.CollectInput( ['y', 'n'] )

        if FeedBack == 'y':
            ScoreData = ModifyScore(ScoreData, LegalMachines[userchoice], 1)
            print("logically.")
        else:
            ScoreData = ModifyScore(ScoreData, LegalMachines[userchoice], -1)
            print("a bad day for the computer age.")


        savescores(ScoreData)

        
    def CollectInput(self, ValidValues):
        userchoice = None
        while userchoice == None:
            userchoice = input(">>>")
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




x = DuelTable()
