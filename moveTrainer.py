#!/bin/python
from moveTraining.creator import trainingDataCreator
from moveTraining.feeder import trainingDataFeeder

from evchess_evolve import core, management

from chessArena import settings
Settings = settings.Settings()
import sys
import json
import random
import optparse

if '--full' in sys.argv:
    engineargs = ['--xdeep', '1', '--deep', '4']
else:
    engineargs = ['--deep', '4']

parser = optparse.OptionParser()
parser.add_option('--mutate', action='store_true', dest='Mutate', default=False)
if '--alternative-folder' in sys.argv:
    _machineDIR = sys.argv[sys.argv.index('--alternative-folder') + 1]
elif 'hof' in sys.argv:
    _machineDIR = Settings.HoFmachineDIR
else:
    _machineDIR = Settings.machineDIR

(options, args) = parser.parse_args()
if "create" in sys.argv:
    x = trainingDataCreator('moveTraining/database2015.pgn')
elif "reeval" in sys.argv:
    x = trainingDataCreator(Database = "manualdb")
elif "refresh" in sys.argv:
    x = trainingDataCreator(Database="manualdb", NewMovements="newfen")
else:
    posLOG = open("pos_log", 'a', 1)
    for N in range(64):
        SESSION = trainingDataFeeder('manualdb', engineargs, _machineDIR)
        result = SESSION.Result
        FullTestLen = len(SESSION.TrialPositions.keys())
        print("Total test lenght: %i" % FullTestLen)
        print(result)
        
        #posLOG.write(json.dumps(result, indent=2)+"\n")
        ApprovedMachines = list(result.keys())
        posLOG.write("\nPassed Tests: %i @ %s, run #%i.\n" % ( SESSION.PassedTests, _machineDIR, N))
        posLOG.write('\n'.join(["%s: %i" % (W, result[W]) for W in ApprovedMachines if result[W] > 0 ]))
        pop = core.loadmachines(DIR=_machineDIR)
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
                    management.bareDeleteMachine(_machineDIR, NAME)
                    pop[IND] = None
            pop = [ x for x in pop if x ]
            
            for IND in pop:
                if FullTestLen * 10 - result[IND.filename] < 20:
                    core.sendtoHallOfFame(IND)
                if result[IND.filename] == BestScoreOnGroup:
                    if NumberOfBestScorers < len(pop)/6:
                        core.Mate([IND, random.choice(pop)], 2, ID="POS")
                    
        stock_popsize=16
        while len(pop) < stock_popsize/2 and pop:
            pop += core.Mate(pop,2,ID="pos")
        if options.Mutate:
            pop = core.populate(pop, stock_popsize-len(pop), 1)
            core.mutatemachines(2,pop)
            core.setmachines(pop, DIR=_machineDIR)

            
