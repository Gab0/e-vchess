#!/bin/python

import chess
import json
import re
import io
import chess.pgn

from random import choice, random, shuffle
from time import sleep



from evchess_evolve import core
from chessArena.enginewrapper import Engine

from chessArena.settings import Settings
Settings = Settings()


class trainingDataFeeder():

    def __init__(self, database_file, engineargs, A_machineDIR=None):
        self.DatabaseFile = database_file
        if '.pgn' in database_file:
            if not self.loadPGNData(database_file):
                return
        else:
            try:
                self.TrialPositions = self.loadDatabase()
            except:
                pass

            if not self.TrialPositions:
                return
        
        self.machineDIR = A_machineDIR if A_machineDIR else settings.machineDIR
        if self.machineDIR:
            engineargs += [ '-MD', self.machineDIR ]
        self.TotalTests = 0
        self.PassedTests = 0

        self.subject = Engine([Settings.enginebin, '--showinfo'] + engineargs)
        sleep(3)
        #self.launchTest()

        self.rollThruMachines()
        self.saveDatabase(self.TrialPositions)


        #print(self.PassedTests)
        self.subject.destroy()
        
    '''
    def loadPGNData(self, PGNfile):
        PGN = open(PGNfile, 'r').read()
        PGN.replace('^M', '')

        separator_indexes = [x for x in re.finditer('\[Event', PGN)]

        MATCHES = []
        for i in range(len(separator_indexes) - 1):
            BEGG = separator_indexes[i].start()
            END = separator_indexes[i + 1].end()
            MATCHES += [PGN[BEGG:END]]

        # print(MATCHES[0])
        # print(len(MATCHES))
        print("Matches loaded.\n")
        self.Matches = MATCHES
        return 1



    def launchTestPGN(self):
        WinnerColor = {'1-0': 'w',
                       '0-1': 'b',
                       '1/2-1/2': 'fail!'}
        Color = {'w': 'white',
                 'b': 'black'}

        game = choice(self.Matches)
        game = chess.pgn.read_game(io.StringIO(game))
        R = game.headers["Result"]

        if R == '1/2-1/2':
            return

        node = game
        R = game.headers["Result"]
        print(R)
        while not node.is_end():
            next_node = node.variation(0)
            # print(node.board().san(next_node.move))

            current_board = node.board()
            nextmove = current_board.san(next_node.move)
            fen = node.board().fen()

            expected_movement = node.board().parse_san(nextmove)

            if random() < 0.1:
                if WinnerColor[R] in fen:
                    break
            node = next_node
        #print(fen)
        expected_movement = str(expected_movement)

        self.subject.send("position %s" % fen)
        #self.subject.send("%s" % Color[WinnerColor[R]])
        self.subject.send("go")


        k = 0

        while True:
            sleep(1)
            k += 1
            #print(k)
            
            response = self.subject.receive()
            for line in response:
                print(line.replace('\n', ''))

                    
                
            move = self.subject.readMove(data=response)
            if move:
                if move == expected_movement:
                    print("good! %s!" % expected_movement)

                else:
                    print("bad! got %s while expecting %s." % (move, expected_movement))
 

                print("\n*********************\n")
                self.TotalTests += 1
                return
            
            if k  > 25:
                print("Timeout.")
                break
        # self.subject.stdout.flush()
        #Response = self.subject.stdout.readlines()

        self.TotalTests += 1
    '''        


    def loadDatabase(self):
        rawdata = open(self.DatabaseFile).read()
        data = json.loads(rawdata)

        return data
        

    def saveDatabase(self, DB):
        dataFile = open(self.DatabaseFile, 'w')

        data = json.dumps(DB, indent=2)
        dataFile.write(data)

        
    def launchTest(self):
        SCORE = 0
        POS = self.TrialPositions
        mortal = len(POS)//2
        for G in range(len(POS)):
            ScoredTest = True
            CurrentPosition = self.TrialPositions[G]
            '''if CurrentPosition["num_succeeded"]:
                if (CurrentPosition["num_tried"] / CurrentPosition["num_succeeded"]) > 0.95: 
                    if CurrentPosition["num_tried"]:
                        if random() > 0.05:
                            continue
                        else:
                            ScoredTest = False'''

            print("sending %s" % self.TrialPositions[G]["pos"])
            print("---[ %i/%i ]---" % (G+1, len(POS)) + " *" * mortal)
            self.subject.send("position %s" % self.TrialPositions[G]["pos"])#####################
            sleep(1.2)
            self.subject.send("go")

            expected_movements = self.TrialPositions[G]["movement"]#.replace('\n', '')#############
            
            TimeWaited = 0
            INFINITE = 999944
            ProbeSpan = 2
            scoreProbe = [[None, -INFINITE] for z in range(ProbeSpan)]
            while True:
                sleep(1)
                TimeWaited += 1
                print()
                response = self.subject.receive()
                for line in response:
                    print(line.replace('\n', ''))
                    
                    scoreReading = line.split(' ')
                    if len(scoreReading) > 4:
                        try:
                            thinkingMoveScore = int(scoreReading[1])
                            for Z in range(ProbeSpan):
                                if thinkingMoveScore > scoreProbe[Z][1]:
                                    for Next in range(ProbeSpan-1, Z, -1):
                                        scoreProbe[Next][1] = scoreProbe[Next-1][1]
                                        scoreProbe[Next][0] = scoreProbe[Next-1][0]
                                        
                                    scoreProbe[Z][1] = thinkingMoveScore
                                    scoreProbe[Z][0] = scoreReading[4]

                                    break
                                else:
                                    raise
                                    pass
                                    #print("denied %i < %i" % (thinkingMoveScore, scoreProbe[Z][1]))
                        except:
                            pass
                    else:
                        #print("*")
                        pass
                move = self.subject.readMove(data=response)                
                
                if move:
                    self.TrialPositions[G]["num_tried"] += 1
                    if move in expected_movements:
                        MOVE_RANK = expected_movements.index(move)
                        NUMBER_ANSWERS = len(expected_movements)
                        print("PASS! %s! at position %i" % (expected_movements[MOVE_RANK], MOVE_RANK+1))
                        if ScoredTest:
                            self.PassedTests += 1
                            SCORE += 5 * (NUMBER_ANSWERS-MOVE_RANK)
                            if not MOVE_RANK:
                                SCORE += 10
                        else:
                            print("not scoring, just testing against easy position.")
                        if not move == scoreProbe[0][0]:
                            print("score probe error!! engine sent %s while read was %s (score: %i)" % (move, scoreProbe[0][0], scoreProbe[0][1]))
                            
                        self.PassedTests += 1
                        self.TrialPositions[G]["num_succeeded"] += 1
                    else:
                        SCORE -= 25
                        print("FAIL! got %s while expecting %s." % (move, ' '.join(expected_movements)))
                        if move == scoreProbe[1][0]:
                            print("but the second move from engine got it right. half point scored")
                            if ScoredTest:
                                SCORE += 0.5
                        #elif move[:2] == expected_movements[:2]:
                        #    print("Correct piece to move. half point scored")
                        #    if ScoredTest:
                        #        SCORE += 0.5
                        
                        # check if it is possible to win the series;
                        if mortal:
                            mortal -= 1
                            if not mortal:
                                return SCORE, False

                        if SCORE + (len(POS) - G) * 10 < 0:
                            print("bailing with score=%i" % SCORE)
                            return SCORE, False

                    print("\n*********************\n")
                    self.TotalTests += 1
                    break
                
                if TimeWaited  > 25:
                    print("Timeout.")
                    break
            # self.subject.stdout.flush()
            #Response = self.subject.stdout.readlines()

            self.TotalTests += 1
        return SCORE, True

    def rollThruMachines(self):
        Approved = {}
        POP = core.loadmachines(DIR=self.machineDIR)
        print("Rolling thru %i machines." % len(POP))
        machinelist = [x.filename for x in POP]
        for machine in machinelist:
            machine = machine.replace('\n', '')
            print(".%s." % machine)
            self.subject.send("load %s" % machine)
            sleep(0.3)
            SCORE, Success = self.launchTest()
            if Success:
                Approved.update({machine: SCORE})


        self.Result = Approved


