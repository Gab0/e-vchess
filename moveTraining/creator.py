#!/bin/python
from chessArena.enginewrapper import Engine
import chess
import random
from time import sleep
import chess.pgn
import re
import io

class trainingDataCreator():
    def __init__(self, PGN_dataBase=None, SimpleDatabase=None, NewMovements=None):
        self.database_size = 32
        self.DatabaseName = SimpleDatabase
        self.PGN_dataBase = PGN_dataBase
        self.NewMovements=NewMovements
        if self.NewMovements:
            self.load_SimpleDatabase(SimpleDatabase)
            self.refreshDatabase()
            return
        elif self.PGN_dataBase:
            self.loadPGN_database()
        elif SimpleDatabase:
            self.load_SimpleDatabase(SimpleDatabase)
        self.runDataCollector()
        
    def generateRandomBoard(self):
        board = chess.Board()
        
        def random_move(board):
            return random.choice(list(board.legal_moves))

        for k in range(round(random.gauss(5,3)*3)):
            board.push(random_move(board))

        return board.fen()

    def fetchBoardFromDatabase(self):
        game = random.choice(self.Matches)
        game = chess.pgn.read_game(io.StringIO(game))

        BoardStateNumber = random.randrange(9,25)
        while not game.is_end() and BoardStateNumber > 0:
            next_node = game.variation(0)
            current_board = game.board()
            nextmove = current_board.san(next_node.move)
            BoardStateNumber -= 1
            game = next_node
            
        if not BoardStateNumber:
            fen = game.board().fen()
            return fen
        else:
            return self.fetchBoardFromDatabase()
        
    def loadPGN_database(self):
        PGN = open(self.PGN_dataBase, 'r').read()
        PGN.replace('^M', '')

        separator_indexes = [x for x in re.finditer('\[Event', PGN)]

        MATCHES = []
        for i in range(len(separator_indexes) - 1):
            BEGG = separator_indexes[i].start()
            END = separator_indexes[i + 1].end()
            MATCHES += [PGN[BEGG:END]]

        print( "%i Matches loaded from PGN database.\n" % len(MATCHES) )
        self.Matches = MATCHES
        return 1
    
    def load_SimpleDatabase(self, FILE):
        DATA = open(FILE, 'r').readlines()
        self.SimpleDatabase = []
        for L in DATA:
            self.SimpleDatabase.append(L.split('|')[0].replace('\n', ''))
            
        self.database_size = len(self.SimpleDatabase)

        print("loaded simple database of len %i" % self.database_size)

    def refreshDatabase(self):
        FenList = []
        NewMovements = open(self.NewMovements, 'r').readlines()
        #self.load_SimpleDatabase(self.SimpleDatabase)
        FENsInDatabase = [x[0] for x in self.SimpleDatabase]
        for NEW in NewMovements:
            NEW = NEW.split('|')[0].replace('\n', '')
            if NEW not in FENsInDatabase:
                FenList.append(NEW)
        if FenList:
            self.database_size = len(FenList)
            self.runDataCollector(FenList)

        NewMovements = open(self.NewMovements, 'w')
        NewMovements.write('\n')
                
            
    
    def fenToFile(self, FEN):
        fname = 'posfeed.fen'
        F = open(fname, 'w')
        F.write(FEN)
        
    def SaveBoard_ExpectedMovement(self, Board, Movement):
        
        F = open(self.DatabaseName, 'a')
        DATA = "%s" % Board
        for W in Movement:
            if W[0]:
                DATA += "|%s" % W[0]
        DATA += '\n'

        F.write(DATA)

    def runDataCollector(self, FenList=None):
        self.ModelEngine = Engine(["stockfish"])
        
        self.ModelEngine.send("uci")
        self.ModelEngine.send("ucinewgame")
        NUMBER_OF_TOP_MOVES = 5
        self.ModelEngine.send("setoption name MultiPV value %i" % NUMBER_OF_TOP_MOVES)
        
        sleep(3)
        #print(FenList)
        for K in range(self.database_size):
            if self.PGN_dataBase:
                FEN = self.fetchBoardFromDatabase()
            elif FenList:
                FEN = FenList[K]
            elif self.SimpleDatabase:
                FEN = self.SimpleDatabase[K]
            else:
                FEN = self.generateRandomBoard()
                
            self.ModelEngine.send("position fen %s" % FEN)
            self.ModelEngine.send("go")
            print("sent %s;" %FEN)
            
            MOVE = None
            TOP_MOVES = [ ["",0] for k in range(NUMBER_OF_TOP_MOVES) ]
            Highest_Depth_Gone = 0
            while not MOVE:
                sleep(5)
                READ = self.ModelEngine.receive()
                for line in READ:
                    splitline = line.split(' ')
                    if 'depth' in splitline:
                        depth = int(splitline[splitline.index('depth') + 1])
                        if depth > Highest_Depth_Gone:
                            Highest_Depth_Gone = depth
                        if depth == Highest_Depth_Gone:
                            try:
                                multipv = int(splitline[splitline.index('multipv') + 1]) - 1
                                TOP_MOVES[multipv][0] = splitline[splitline.index("pv") + 1]
                                TOP_MOVES[multipv][1] = int(splitline[splitline.index("score") + 2])
                            except:
                                print("Moveline Interpreter Fail.")
                                print(splitline)
                                raise
                            
                            
                                          
                MOVE = self.ModelEngine.readMove(data=READ, moveKeyword="bestmove", Verbose=False)
                if not MOVE:
                    print("Movement not available.")
                else:
                    print("got move %s" % MOVE)
                    print("progress [%i/%i]\n" % (K+1, self.database_size))

            print(TOP_MOVES)
            self.SaveBoard_ExpectedMovement(FEN, TOP_MOVES)



        

