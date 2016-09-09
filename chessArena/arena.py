from random import randrange
from time import time, strftime, sleep

from sys import exit as Exit

from tkinter import *

from threading import Thread
from subprocess import *
import psutil

from chessArena.settings import *
from chessArena.table import Table
from chessArena.tournament import Tournament

from evchess_evolve.advanced import *
from evchess_evolve.core import populate, loadmachines, mutatemachines,\
    setmachines, deltheworst_clonethebest, select_best_inds, Mate,\
    replicate_best_inds, clone_from_template, EliminateEquals

from makegraphic import show_mem_graphic
#from evchess_evolve.management import *

# DEBUG, MEMORY SCAN MODULES;
import objgraph
import resource
from pympler import muppy, asizeof, tracker
import matplotlib.pyplot as plt
from json import dumps
import sys
import tracemalloc
import gc


class Arena():

    def __init__(self, GraphicInterface=True, GO=False):

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

        for i in range(TABLECOUNT):
            if self.GraphicInterface:
                self.TABLEBOARD.append(Table(self, master=self.root))
                self.TABLEBOARD[i].grid(column=k, row=j, stick=NSEW)
            else:
                self.TABLEBOARD.append(Table(self, forceNoGUI=True))

            k += 1
            if k == TABLEonROW:
                k = 0
                j += 1

        self.showhide_ = 0

        self.move_read_reliability = 0

        self.setlooplimit(0)

        self.EvolveRatio = 250

        self.setcounter_illegalmove = 0
        self.setcounter_draws = 0
        self.setcounter_checkmate = 0
        self.setcounter_forcedwin = 0
        self.setcounter_inactivity = 0

        self.graphpath = "mempertime"

        if GO:
            self.setlooplimit(TABLECOUNT - 1)
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
                        "H": [100000, 'mx']}
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
                if arena_showmemuse:
                    self.show_memory_usage()    

            # each N rounds, do maintenance management,
            # in order to get best evolving performance.
            # also prints running info to log.
            if (self.ROUND) and not (self.ROUND % self.EvolveRatio):
                LEVEL = ""

                if not self.ROUND % self.EvolveRatio:
                    LEVEL += "A"
                if not self.ROUND % (self.EvolveRatio * 2):
                    LEVEL += "B"
                if not self.ROUND % (self.EvolveRatio * 3):
                    LEVEL += "C"
                if not self.ROUND % (self.EvolveRatio * 10):
                    LEVEL += "T"
                if not self.ROUND % (self.EvolveRatio // 3 * 13):
                    LEVEL += "H"  # better act alone.

                if LEVEL:
                    for LETTER in LEVEL:
                        self.plotOnGraph(RoutineColor[LETTER][0],
                                         RoutineColor[LETTER][1])

                    self.routine_pop_management(LEVEL)

            self.move_read_reliability = 0
            if not self.ROUND % 10:
                print(" < ROUND %i   %.1fs  Active: %.2f%% > " % (
                    self.ROUND,
                    SLEEPTIME,
                    TABLERESPONSE * 100
                ))
            for t in range(self.looplimit + 1):
                if self.Cycle:
                    if self.TABLEBOARD[t].online:
                        self.TABLEBOARD[t].readmove()
                    else:
                        # maybe this is the cause of the 'memory leak'? if we
                        # recreate the table instance so it stops growing larger on size
                        # after each sucessfull game.
                        #self.TABLEBOARD[t] = 0
                        #self.TABLEBOARD[t] = Table(self, master=self.root)

                        self.TABLEBOARD[t].newmatch()

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

    def killunused(self):
        for T in range(len(self.TABLEBOARD)):
            if T > self.looplimit:
                self.TABLEBOARD[T].endgame()

    def StatLock_routine_management(self):
        population = loadmachines()
        population = CyclingStatLock(population)

        self.log('')
        self.log('>>>>>STATLOCK ROUTINE MANAGEMENT')
        self.log('')

        setmachines(population)

    def routine_pop_management(self, LEVEL):
        if not len(LEVEL):
            return
        population = loadmachines()

        originalPOPLEN = len(population)
        # for individual in population:
        # dump_all_paramstat(individual)

        # for k in range(8):
        #    CHILD = create_hybrid(population)
        #    if CHILD: population.append(CHILD)

        DELTAind = originalPOPLEN // 8

        if "T" in LEVEL:
            sendtoHallOfFame(select_best_inds(population, 1)[0])
            #self.Tournament = Tournament(1,1)
            #self.log('RUNNING TOURNAMENT!')

        if "B" in LEVEL:
            MODscorelimit = 2

            population = deltheworst_clonethebest(population,
                                                  -2 * DELTAind,
                                                  MODscorelimit)

            population = populate(population, DELTAind, 1)

            population = replicate_best_inds(population,
                                             DELTAind // 2)

            population += Mate(select_best_inds(population,
                                                DELTAind // 2), DELTAind)

            population = deltheworst_clonethebest(population,
                                                  originalPOPLEN -
                                                  len(population),
                                                  MODscorelimit)

        if "C" in LEVEL:
            population = EliminateEquals(population, DELTAind)
            population = populate(
                population, originalPOPLEN - len(population), 1)

        if "H" in LEVEL:
            population = deltheworst_clonethebest(population,
                                                  -DELTAind,
                                                  MODscorelimit)
            while len(population) < originalPOPLEN:
                population += clone_from_template()

        # setmachines need to happen before management level C, which is advanced and loads
            # the population by it's own method.
        setmachines(population)
        if "E" in LEVEL:
            ADVmanagement()

        totalgames = (self.setcounter_illegalmove
                      + self.setcounter_forcedwin
                      + self.setcounter_checkmate
                      + self.setcounter_draws + 1)

        if "A" in LEVEL:
            if randrange(10) > 8:
                for k in range(1):
                    population = mutatemachines(1, population)

        AverageElo = 0
        for I in population:
            AverageElo += I.ELO
        AverageElo //= len(population)

        self.log('')
        self.log('>>>>ROUTINE MANAGEMENT %s' % LEVEL)
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
