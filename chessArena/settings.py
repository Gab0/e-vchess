#!/bin/python
class Settings():
    def __init__(self):
        
        self.COLOR = {0: 'WHITE', 1: 'BLACK'}

        # path to e-vchess executable.
        self.enginebin = "engine/lampreia"

        # paths to the directories where machines are stored.
        self.machineDIR = "machines"
        self.TOPmachineDIR = self.machineDIR + "/top_machines"  # % machineDIR

        # path and arguments to run those chess engines.
        self.engineARGS = [self.enginebin, "-MD", self.machineDIR, "--deep", "3", "--xdeep", "0"]

        # if each movement done should be logged on stdout.
        self.VerboseMove = 0

        self.ArenaRoundInfoRate = 10

        # if arenaArray should load it's graphical interface.
        self.GUI = 1

        # sets the number of simultaneous chess tables to be created and played.
        self.TABLECOUNT = 64
        # number of tables to be shown on each row of machines.
        self.TABLEonROW = 8
        # allow chessarena to plot memory usage on graphic.
        self.arena_showmemuse = 0
        
        self.EvolveRatio = 250
        
        self.standard_popsize = 64 
        self.TournamentPoolSize = 16

        self.ChromosomeSize =100
