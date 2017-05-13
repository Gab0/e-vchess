#!/bin/python
from chessArena.enginewrapper import Engine
import chess
import random
from time import sleep
import chess.pgn
import re
import io
import json

class trainingDataCreator():
    def __init__(self, PGN_dataBase=None, Database=None, NewMovements=None):
        self.database_size = 32
        self.DatabaseName = Database
        self.PGN_dataBase = PGN_dataBase
        self.NewMovements=NewMovements
        
        
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
    
    def loadDatabase(self, FILE):
        DATA = open(FILE, 'r').read()
        self.Database = json.loads(DATA)
            
        self.database_size = len(self.Database)

        print("loaded simple database of len %i" % self.database_size)
        
    def saveDatabase(self, FILE):
        DATA = open(FILE, 'w')
        DATA.write(json.dumps(self.Database, indent=2))
                   
    def refreshDatabase(self):
        FenList = []
        NewMovements = open(self.NewMovements, 'r').readlines()
        FENsInDatabase = [x['pos'] for x in self.Database]
        for NEW in NewMovements:
            NEW = NEW.split('|')[0].replace('\n', '')
            if NEW not in FENsInDatabase:
                FenList.append(NEW)

        if FenList and len(FenList[0]) > 8:
            self.database_size = len(FenList)
            self.runDataCollector(FenList)
        else:
            print("No new FEN found on %s." % self.NewMovements)
        NewMovements = open(self.NewMovements, 'w')
        NewMovements.write('')
        self.saveDatabase(self.DatabaseName)

    def reEvaluateDatabase(self):
        FenList = [ x['pos'] for x in self.Database ]
        self.Database = []
        self.runDataCollector(FenList)
        self.saveDatabase(self.DatabaseName)
        
    
    def fenToFile(self, FEN):
        fname = 'posfeed.fen'
        F = open(fname, 'w')
        F.write(FEN)
        
    def runDataCollector(self, FenList=None):
        self.ModelEngine = Engine(["stockfish"])
        
        self.ModelEngine.send("uci")
        self.ModelEngine.send("ucinewgame")
        NUMBER_OF_TOP_MOVES = 5
        self.ModelEngine.send("setoption name MultiPV value %i" % NUMBER_OF_TOP_MOVES)
        
        sleep(3)
        #print(FenList)
        DataBase = []
        for K in range(self.database_size):
            if self.PGN_dataBase:
                FEN = self.fetchBoardFromDatabase()
            elif FenList:
                FEN = FenList[K]
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
                    #print(line)
                    splitline = line.split(' ')
                    if 'depth' in splitline:
                        depth = int(splitline[splitline.index('depth') + 1])
                        if depth > Highest_Depth_Gone:
                            Highest_Depth_Gone = depth
                        if depth == Highest_Depth_Gone:
                            try:
                                multipv = int(splitline[splitline.index('multipv') + 1]) - 1
                                Move =  splitline[splitline.index("pv") + 1].replace('\n', '')
                                Score = int(splitline[splitline.index("score") + 2])
                                TOP_MOVES[multipv] = [Move, Score]
                            except:
                                print("Moveline Interpreter Fail.")
                                print(splitline)
                                raise
                
                # Filter and Select expected movements;
                if TOP_MOVES[0][1] > 0:
                    TOP_MOVES = [ x for x in TOP_MOVES if (x[1] > (TOP_MOVES[0][1]-100) / 2)]

                elif TOP_MOVES[0][1] < 0:
                    TOP_MOVES = [ x for x in TOP_MOVES if (x[1] > (TOP_MOVES[0][1]-100) * 2) ]
                TOP_MOVES = TOP_MOVES[:5]
                                          
                MOVE = self.ModelEngine.readMove(data=READ, moveKeyword="bestmove", Verbose=False)
                if not MOVE:
                    print("Movement not available.")
                else:
                    print("got move %s" % MOVE)
                    print("progress [%i/%i]\n" % (K+1, self.database_size))

            showTOP_MOVES = [ '%s -> %i' % (x[0],x[1]) for x in TOP_MOVES ]
            print('\n'.join(showTOP_MOVES))
            Data = {
                'pos': FEN,
                'movement': [ x[0] for x in TOP_MOVES ],
                'num_succeeded': 0,
                'num_tried': 0
            }
            self.Database.append(Data)




        

