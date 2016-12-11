#!/bin/python


from moveTraining.creator import trainingDataCreator
from moveTraining.feeder import trainingDataFeeder

from evchess_evolve import core, management

from chessArena import settings
Settings = settings.Settings()
import sys
import json
import random

if '--full' in sys.argv:
    engineargs = ['--xdeep', '1']
else:
    engineargs = ['--deep', '4']
    
if '--alternative-folder' in sys.argv:
    A_machineDIR = sys.argv[sys.argv.index('--alternative-folder') + 1]
    engineargs += [ A_machineDIR ]
else:
    A_machineDIR = None
    

if "create" in sys.argv:
    x = trainingDataCreator('moveTraining/database2015.pgn')
elif "createsimple" in sys.argv:
    x = trainingDataCreator(SimpleDatabase = "newdatabase")
else:
    posLOG = open("pos_log", 'a')
    for N in range(3):
        SESSION = trainingDataFeeder('manualdb', engineargs, A_machineDIR)
        result = SESSION.Result
        FullTestLen = len(SESSION.TrialPositions.keys())
        print("Total test lenght: %i" % FullTestLen)
        print(result)
        posLOG.write("\nPassed Tests: %i.\n" % SESSION.PassedTests)
        posLOG.write(json.dumps(result, indent=2)+"\n")
        ApprovedMachines = list(result.keys())
        pop = core.loadmachines()
        if result:
            for scoreNumber in range(1, round(max([result[x] for x in result]))):
                for MAC in ApprovedMachines:
                    if scoreNumber < result[MAC] < scoreNumber +1:
                        print("%s: %i" % (MAC, result[MAC]))

        
            BestScoreOnGroup = max([result[x] for x in ApprovedMachines])
            NumberOfBestScorers = sum([1 for x in ApprovedMachines if result[x] == BestScoreOnGroup])
            for IND in range(len(pop)):
                NAME = pop[IND].filename
                if NAME not in ApprovedMachines or result[NAME] < 0:
                    management.bareDeleteMachine(Settings.machineDIR, NAME)
                    pop[IND] = None
            pop = [ x for x in pop if x ]
            
            for IND in pop:
                if FullTestLen - result[IND.filename] < 3:
                    core.sendtoHallOfFame(IND)
                if result[IND.filename] == BestScoreOnGroup:
                    if NumberOfBestScorers < len(pop)/6:
                        core.Mate([IND, random.choice(pop)], 2, ID="POS")
                    
        stock_popsize=16
        while len(pop) < stock_popsize/2:
            pop += core.Mate(pop,2,ID="pos")

        pop = core.populate(pop, stock_popsize-len(pop), 1)
            
        core.mutatemachines(2,pop)

        core.setmachines(pop)

            
