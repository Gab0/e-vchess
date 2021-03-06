from random import randrange, choice, random
from time import time, strftime, sleep
import string
from sys import exit as Exit

from tkinter import *

from threading import Thread
from subprocess import *
import psutil
from os import listdir

from chessArena.settings import Settings
from chessArena.table import Table
from chessArena.tournament import Tournament

from evchess_evolve.evolution import *
from evchess_evolve.core import populate, loadmachines, mutatemachines,\
    setmachines, deltheworst_clonethebest, select_best_inds, Mate,\
    replicate_best_inds, clone_from_template, EliminateEquals,\
    EvaluateSimilarityTwoMachines

from makegraphic import show_mem_graphic
#from evchess_evolve.management import *
import signal
# DEBUG, MEMORY SCAN MODULES;
#import objgraph
#import resource
#from pympler import muppy, asizeof, tracker
#import matplotlib.pyplot as plt
#from json import dumps
#import sys
#import tracemalloc
#import gc

settings = Settings()
class Arena():
    def cleanExit(self, signal, frame):
        self.killemall()
        exit(0)
        
    def __init__(self, GraphicInterface=True, GO=False):
        signal.signal(signal.SIGINT, self.cleanExit)
        self.Cycle = False
        self.looplimit = 0
        self.GraphicInterface = GraphicInterface
        self.TIME = time()
        k, j = 0, 0

        if self.GraphicInterface:
            try:
                self.root = Tk()
            except:
                self.GraphicInterface = False
                GO = True

        if self.GraphicInterface:

            self.menubar = Menu(self.root)

            self.Ccycle = self.menubar.add_command(
                label='cycle thru', command=self.startcycle)

            self.menubar.add_command(
                label="Kill 'em All", command=self.killemall)
            self.menubar.add_command(
                label='show/hide ALL',
                command=self.showhideall)

            self.root.config(menu=self.menubar)
        else:
            self.root = None

        self.TABLEBOARD = []

        for i in range(settings.TABLECOUNT):
            if self.GraphicInterface:
                self.TABLEBOARD.append(Table(self, master=self.root))
                self.TABLEBOARD[i].grid(column=k, row=j, stick=NSEW)
            else:
                self.TABLEBOARD.append(Table(self, forceNoGUI=True))

            k += 1
            if k == settings.TABLEonROW:
                k = 0
                j += 1

        self.showhide_ = 0

        self.move_read_reliability = 0

        self.setlooplimit(0)

        self.EvolveRatio = settings.EvolveRatio

        self.setcounter_illegalmove = 0
        self.setcounter_draws = 0
        self.setcounter_checkmate = 0
        self.setcounter_forcedwin = 0
        self.setcounter_inactivity = 0
        
        self.setcounter_createdmachines = 0
        self.setcounter_deletedmachines = 0
        self.setcounter_sentToHallOfFame = 0
        self.ID = ''.join(
            choice(
                string.ascii_uppercase) for _ in range(3))
        
        self.graphpath = "mempertime"
        self.InitTime = time()
        if GO:
            self.setlooplimit(settings.TABLECOUNT - 1)
            self.startcycle()

        if self.GraphicInterface:
            self.Title = "arenaArray"
            self.root.wm_title(self.Title)
            self.root.resizable(False, False)
            self.root.mainloop()
            

    def startcycle(self):

        self.CYCLE = Thread(target=self.gocycle)
        self.CYCLE.start()

    def gocycle(self):
        SLEEPTIME = 1

        self.ROUND = 0
        self.Cycle = True

        RoutineColor = {"A": [20000, 'gx'],
                        "B": [40000, 'rx'],
                        "C": [60000, 'bx'],
                        "T": [80000, 'yx'],
                        "H": [100000, 'mx'],
                        "V": [120000, 'gx']}
        if self.GraphicInterface:
            self.menubar.entryconfigure(1, label='stop cycle')
            self.menubar.entryconfigure(1, command=self.killcycle)

        TIME = time()
        self.TABLEBOARD[0].log("\n\n\nSTARTING CYCLE",
                               str(strftime("%d/%b - %H:%M:%S")))

        # arena/cycle main loop. Important functions and values.
        while self.Cycle:
            TIME = time() - TIME
            TABLERESPONSE = self.move_read_reliability / (self.looplimit + 1)

            if TABLERESPONSE < 0.42:
                SLEEPTIME += 0.1
            if (TABLERESPONSE > 0.81 and SLEEPTIME >= 0.1):
                SLEEPTIME -= 0.1
            SLEEPTIME = round(SLEEPTIME, 1)

            # update window name to reflect current values.
            if not self.ROUND % 3 and self.GraphicInterface:
                self.root.wm_title("%s  T=%s|R=%s|I=%i" % (
                    self.Title,
                    round(SLEEPTIME, 1),
                    self.ROUND,
                    self.setcounter_inactivity
                ))

            if not self.ROUND % 100:
                if settings.arena_showmemuse:
                    self.show_memory_usage()    

            # each N rounds, do maintenance management,
            # in order to get best evolving performance.
            # also prints running info to log.
            if self.ROUND and not self.ROUND % self.EvolveRatio:
                LEVEL = ""

                if not self.ROUND % self.EvolveRatio:
                    LEVEL += "A"
                if not self.ROUND % (self.EvolveRatio * 2):
                    LEVEL += "B"
                if not self.ROUND % (self.EvolveRatio * 3):
                    LEVEL += "C"
                if not self.ROUND % (self.EvolveRatio * 7):
                    LEVEL += "T"
                if not self.ROUND % (self.EvolveRatio // 3 * 13):
                    LEVEL += "H"  # better act alone.

                if not self.ROUND % (self.EvolveRatio * 7):
                    LEVEL += "V"
                if LEVEL:
                    for LETTER in LEVEL:
                        self.plotOnGraph(RoutineColor[LETTER][0],
                                         RoutineColor[LETTER][1])

                    self.routine_pop_management(LEVEL)

            self.move_read_reliability = 0
            if not self.ROUND % settings.ArenaRoundInfoRate:
                print(" < ROUND %i   %.1fs  Active: %.2f%% > " % (
                    self.ROUND,
                    SLEEPTIME,
                    TABLERESPONSE * 100
                ))
            for t in range(self.looplimit + 1):
                if self.Cycle:
                    if not self.TABLEBOARD[t].online:
                        self.TABLEBOARD[t].startEngines()
                        continue
                    if self.TABLEBOARD[t].onGame:
                        self.TABLEBOARD[t].readmove()
                    else:
                        opening = self.selectChessOpening()
                        self.TABLEBOARD[t].newmatch(specificOpening=None)

            if not self.ROUND:
                sleep(0.1)

            self.ROUND += 1

            if self.ROUND > 100000:
                self.ROUND = 1
            sleep(SLEEPTIME)
        return

    def killcycle(self):
        self.Cycle = False
        self.CYCLE.join(0)
        self.CYCLE = None
        # self.cycle["text"] = "cycle thru"
        self.menubar.entryconfigure(1, label='cycle thru')
        # self.cycle["command"] = self.gocycle
        self.menubar.entryconfigure(1, command=self.startcycle)

    def setlooplimit(self, limit):
        # if self.Cycle == True: return

        self.looplimit = limit
        if self.GraphicInterface:
            for i in range(len(self.TABLEBOARD)):
                if i <= self.looplimit:
                    self.TABLEBOARD[i].setlimit["background"] = "green"
                else:
                    self.TABLEBOARD[i].setlimit["background"] = "brown"

    def killemall(self):
        for table in self.TABLEBOARD:
            if table.online == 1:
                table.endgame()
                table.shutdown()

    def killunused(self):
        for T in range(len(self.TABLEBOARD)):
            if T > self.looplimit:
                self.TABLEBOARD[T].endgame()

    def selectChessOpening(self):
        database = ["r1bqkbnr/pppp1ppp/2n5/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 3", #ruy lopez
                    "rnbqkb1r/pppppp1p/5np1/8/2PP4/8/PP2PPPP/RNBQKBNR w KQkq g7 3 3", #king's indian
                    "rnbqkbnr/pppppp1p/6p1/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2" #modern defense
                    ]

        if randrange(10) > 8:
            return choice(database)
        return None                   


    def routine_pop_management(self, LEVEL):
        if not len(LEVEL):
            return
        population = loadmachines()

        originalPOPLEN = settings.standard_popsize #len(population)

        DELTAind = originalPOPLEN // 16

        if "T" in LEVEL:
            halloffame = loadmachines(DIR=settings.HoFmachineDIR)
            halloffame = [mac.filename for mac in halloffame]

            currentbestinds = select_best_inds(population, 18)
            
            for mac in range(len(currentbestinds)):
                if currentbestinds[mac].filename not in halloffame:
                    sendtoHallOfFame(currentbestinds[mac])
                    self.setcounter_sentToHallOfFame += 1
                    break
            #self.Tournament = Tournament(1,1)
            #self.log('RUNNING TOURNAMENT!')

        if "B" in LEVEL:
            MODscorelimit = 2

            population = deltheworst_clonethebest(population,
                                                  -2 * DELTAind,
                                                  MODscorelimit,
                                                  ID=self.ID)
            self.setcounter_deletedmachines += 2 * DELTAind

            population += populate([], DELTAind, True, ID=self.ID)
            self.setcounter_createdmachines += DELTAind

            population += Mate(select_best_inds(population,
                                                DELTAind), DELTAind,ID=self.ID)
            self.setcounter_createdmachines += DELTAind


            numToEquilibrium = originalPOPLEN - len(population)
            if numToEquilibrium > 0:
                self.setcounter_createdmachines += numToEquilibrium
            else:
                self.setcounter_deletedmachines -= numToEquilibrium
            population = deltheworst_clonethebest(population,
                                                numToEquilibrium,
                                                  MODscorelimit,
                                                  ID=self.ID)
            
        if "C" in LEVEL:
            population = EliminateEquals(population, DELTAind)
            NUM = originalPOPLEN - len(population)
            population += populate([], NUM, True, ID=self.ID)
            self.setcounter_deletedmachines += DELTAind
            self.setcounter_createdmachines += NUM
        if "H" in LEVEL:
            population = deltheworst_clonethebest(population,
                                                  -DELTAind,
                                                  MODscorelimit,
                                                  ID=self.ID)
            self.setcounter_deletedmachines += DELTAind
            while len(population) < originalPOPLEN:
                population += clone_from_template(ID=self.ID)
                self.setcounter_createdmachines +=1
        # setmachines need to happen before management level C, which is advanced and loads
            # the population by it's own method.

        if "A" in LEVEL:
            if randrange(10) > 8:
                for k in range(1):
                    population = mutatemachines(1, population)

        if "E" in LEVEL:
            ADVmanagement()

        totalgames = (self.setcounter_illegalmove
                      + self.setcounter_forcedwin
                      + self.setcounter_checkmate
                      + self.setcounter_draws + 1)

  

        if "V" in LEVEL:
            for k in range(len(population)-1):
                for v in range(k+1, len(population)):
                    if not population[k]:
                        break
                    Similarity = EvaluateSimilarityTwoMachines(
                            population[k], population[v]) 
                    if False:
                        bareDeleteMachine(population[k].filename)
                        self.setcounter_deletedmachines +=1
                        print("Excluding %s by similarity to %s." % (
                            population[k].filename, population[v].filename))
                        population[k] = None
                    elif Similarity < 1:
                        print("Mutating %s by similarity." % population[k].filename)
                        population[k].mutate(3,1)


            population = [ x for x in population if x ]

        setmachines(population)
        AverageElo = 0
        for I in population:
            AverageElo += I.ELO
        AverageElo //= len(population)

        self.log('')
        self.log('>> %s >>ROUTINE MANAGEMENT %s' % (self.ID,LEVEL))
        self.log("ROUND = %i. checkmate-> %i; forced wins-> %i; draws-> %i; illegal moves-> %i"
                 % (self.ROUND, self.setcounter_checkmate, self.setcounter_forcedwin, self.setcounter_draws, self.setcounter_illegalmove))
        self.log("initial population size-> %i; final population size-> %i"
                 % (originalPOPLEN, len(population)))
        self.log('Illegal move percentage is %f %%.'
                 % (self.setcounter_illegalmove * 100 / totalgames))
        self.log('Average games per round equals to %i.'
                 % (totalgames // self.ROUND))
        self.log("Average ELO is %i" % AverageElo)
        self.log('')
        print('routine management %s done. Average ELO: %i' %
              (LEVEL, AverageElo))
        self.log("Running Time is %i" % (time() - self.InitTime))
        self.log("Deleted machines: %i; Created machines: %i; Sent to HOF: %i"%\
                (self.setcounter_deletedmachines,
                    self.setcounter_createdmachines,
                    self.setcounter_sentToHallOfFame))

    def showhideall(self):
        for T in self.TABLEBOARD:
            if (T.visible) and (self.showhide_):
                T.shrink_maximize()

            elif not (T.visible) and not (self.showhide_):
                T.shrink_maximize()

        self.showhide_ = 1 - self.showhide_

    def shrinkloop(self):
        self.looplimit -= 3
        self.killunused()

    def log(self, event):
        LOG = open("log.txt", "a+")

        LOG.write(event + " \n")

        LOG.close()

    def show_memory_usage(self):
        memuse = resource.getrusage(resource.RUSAGE_SELF).ru_maxrss
        print("using %.3f GB of ram." % (memuse / 1000000))
        self.plotOnGraph(memuse, 'ro')

        TableMemUsage=0
        for TABLE in self.TABLEBOARD:
            for MAC in TABLE.MACHINE:
                size=0
                pid = MAC.pid()
                proc = psutil.Process(pid)
                TableMemUsage=proc.memory_info()[0]


        self.plotOnGraph(TableMemUsage, 'bo')


    def plotOnGraph(self, value, color):
        graphic_file = open(self.graphpath, 'a')
        graphic_file.write("%i|%i|%s\n" % (self.ROUND,
                                           value,
                                           color))

        graphic_file.close()
