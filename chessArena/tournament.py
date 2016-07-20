#!/bin/python

from time import sleep
from json import loads, dumps
from os import remove
from threading import Thread

from chessArena import settings
settings.initialize()

from chessArena.table import Table
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

def LoadMachineList():
    MachineListLocation = settings.TOPmachineDIR + '/machines.list'
    MachineList = open(MachineListLocation, 'r').readlines()

    LegalMachines = []
    for line in MachineList:
        if '.mac' in line:
            LegalMachines.append(line[:-1])

    return LegalMachines


class Tournament():
    def __init__(self, RUN, DELETE):
        self.Competitors = LoadMachineList()

        self.Scores = {}
        self.TournamentRounds = self.DefineGames()

        self.ToDeleteLosers = DELETE
        for PLAYER in self.Competitors:
            self.Scores[PLAYER] = 0    

        if RUN:
            T = Thread(target=self.RUNTournament)
            T.start()

    def DefineGames(self):
        ROUNDS = [[]]
        allGames = []

        def searchPlayersInBracket(participants, group):
            found = 0

            for participant in participants:
                for p in group:
                    if type(p) == list:
                        found += searchPlayersInBracket(participants, p)
                    else:
                        if p == participant:
                            found = 1

            return found
            
        for K in range(len(self.Competitors)):
            for T in range(K + 1, len(self.Competitors)):
                allGames.append( [self.Competitors[K], self.Competitors[T]] )

        RoundIndex = 0
        while len(allGames):
            
            if not searchPlayersInBracket(allGames[0], ROUNDS[RoundIndex]):
                ROUNDS[RoundIndex].append( allGames.pop(0) )
                RoundIndex = 0
            else:
                
                RoundIndex += 1
                if RoundIndex >= len(ROUNDS):
                    ROUNDS.append([])


        return ROUNDS

    def DeleteLosers(self):
        Deaths = len(self.Competitors)//4
        Deaths = 1 if not Deaths else Deaths

        for k in range(Deaths):
            Worst = ["", 666]
            KEYS = list(self.Scores.keys())
            for K in KEYS:
                if self.Scores[K] < Worst[1]:
                    Worst[0] = K
                    Worst[1] = self.Scores[K]
                    
            remove("%s/%s" % (settings.TOPmachineDIR, Worst[0]) )
            MachineListLocation = settings.TOPmachineDIR + '/machines.list'
            MachineList = open(MachineListLocation, 'r').readlines()
            for L in range(len(MachineList)):
                if Worst[0] in MachineList[L]:
                    MachineList[L] = 0

            MachineList = [ x for x in MachineList if MachineList ]
                    
            WriteToList = open(MachineListLocation, 'w')
            for line in MachineList:
                WriteToList.write(line)
            WriteToList.close()
                
    def RUNTournament(self):
        MoveInfo = { 0: "move not played.", 1: "move played." }
        TABLEBOARD = [ Table(None, forceNoGUI=True)\
                       for k in range(len(self.TournamentRounds[0])) ]
            
        for ROUND in self.TournamentRounds:
            GOING = 1
            SCORE = [ [0,0] for i in range(len(ROUND)) ]
            DRAWS = [ 0 for i in range(len(ROUND)) ]

            while GOING:
                GOING = 0
                for G in range(len(ROUND)):
                    if TABLEBOARD[G].initialize:
                        GOING = 1
                        
                    elif not TABLEBOARD[G].online:
                        R = TABLEBOARD[G].result
                        if R != None:
                            if R != 0.5:
                                R = round(R)
                                print("Game Ends. %s wins. (%i)" % (settings.COLOR[R], R))
                                SCORE[G][R] += 1
                            else:
                                print("Game Draws. %i" % G)
                                DRAWS[G] += 1
                                
                            TABLEBOARD[G].result = None
                            # print(SCORE)
                                
                        if not abs(SCORE[G][0] - SCORE[G][1]) > 1\
                           and not DRAWS[G] > 3:
                            print("Starting Game at Table %i %s" % (G, ROUND[G]))
                            TABLEBOARD[G].newmatch_thread(specificMatch=ROUND[G])
                            GOING = 1
                    else:
                        x = TABLEBOARD[G].readmove()

                        x = x if x else 0
                        # print('playing %i %s' % ( G,MoveInfo[x] ) )
                        GOING = 1

                # print(TABLEBOARD[0].board)# .board()
                sleep(0.5)
            for GAME in range(len(ROUND)):
                for MAC in range(len(ROUND[GAME])):
                    self.Scores[ROUND[GAME][MAC]] += SCORE[GAME][MAC]
            print("Round Ends. %s" % SCORE)    

        if self.ToDeleteLosers:
            T = Thread(Target=self.DeleteLosers)
            T.start()
