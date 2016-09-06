#!/bin/python

import chess
import json
import re
import io
import chess.pgn

from random import choice


from chessArena import settings
settings.initialize()

class trainingDataFeeder():
    def __init__(self, PGN):
        if not self.loadPGNData(PGN):
            return

        a = trainingDataMatch(self.Matches[0])
        print(a.DATA.headers["Result"])
        #DATA = self.loadTrainingData()
        #self.feedTrainingData(DATA)
        
    def loadTrainingData(self):
        if not os.path.isdir("traindata"):
            print('Directory not found.')
            return
        Files = os.listdir('traindata')
        Fn = open('traindata/%s' % choice(Files), 'r').read()

        DATA = json.loads(Fn)

        # print(DATA['score'])
        # print(DATA['moveline'])
        # print(DATA['board'])

        return DATA

    def feedTrainingData(self, DATA):
        if not DATA:
            print("NO DATA")
            return

        OutgoingArray = [DATA['board']]
        _BOARD = chess.Board(DATA['board'])

        # print(DATA['moveline'])
        for M in DATA['moveline'].split(" "):
            MOVE = _BOARD.parse_san(M)
            _BOARD.push(MOVE)
            OutgoingArray.append(_BOARD.fen())

        print(DATA['score'])
        for X in OutgoingArray:
            print(X)
            
        return OutgoingArray

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
        game = choice(self.Matches)
        game = chess.pgn.read_game(io.StringIO(game))
        
        
        
X = trainingDataFeeder('database_2015.pgn')
