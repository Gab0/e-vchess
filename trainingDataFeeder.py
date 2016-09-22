#!/bin/python

import chess
import json
import re
import io
import chess.pgn

from random import choice, random
from time import sleep




from chessArena.enginewrap import Engine
from chessArena import settings
settings.initialize()


class trainingDataFeeder():

    def __init__(self, PGN):
        if not self.loadPGNData(PGN):
            return

        self.TotalTests = 0
        self.PassedTests = 0

        self.subject = Engine(settings.engineARGS+['--showinfo'])
        sleep(4)
        self.subject.receive()
        for k in range(19):
            self.launchTest()

        print(self.PassedTests)
        self.subject.destroy()
        
    def generateRandomBoard(self):
        board = chess.Bitboard()
        
        def random_move(board):
            return random.choice(list(board.legal_moves))

        for k in range(round(gauss(3,5)*3)):
            board.push(random_move(board))

        return board.fen()
    def runDataColledtor(self):
        pass
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
        self.subject.send("%s" % Color[WinnerColor[R]])
        self.subject.send("go")


        k = 0
        while True:
            sleep(1)
            k += 1
            #print(k)
            
            response = self.subject.receive()
            for line in response:
                print(line.replace('\n', ''))
                pass
            move = self.subject.readMove(data=response)
            if move:
                if move == expected_movement:
                    print("good! %s!" % expected_movement)
                    self.PassedTests += 1

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

X = trainingDataFeeder('database_2015.pgn')
