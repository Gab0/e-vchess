#!/bin/python
import curses
from time import sleep
from json import loads, dumps
from os import remove, path, listdir
from threading import Thread
from random import shuffle, choice
from time import time
from copy import copy
import signal
from evchess_evolve.management import bareDeleteMachine
from evchess_evolve.core import loadmachines

from chessArena.settings import Settings
settings = Settings()

from chessArena.table import Table

xDeepValue = 0
yDeepValue = 0 
class Tournament():

    def __init__(self, scr, RUN, DELETE):
        if scr:
            self.mainscr = scr
            self.stdscr = curses.newpad(1024, 768)
        else:
            self.mainscr = None
            self.stdscr = None

        self.verboseBoards = False

        signal.signal(signal.SIGINT, self.SIGINTFinishAndQuit) 

        self.Competitors = loadmachines(DIR=settings.TOPmachineDIR)
        shuffle(self.Competitors)
        self.Competitors = self.Competitors[:settings.TournamentPoolSize]
        
        self.TotalDead = 0
        self.ToDeleteLosers = DELETE
        for PLAYER in self.Competitors:
            PLAYER.TournamentScore = 0

        self.Deaths = len(self.Competitors) // 2
        L = len(self.Competitors)//2
        self.MaxTableboardSize = L if L < 7 else 7

        print("%i competitors." % len(self.Competitors))
        self.EngineCommand = [settings.enginebin,
                              '-MD', settings.TOPmachineDIR,
                              '--deep', '4',
                              '--xdeep', str(xDeepValue),
                              '--ydeep', str(yDeepValue),
                              '--specific']

        self.TABLEBOARD = [Table(None, forceNoGUI=True)
                          for k in range(self.MaxTableboardSize)]
        if RUN:
            self.TypeOfTournament = RUN
            print("\n\n")
            T = None
            if RUN == "tournament":
                T = Thread(target=self.ProperTournament)
            elif RUN == "killemall":
                T = Thread(target=self.KillEmAllTournament)
            if T:
                T.start()

    def DefineGames(self, NumberOfRounds):
        ROUNDS = [[] for r in range(NumberOfRounds)]
        allGames = []

        def searchPlayersInBracket(participants, Round):
            found = 0

            for participant in participants:
                if found:
                    break
                for match in Round:
                    for matchplayer in match:
                        if participant.filename == matchplayer.filename:
                            found = 1
                            break

            return found


        for T in range(len(self.Competitors)):
            for K in range(T + 1, len(self.Competitors)):
                allGames.append([self.Competitors[T], self.Competitors[K]])

        RoundIndex = 0
        U = 0
 
        while len(ROUNDS[-1]) < self.MaxTableboardSize  and U < 1000:
            x = choice(allGames)
            if not searchPlayersInBracket(x, ROUNDS[RoundIndex]):
                ROUNDS[RoundIndex].append(x)
                RoundIndex += 1
                if RoundIndex == len(ROUNDS):
                    RoundIndex = 0
                    
            U += 1

        shuffle(ROUNDS)
        return ROUNDS

    def DeleteLosers(self):
        for machine in self.Competitors:
            print("%s: %s" % (machine.filename, machine.TournamentScore))

        self.Deaths = 1 if not self.Deaths else self.Deaths

        for k in range(self.Deaths):
            Worst = ["", 666]

            for K in self.Competitors:
                if K.TournamentScore < Worst[1]:
                    Worst[0] = K.filename
                    Worst[1] = K.TournamentScore


            print("Deleting %s; %i points." % (Worst[0], Worst[1]))

            bareDeleteMachine(settings.TOPmachineDIR, Worst[0])
            self.Competitors = [
                    x for x in self.Competitors if x.filename != Worst[0] ]
            self.TotalDead += 1
    def ProperTournament(self):

        self.TournamentRounds = self.DefineGames(len(self.Competitors))
 
        for ROUND in self.TournamentRounds:
            CURRENT = self.TournamentRounds.index(ROUND)
            GOING = 1
            END = 0

            SCORE = self.RunTournamentRound(ROUND,
                    CURRENT, len(self.TournamentRounds))

            print("Round Ends. %s" % SCORE)
            for GAME in range(len(ROUND)):
                for MAC in range(len(ROUND[GAME])):
                    ROUND[GAME][MAC].TournamentScore += SCORE[GAME][MAC]

            # to end tournament prematurely if one machine is really crap.
            if CURRENT > (len(self.TournamentRounds) / 4):
                if not self.checkFateContinue(
                        len(self.TournamentRounds) - CURRENT):
                    print("Tournament early ending conditions met. ")
                    self.cleanup()
                    break

        # delete losers of tournament.
        if self.ToDeleteLosers:
            self.DeleteLosers()

        print("Tournament Ends.")
        
    def KillEmAllCheckDead(self, Score, Round):
        for MACHINE in range(len(Round)):
            if Score[1-MACHINE] - Score[MACHINE] >= 2:
                deadmac = Round[MACHINE]
                self.log("%s dies. [%i]" % (deadmac.filename,
                    deadmac.ELO))
                try:
                    self.Competitors.pop(self.Competitors.index(deadmac))
                except:
                    print("Tried to kill %s but it's already deleted." % deadmac.filename)
                    return
                bareDeleteMachine(settings.TOPmachineDIR, deadmac.filename)
                
                print("%s dies." % deadmac.filename)
                self.TotalDead += 1
            
    def KillEmAllTournament(self):
        RemovedMachineCount = 0
        I=0
        while len(self.Competitors) > 3:
            print("Starting ffa tournament round, with %i competitors." %\
                  (len(self.Competitors)))
            ROUND = self.DefineGames(1)[0]
            SCORE = self.RunTournamentRound(ROUND, I, 0)
            for GAME in range(len(ROUND)):
                self.KillEmAllCheckDead(SCORE[GAME], ROUND[GAME])
               
            I+=1
        print("Ending bloodbath. removed count: %i" % RemovedMachineCount)

    def RunTournamentRound(self, ROUND, ThisRoundNumber, ExpectedRoundNumber):
        for MAC in self.Competitors:
            MAC.Load()
        ACTIVE = [True for i in ROUND]
        SCORE = [[0, 0] for i in ROUND]
        DRAWS = [0 for i in ROUND]
        GAMELENGHT = [0 for i in ROUND]
        elapsed = 0
        I = 0

        last_time = time()
        while True:
            STATUS = [ "-" for i in ACTIVE ]
            for G in range(len(ROUND)):
                machineFilenames = [ x.filename for x in ROUND[G] ]

                try:
                    self.TABLEBOARD[G]
                except IndexError:
                    print(self.TABLEBOARD)
                    print([len(self.TournamentRounds[z])
                           for z in range(len(self.TournamentRounds))])
                    raise

                if self.TABLEBOARD[G].initialize:
                    print("intializing")
                    continue

                if not self.TABLEBOARD[G].online:
                    self.TABLEBOARD[G].startEngines([settings.enginebin, '-MD', settings.machineDIR+'/top_machines', '--deep', '4'])
                    continue

                elif not ACTIVE[G]:
                    # CREATE NEW GAME ON THE FLY.
                    if self.TypeOfTournament == "killemall":

                        ROUND[G][0].TournamentScore += SCORE[G][0]
                        ROUND[G][1].TournamentScore += SCORE[G][1]
                        
                        self.KillEmAllCheckDead(SCORE[G], ROUND[G])
                        DRAWS[G] = 0
                        SCORE[G] = [0,0]
                        ACTIVE[G] = True

                        ROUND[G] = self.DefineGames(1)[0][0]
                        machineFilenames = [ x.filename for x in ROUND[G] ]
                        self.TABLEBOARD[G].newmatch(specificMachines=machineFilenames)
                    else:
                    # NORMAL METHOD, WAIT FOR ROUND TO FINISH.
                        continue
                    
                elif GAMELENGHT[G] > 46:
                    self.TABLEBOARD[G].onGame = 0

                    PIECES = [0,0]
                    for p in range(64):
                        piece = self.TABLEBOARD[G].board.piece_at(p)
                        if piece != None:
                            piece = 0 if piece.color else 1

                            PIECES[piece] += 1

                    print("drawn game by movelenght "+\
                          ("WHITE got %i pieces;" % PIECES[0]) +\
                          (" while BLACK got %i." % PIECES[1]))
                    wonbyadvantage = 0
                    for k in [0,1]:
                        if PIECES[k] > 3 and PIECES[1-k] == 1:
                            self.TABLEBOARD[G].result = k
                            print("%i wins by piece advantage." % k)
                            wonbyadvantage = 1
                            print(self.TABLEBOARD[G].board)

                    if not wonbyadvantage:
                        self.TABLEBOARD[G].result = 0.5
                        
                if not self.TABLEBOARD[G].onGame:
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
                        self.TABLEBOARD[G].endgame()
                        
                        # print(SCORE)

                    if abs(SCORE[G][0] - SCORE[G][1]) <= 1\
                       and not DRAWS[G] > 0:
                        GAMELENGHT[G]=0
                        print("Starting Game at Table  +\
                               %i [%s x %s]" % (G, machineFilenames[0], machineFilenames[1]))
                        self.TABLEBOARD[G].newmatch(specificMachines=machineFilenames)
                        #ACTIVE[G] = True


                    else:
                        ACTIVE[G] = False
                else:

                    x = self.TABLEBOARD[G].readmove()
                    
                    x = x if x else 0
                    if x:
                        STATUS[G] = "+"
                        GAMELENGHT[G]+=1
                    
            print(' '.join(STATUS))
            if not I % 10:
                elapsed += time() - last_time
                last_time = time()
                self.showStatus(I,
                                ThisRoundNumber,
                                ExpectedRoundNumber,
                                SCORE,
                                ACTIVE,
                                DRAWS,
                                ROUND,
                                elapsed)
            sleep(1 + xDeepValue * 1.5)
            I += 1

            if not True in ACTIVE:
                for T in self.TABLEBOARD:
                    for M in T.MACHINE:
                        M.destroy()
                return SCORE
            



        
    def showStatus(self, I, RoundIndex, TotalRounds, SCORE,
                   ACTIVE, DRAWS, ROUND, elapsed):
        if self.stdscr:
            self.stdscr.clear()
        ActiveSymbol = {True: 'o', False: 'x'}
        if self.TypeOfTournament == "killemall":
            print("\n Play %i; %i deads already.\n" % (I, self.TotalDead))
        else:
            print("\nPlay %i of Round %i/%i.  t: %is" %
                  (I,
                   RoundIndex,
                   TotalRounds,
                   elapsed))

        I = 0
        for iTABLE in self.TABLEBOARD:
            if I>len(SCORE)-1:
                continue
            try:
                score = [0,0]
                for k in [0,1]:
                    for mac in self.Competitors:
                        if iTABLE.MACnames[k] == mac.filename:
                            score[k] = mac.TournamentScore
                TableInfoMA = "{%.1f} %s %.1f" % (
                    score[0],
                    iTABLE.MACnames[0],
                    SCORE[I][0])
                TableInfoMB = "%.1f %s {%.1f}" % (
                   SCORE[I][1],
                    iTABLE.MACnames[1],
                    score[1])

                TableInfo = "%s%sx%s%s   (%i)    %s" % (
                        " " * (28-len(TableInfoMA)),
                        TableInfoMA,
                        TableInfoMB,
                        " " * (28-len(TableInfoMB)),
                        DRAWS[I],
                        ActiveSymbol[ACTIVE[I]])

                TableBoard = str(iTABLE.board)

                if self.stdscr:
                    self.stdscr.addstr(I * 10, 0, TableInfo)
                    self.stdscr.addstr(I * 10 + 1, 0, TableBoard)
                else:
                    print(TableInfo)
                    if self.verboseBoards:
                        print(TableBoard)

            except AttributeError:
                print("[NOT ONLINE?]")
                pass

            except KeyError:
                print(" OFFLINE ")
                pass

            I += 1
        print("")
        print("number of engine processes: %i / maximum expected: %i" %
              (checknumberofPROCS(),
               len(self.TABLEBOARD) * 2))

        print("\n\n")
        if self.stdscr:
            self.mainscr.refresh()

    def cleanup(self):
        for TABLE in self.TABLEBOARD:
            TABLE.endgame()

    def checkFateContinue(self, remainingRounds):
        PossiblePoints = remainingRounds * 1.7
        WORST = 0
        POORBEST = 1000
        LASTPOORBEST = 0

        for DEATH in range(self.Deaths + 1):
            LASTPOORBEST = POORBEST
            POORBEST = 1000
            for MACHINE in self.Competitors:
                SCORE = MACHINE.TournamentScore
                if SCORE <= WORST:
                    continue
                elif SCORE < POORBEST:
                    POORBEST = SCORE
            WORST = POORBEST

        # print("LPB: %i; PB: %i" % (LASTPOORBEST, POORBEST))

        if LASTPOORBEST + PossiblePoints > POORBEST:
            return True
        else:
            return False

    def log(self, text):
        logfile = open("log.txt", "a")
        logfile.write(text)
        logfile.close()

    def SIGINTFinishAndQuit(self, signum, frame):
        self.cleanup()
        exit()
        
def checknumberofPROCS():
    pids = [pid for pid in listdir('/proc') if pid.isdigit()]
    ENGINE_COUNT = 0
    for pid in pids:
        try:
            PROC = open(path.join('/proc', pid, 'cmdline'),
                        'rb').read().decode('utf-8', 'ignore')
            # print(PROC)
            if "engine/e-vchess" in PROC:
                ENGINE_COUNT += 1

        except IOError:  # proc has already terminated
            continue

    return ENGINE_COUNT
