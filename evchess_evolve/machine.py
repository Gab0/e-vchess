#!/usr/bin/python

from evchess_evolve.std_parameters import STDPARAMETERS
from evchess_evolve.parameter import parameter
from evchess_evolve.core import Fdir

class machine():

    def __init__(self, fname):
        self.filename = fname
        self.wasmodified = 0


        self.PARAMETERS = STDPARAMETERS()

        self.TPARAMETERS = [
        parameter("stat_games", 1, 0, 0),
        parameter("stat_wins", 1, 0, 0),
        parameter("stat_draws", 1, 0, 0),
        parameter("stat_loss", 1, 0, 0),
        parameter("stat_K", 1, 0, 0),
        parameter("stat_elo", 0,0,1000)]
        

        self.onTOP = 0
        
        self.stat_games = 0
        self.stat_wins = 0
        self.stat_draws = 0
        self.stat_loss = 0
        self.K = 0
        
        self.dumped_games = 0
        self.dumped_wins = 0
        self.dumped_draws = 0
        self.dumped_loss = 0
        self.dumped_K = 0   

    def read(self, split_line):
        for parameter in self.TPARAMETERS+self.PARAMETERS:
            if len(split_line) < 2:
                if split_line[0] == 'W':
                    self.TPARAMETERS[1].value +=1
                    self.TPARAMETERS[0].value +=1
                elif split_line[0] == 'L':
                    self.TPARAMETERS[3].value +=1
                    self.TPARAMETERS[0].value +=1
                elif split_line[0] == 'D':
                    self.TPARAMETERS[2].value +=1
                    self.TPARAMETERS[0].value +=1
                    
            if parameter.name == split_line[0]:
                parameter.read(split_line)

        if 'TOP' in split_line:
            self.onTOP = 1
            
    def write(self):
        Fo = open(Fdir+'/'+self.filename, "w+")
        for parameter in self.PARAMETERS+self.TPARAMETERS:
            Fo.write(parameter.write())
            Fo.write('\n')

        if self.onTOP:
            Fo.write('TOP\n')

        Fo.close()


    def mutate(self, eMOD, AGR):
        print('mutating ' + self.filename)
        for parameter in self.PARAMETERS:
            parameter.mutate(eMOD,AGR)
            parameter.value = parameter.putonlimits(parameter.value)

    def dump_parameter_stat(self):
        for parameter in self.PARAMETERS:
            if parameter.marks_dumpable == 0:
                parameter.dump_parameter_stat(self)

        for i in range(4):
            self.PARAMETERS[i].dumpedvalue = self.PARAMETERS[i].value

    def randomize(self):
        for parameter in self.PARAMETERS:
            parameter.randomize()

    def delete(self):
        os.remove(Fdir+'/'+self.filename)

    def resetscores(self):
        for P in self.TPARAMETERS:
            P.value = 0
