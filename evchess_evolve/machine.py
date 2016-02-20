#!/usr/bin/python

class machine ():

    def __init__(self, fname):
        self.filename = fname
        self.wasmodified = 0


        self.TPARAMETERS = []
        self.PARAMETERS = []

        self.TPARAMETERS.append(parameter("stat_games", 1, 0, 0))
        self.TPARAMETERS.append(parameter("stat_wins", 1, 0, 0))
        self.TPARAMETERS.append(parameter("stat_draws", 1, 0, 0))
        self.TPARAMETERS.append(parameter("stat_loss", 1, 0, 0))
        self.TPARAMETERS.append(parameter("stat_K", 1, 0, 0))

        self.TPARAMETERS.append(parameter("stat_elo", 0,0,1000))
        


        self.PARAMETERS.append(parameter("param_DEEP", 0, 30, 2, bLIM=4, INCR=2, LIM=10))

        self.PARAMETERS.append(parameter("eval_randomness", 0, 30, 60, INCR=10, bLIM=1))
        self.PARAMETERS.append(parameter("param_seekpieces", 0, 30, 1, bLIM=0, INCR=0.25, LIM=3))
        #self.PARAMETERS.append(parameter("param_deviationcalc", 0, 30, 0.1, INCR=0.2))
        self.PARAMETERS.append(parameter("param_evalmethod", 0, 30, 0, aP=1, bLIM=0, LIM=0))
        self.PARAMETERS.append(parameter("param_seekatk", 0, 30, 0.5, bLIM=-1, INCR=0.25, LIM=3))
        self.PARAMETERS.append(parameter("param_seekmiddle", 0, 30, 3, LIM = 5))
        self.PARAMETERS.append(parameter("param_presumeOPPaggro", 0, 30, 1, LIM=1.1, bLIM=1, INCR=0.1))
        self.PARAMETERS.append(parameter("param_pawnrankMOD", 0, 30, 1, LIM=37))
        self.PARAMETERS.append(parameter("param_parallelcheck", 0, 80, 4,LIM=21, bLIM=0))
        self.PARAMETERS.append(parameter("param_balanceoffense", 0, 80, 1,LIM=5, bLIM=0))
        #self.PARAMETERS.append(parameter("param_cumulative", 0, 80, 0,LIM=1, bLIM=0, INCR=0.1))
        
        #self.PARAMETERS.append(parameter("param_pvalues", 0, 5, [100,500,300,300,900,2000], INCR=50, bLIM=70, LIM=2500, locked=1))
        #self.PARAMETERS.append(parameter("param_TIMEweight", 0, 30, [0.9, 0.85, 0.9, 0.85, 0.81, 0.765, 0.825, 0.789, 0.844, 0.85], LIM=1.3, bLIM=0.01, INCR = 0.05))


 

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
            parameter.randomize
            

    def delete(self):
        os.remove(Fdir+'/'+self.filename)

    def resetscores(self):
        for P in self.TPARAMETERS:
            P.value = 0
