#!/usr/bin/python

from evchess_evolve.std_parameters import STDPARAMETERS
from evchess_evolve.parameter import parameter
from evchess_evolve.core import machine_dir

from random import choice, randrange

class machine():

    def __init__(self, fname, DIR=machine_dir):
        self.DIR = DIR
        self.filename = fname
        self.wasmodified = 0

        self.PARAMETERS = STDPARAMETERS()

        self.TPARAMETERS = [
            parameter("stat_games", 1, 0, 0),
            parameter("stat_wins", 1, 0, 0),
            parameter("stat_draws", 1, 0, 0),
            parameter("stat_loss", 1, 0, 0),
            parameter("stat_K", 1, 0, 0),
            parameter("real_world_score", 1, 0, 0)
        ]

        self.ELO = 1000
        self.onTOP = 0

        self.stat_games = 0
        self.stat_wins = 0
        self.stat_draws = 0
        self.stat_loss = 0
        self.K = 0


    def Load(self):
        try:
            selfContent = open("%s/%s" % (self.DIR, self.filename))
        except:
            return
        for line in selfContent.readlines():
            self.read(line.split(" "))
    def read(self, split_line):
        for parameter in self.TPARAMETERS + self.PARAMETERS:
            if len(split_line) < 2:
                if split_line[0] == 'W':
                    self.TPARAMETERS[1].value += 1
                    self.TPARAMETERS[0].value += 1
                elif split_line[0] == 'L':
                    self.TPARAMETERS[3].value += 1
                    self.TPARAMETERS[0].value += 1
                elif split_line[0] == 'D':
                    self.TPARAMETERS[2].value += 1
                    self.TPARAMETERS[0].value += 1

            if parameter.name == split_line[0]:
                parameter.read(split_line)

        if 'TOP' in split_line:
            self.onTOP = 1

        if 'stat_elo' in split_line:
            self.ELO = int(split_line[2])

    def write(self):
        Fo = open(self.DIR + '/' + self.filename, "w+")
        for parameter in self.PARAMETERS + self.TPARAMETERS:
            Fo.write(parameter.write())
            Fo.write('\n')

        if self.onTOP:
            Fo.write('TOP\n')

        Fo.write('stat_elo = %i\n' % self.ELO)

        Fo.close()
        
    def toJson(self):
        P = {}

        for T in self.PARAMETERS:
            P.update({T.name: T.value})
        
        O = {self.filename: T}

    def mutate(self, MutateProbabilityDamper, Aggro):
        print("mutating %s [ MPD: %i, ELO: %i ] " % (self.filename,
                                                     MutateProbabilityDamper,
                                                     self.ELO))
        for parameter in self.PARAMETERS:
            parameter.mutate(MutateProbabilityDamper, Aggro)

    def randomize(self):
        for parameter in self.PARAMETERS:
            parameter.randomize()

    def delete(self):
        DataLocation = "%s/%s" %(self.DIR, self.filename)
        os.remove(DataLocation)

    def resetscores(self):
        for P in self.TPARAMETERS:
            P.value = 0

    def getParameter(self, paramname, toSUM=0):
        for k in self.PARAMETERS + self.TPARAMETERS:
            if k.name == paramname:
                if not toSUM:
                    return k.value
                else:
                    k.value += toSUM
                    return k.value

    def generateOwnChromosomes(self):
        Chromosome = ""

        for P in self.PARAMETERS:
            V = P.toGene()
            Chromosome += P.promoter
            Chromosome += V
            spacer = ''.join([choice(['0','1']) for k in range(randrange(32))])
            Chromosome += spacer

        self.Chromosomes = [Chromosome] * 2
        return Chromosome

    def readOwnChromosomes(self):
        for P in self.PARAMETERS:
            PositionsOnChromosomes = [c.index(P.promoter) for c in self.Chromosomes]
            Values=[]
            for c in range(len(self.Chromosomes)):
                coordinates = [ PositionsOnChromosomes[c], PositionsOnChromosomes[c]+8 ] 
                ChromosomeRegion = self.Chromosomes[c][coordinates[0]:coordinates[1]]
                Values.append(ChromosomeRegion)
            
        
            
        
