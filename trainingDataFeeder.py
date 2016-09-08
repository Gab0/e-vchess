#!/bin/python

import chess
import json
import re
import io
import chess.pgn

from random import choice, random
from time import sleep
from subprocess import Popen, PIPE
from fcntl import fcntl, F_GETFL, F_SETFL
from os import O_NONBLOCK, read, system
from chessArena import settings
settings.initialize()

class trainingDataFeeder():
    def __init__(self, PGN):
        if not self.loadPGNData(PGN):
            return

        self.TotalTests=0
        self.PassedTests=0
        
        self.startEngine()

        for k in range(19):
            self.launchTest()

        print(self.PassedTests)
    def loadPGNData(self, PGNfile):
        PGN = open(PGNfile, 'r').read()
        PGN.replace('^M', '')

        separator_indexes = [x for x in re.finditer('\[Event', PGN ) ]

        MATCHES = []
        for i in range(len(separator_indexes)-1):
            BEGG = separator_indexes[i].start()
            END = separator_indexes[i+1].end()
            MATCHES += [ PGN[BEGG:END] ] 

        #print(MATCHES[0])
        #print(len(MATCHES))
        print("Matches loaded.\n")
        self.Matches = MATCHES
        return 1

    def launchTest(self):
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
            #print(node.board().san(next_node.move))
            
            current_board = node.board()
            nextmove = current_board.san(next_node.move)
            fen = node.board().fen()

            expected_movement = node.board().parse_san(nextmove)


            if random() < 0.1:
                if WinnerColor[R] in fen:
                    break
            node = next_node
        print(fen)
        print(expected_movement)

        
        self.subject.stdin.write(bytearray('position %s\n' % fen, 'utf-8'))
        self.subject.stdin.flush()        
        self.subject.stdin.write(bytearray('%s\n' % Color[WinnerColor[R]], 'utf-8'))
        self.subject.stdin.flush()        
        self.subject.stdin.write(bytearray('go\n', 'utf-8'))
        self.subject.stdin.flush()

        
        k=0
        while True:
            sleep(1)
            k+=1
            print(k)
            self.subject.stdout.flush()
            response = self.subject.stdout.readlines()
            for line in response:
                line = line.decode('utf-8')
                print(line)
                if 'move' in line:
                    #print(line)
                    print("*********************")
                    self.TotalTests +=1
                    if str(expected_movement) in line:
                        self.PassedTests +=1

                    return

        #self.subject.stdout.flush()
        #Response = self.subject.stdout.readlines()

        self.TotalTests +=1


    def startEngine(self):
        self.subject = Popen(settings.engineARGS, stdin=PIPE, stdout=PIPE)
        flags = fcntl(self.subject.stdout, F_GETFL)
        fcntl(self.subject.stdout, F_SETFL, flags | O_NONBLOCK)
        
X = trainingDataFeeder('database_2015.pgn')
