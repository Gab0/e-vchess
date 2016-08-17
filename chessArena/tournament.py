#!/bin/python
import curses
from time import sleep
from json import loads, dumps
from os import remove, path, listdir
from threading import Thread
from random import shuffle, choice
from chessArena import settings
settings.initialize()

from chessArena.table import Table


def LoadMachineList():
    MachineListLocation = settings.TOPmachineDIR + '/machines.list'
    MachineList = open(MachineListLocation, 'r').readlines()

    LegalMachines = []
    for line in MachineList:
        if '.mac' in line:
            LegalMachines.append(line[:-1])

    return LegalMachines


class Tournament():
    def __init__(self, scr, RUN, DELETE):
        if scr:
            self.mainscr = scr
            self.stdscr = curses.newpad(1024, 768)
        else:
            self.mainscr = None
            self.stdscr = None

            
        self.Competitors = LoadMachineList()

        self.Scores = {}
        self.TournamentRounds = self.DefineGames(len(self.Competitors))

        self.ToDeleteLosers = DELETE
        for PLAYER in self.Competitors:
            self.Scores[PLAYER] = 0    

        if RUN:
            T = Thread(target=self.RUNTournament)
            T.start()

    def DefineGames(self, NumberOfRounds):
        ROUNDS = [[] for r in range(NumberOfRounds) ]
        allGames = []

        def searchPlayersInBracket(participants, Round):
            found = 0

            for participant in participants:
                if found:
                    break
                for match in Round:
                    if participant in match:
                        found = 1
                        break

            return found

        def countUniqueMachines(Round):
            space = []

            for game in Round:
                for player in game:
                    if not player in space:
                        space.append(player)

            # print("space len %i " % len(space))
            return space

        for T in range(len(self.Competitors)):
            for K in range(T + 1, len(self.Competitors)):
                allGames.append([ self.Competitors[T], self.Competitors[K] ])

        RoundIndex = 0        
        while len(ROUNDS[-1]) < len(self.Competitors)//2:
            x = choice(allGames)
            if not searchPlayersInBracket(x, ROUNDS[RoundIndex]):
                ROUNDS[RoundIndex].append(x)
                RoundIndex += 1
                if RoundIndex == len(ROUNDS):
                    RoundIndex = 0



        shuffle(ROUNDS)      
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

            self.Scores.pop(Worst[0])
    
            MachineListLocation = settings.TOPmachineDIR + '/machines.list'
            MachineList = open(MachineListLocation, 'r').readlines()
            for L in range(len(MachineList)):
                if Worst[0] in MachineList[L]:
                    MachineList[L] = 0

            MachineList = [ x for x in MachineList if x ]
                    
            WriteToList = open(MachineListLocation, 'w')
            for line in MachineList:
                WriteToList.write(line)
            WriteToList.close()
                
    def RUNTournament(self):

        # print([ len(self.TournamentRounds[z]) for z in range(len(self.TournamentRounds)) ] )

        xDeepValue = 0
        MoveInfo = { 0: "move not played.", 1: "move played." }
        self.TABLEBOARD = [ Table(None, forceNoGUI=True)\
                       for k in range(len(self.TournamentRounds[0])) ]

        for ROUND in self.TournamentRounds:
            GOING = 1
            
            ACTIVE =[ True for i in range(len(ROUND)) ]
            SCORE = [ [0,0] for i in range(len(ROUND)) ]
            DRAWS = [ 0 for i in range(len(ROUND)) ]
            I=0

            while True:
                for G in range(len(ROUND)):
                 
                    try:
                        self.TABLEBOARD[G]    
                    except IndexError:
                        print(self.TABLEBOARD)
                        print([ len(self.TournamentRounds[z]) \
                                for z in range(len(self.TournamentRounds)) ] )
                        raise
                    
                    if self.TABLEBOARD[G].initialize:
                            pass
                    elif not self.TABLEBOARD[G].online:
                        R = self.TABLEBOARD[G].result
                        if R != None:
                            nameTAG = "[%s x %s]" % (self.TABLEBOARD[G].MACnames[0],
                                                     self.TABLEBOARD[G].MACnames[1])
                            if R != 0.5:
                                R = round(R)
                                print("Game Ends. %s wins. (%i) %s" % (settings.COLOR[R],
                                                                              R,
                                                                              nameTAG))
                                SCORE[G][R] += 1
                            else:
                                print("Game Draws. %i %s" % (G, nameTAG))
                                SCORE[G][0] += 0.5
                                SCORE[G][1] += 0.5

                                DRAWS[G] += 1
                                
                            self.TABLEBOARD[G].result = None
                            # print(SCORE)
                                
                        if not abs(SCORE[G][0] - SCORE[G][1]) > 1\
                           and not DRAWS[G] > 1:
                            print("Starting Game at Table %i [%s x %s]" % (G,
                                                                    ROUND[G][0],
                                                                    ROUND[G][1]))

                            engineCommand = [[settings.enginebin,
                                              '-MD', settings.TOPmachineDIR,
                                              '--deep', '4',
                                              '--xdeep', str(xDeepValue),
                                              '--specific'] for k in range(2)]
                            engineCommand[0] += [ROUND[G][0]]
                            engineCommand[1] += [ROUND[G][1]]
                            #for k in engineCommand: print (" ".join(k))
                            self.TABLEBOARD[G].newmatch_thread(specificMatch=engineCommand)
                            
                        else:
                            ACTIVE[G] = False
                    else:
                        
                        x = self.TABLEBOARD[G].readmove()
                        x = x if x else 0
                        # print('playing %i %s' % ( G,MoveInfo[x] ) )
                        

                toEndRound=1
                for k in ACTIVE:
                    if k:
                        toEndRound=0

                if toEndRound:
                    break
                    
                if not I % 10:
                    self.showStatus(I, SCORE, ACTIVE, DRAWS, ROUND)
                sleep(0.5 + xDeepValue * 5)
                I+=1
                
            for GAME in range(len(ROUND)):
                for MAC in range(len(ROUND[GAME])):
                    self.Scores[ROUND[GAME][MAC]] += SCORE[GAME][MAC]
                    
            #for T in self.TABLEBOARD:
            #    T.endgame()
            print("Round Ends. %s" % SCORE)
            
            # to end tournament prematurely if one machine is really crap.
            if I > I//2:
                for K in self.Scores:
                    if not self.Scores[K]:
                        print("Ending tournament early. %s got zero points so far." % self.Scores[K])
                        self.cleanup()
                        break
                    
        # delete losers of tournament.
        if self.ToDeleteLosers:
            T = Thread(target=self.DeleteLosers)
            T.start()

        print("Tournament Ends.")
        
    def showStatus(self, RoundIndex, SCORE, ACTIVE, DRAWS, ROUND):
        if self.stdscr:
            self.stdscr.clear()
        ActiveSymbol={True: 'o', False: 'x'}
        print( "\nPlay %i of Round %i/%i." % (RoundIndex,
                                              self.TournamentRounds.index(ROUND)+1,
                                              len(self.TournamentRounds)) )
        I=0
        for iTABLE in self.TABLEBOARD:
            try:
                TableInfo = "{%i} %s %ix%i %s {%i} (%i)   %s" % (
                    self.Scores[iTABLE.MACnames[0]],
                    iTABLE.MACnames[0],
                    SCORE[I][0],
                    SCORE[I][1],
                    iTABLE.MACnames[1],
                    self.Scores[iTABLE.MACnames[1]],
                    DRAWS[I],
                    ActiveSymbol[ACTIVE[I]])
                
                TableBoard = str(iTABLE.board)

                if self.stdscr:
                    self.stdscr.addstr(I*10, 0 ,TableInfo)
                    self.stdscr.addstr(I*10+1,0,TableBoard)
                else:
                    print(TableInfo)
                    print(TableBoard)
                    
            except AttributeError:
                print("[NOT ONLINE?]")
                pass

            except KeyError:
                print(" OFFLINE ")
                pass

            I+=1
        print("")
        print("number of engine processes: %i / maximum expected: %i" % \
              (checknumberofPROCS(),
               len(self.TABLEBOARD) *2))
        
        print("\n\n")
        if self.stdscr:
            self.mainscr.refresh()
        
    def cleanup(self):
        for TABLE in self.TABLEBOARD:
            TABLE.endgame()
            
def checknumberofPROCS():
    pids = [pid for pid in listdir('/proc') if pid.isdigit()]
    ENGINE_COUNT = 0
    for pid in pids:
        try:
            PROC = open(path.join('/proc', pid, 'cmdline'),
                        'rb').read().decode('utf-8', 'ignore')
            # print(PROC)
            if "e-vchess" in PROC:
                ENGINE_COUNT += 1
                
        except IOError:  # proc has already terminated
            continue


    return ENGINE_COUNT
