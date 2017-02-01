from tkinter import *

from threading import Thread

import chess

from time import sleep

import gc
from random import randrange
from weakref import ref

from chessArena.enginewrapper import Engine
from evchess_evolve.core import machine

from chessArena.settings import Settings
settings = Settings()

class Table(Frame):
    def __init__(self, arena, master=None, forceNoGUI=False):

        if settings.GUI and not forceNoGUI:
            Frame.__init__(self, master)
            self.GUI = 1
        else:
            self.GUI = 0

        self.Color = {'nogame': 'brown',
                      'game': 'grey'}
        self.board = chess.Board()
        
        self.online = 0
        self.initialize = 0
        
        self.movelist = []

        self.arena = arena

        self.MACHINE = []

        self.consec_failure = 0

        self.Damaged = 0

        self.turn = 0
        self.number = len(arena.TABLEBOARD) if self.arena else 0

        if not self.arena:
            self.result = None

        self.rounds_played = 0

        self.visible = 0

        self.LastFlush = ""

        self.flagged_toend = 0

        if self.GUI:
            self.setWidgets()

        self.startThread = None

        if forceNoGUI:
            self._w = "chessArena table #%i" % self.number

        self.onGame = 0
        

    # Threads for engine loading has been cast aside; Performance gain
    # is negible in face of greater RAM comsumption.
    # It depends on the table, who can call newmatch_thread() or newmatch().
    def newmatch_thread(self, specificMachines=None):

        if not self.online and not self.initialize and not self.startThread:
            try:
                if self.startThread:
                    self.startThread.join()

                self.startThread = Thread(target=self.newmatch, kwargs={
                                          'specificMachines': specificMachines})

                self.startThread.start()
            except RuntimeError:
                print('Error starting match.')
                
    def newmatch(self, specificMachines=["any", "any"], specificOpening=None):
        
        self.MACnames = ['zero', 'zero']
        self.MACcontent = []
        try:
            for M in [0, 1]:
                self.MACHINE[M].send("load %s" % specificMachines[M])
            sleep(0.2)
            for M in [0, 1]:
                for line in self.MACHINE[M].receive():
                    L = line
                    # print(L)
                    if "machinepath>> " in L:
                        self.MACnames[M] = L.split('\n')[0].split('/')[-1].split('.')[-2]


            if specificOpening:
                for M in self.MACHINE:
                    M.send("position %s" % specificOpening)
                self.board.set_board_fen(specificOpening)
            else:
                self.MACHINE[1].send("new")
                self.MACHINE[0].send("new")
                
            self.MACHINE[0].send("white")
            SideToPlay = 1 - int(self.board.turn)
            self.turn = SideToPlay
            self.MACHINE[SideToPlay].send("go")

        except BrokenPipeError:
            print("broken pipe @ " + str(self.number) + " while starting match.")
            #self.log("broken pipe %s %s", "setup." % (self.MACnames[0],self.MACnames[1]))
            try:
                if self.startuplog[0]:
                    self.log(self.startuplog[0].decode('utf-8'), 'BLACK')
                if self.startuplog[1]:
                    self.log(self.startuplog[1].decode('utf-8'), 'WHITE')
            except:
                print("Startuplog logging failed.")
                pass
            
        if self.GUI:
            self.Maximize["background"] = self.Color['game']
            self.Mnames["text"] = self.MACnames[0] + " X " + self.MACnames[1]
            if self.visible:
                self.visor.delete('1.0', END)
                self.visor.insert('1.0', self.board)
            self.switch["command"]
            self.setlimit["text"] = "0"
        self.LoadMachineData()
        self.onGame = 1
        
    def startEngines(self, Command=settings.engineARGS):

        # while len(self.MACHINE) > 0:
        #    self.MACHINE[0].kill()

        self.MACHINE = []


        self.rounds_played = 0

        self.initialize = 1
        if self.GUI:
            self.Maximize["background"] = "purple"


        for W in range(2):
            self.MACHINE.append(Engine(Command))

        sleep(0.02)
        if self.GUI:
            self.Maximize["background"] = "gold"
        try:
            self.MACnames = ['zero', 'zero']

            self.board.reset()
        except:
            self.log('exception', '#2')
            self.initialize = 0
            self.log('Uknown startup error.', '')
            self.endgame()
            # raise #MODF
            return

        if self.GUI:
            self.Maximize["background"] = self.Color['nogame']
        # self.Pout(self.Wmachine.stdout.readlines())
        # self.Pout(self.Bmachine.stdout.readlines())

        sleep(0.1)
        self.startuplog = []

        #if not self.LoadMachineData(MachineDirectory):
        #    return
        
        self.online = 1
        #self.turn = 0
        self.initialize = 0

        self.startThread = 0

    def LoadMachineData(self, DIR=settings.machineDIR):
        del self.MACcontent
        self.MACcontent = []
        for NAME in self.MACnames:
            try:
                self.MACcontent.append(machine('%s.mac' % NAME, DIR=DIR))
                self.MACcontent[-1].Load()

                
            except FileNotFoundError:
                self.log(
                    "filename not found ( %s %s ). " % (DIR, DIR) +
                    "Maybe coudn't be read properly by arena.", "")
                self.MACcontent.append(machine('zero'))
        return 1

    def Pout(self, readobj):
        for line in readobj:
            print(line.decode('utf-8'))

    def readmove(self):
        SUCCEED = 0

        if self.flagged_toend == 1:
            self.endgame()
            return
        if self.initialize == 1:
            self.log('denied by', 'positive initialize')
            return
        if len(self.MACHINE) < 2:
            print('machine breakdown @ %i' % self.number)
            self.endgame()
            return

        if settings.VerboseMove:
            print(COLOR[self.turn] + " @  table " + str(self.number))

        if self.GUI:
            self.setlimit["background"] = "white"
            self.Maximize["background"] = "black"

        self.movelist = []

        try:
            self.LastFlush = self.MACHINE[self.turn].receive()
            if settings.VerboseMove:
                print(self.LastFlush)

        except BrokenPipeError:
            print("broken pipe @ " + str(self.number) + " while gaming.")
            self.log("broken pipe", self.number)
            self.endgame()
            return
        except IndexError:
            self.log("read of move with unintialized machines.", self.number)
            self.endgame()
            return
        MOVE = 0

        # for line in self.MACHINE[self.turn].stdout.readlines():

        MOVE = self.MACHINE[self.turn].readMove(data=self.LastFlush)

        if MOVE:
            for move in self.board.legal_moves:
                self.movelist.append(str(move))
            if MOVE in self.movelist:
                try:
                    self.board.push(chess.Move.from_uci(MOVE))
                    SUCCEED = 1
                    if self.arena:
                        self.arena.move_read_reliability += 1
                    self.consec_failure = 0
                    if self.GUI:
                        self.setlimit["text"] = "0"
                    if settings.VerboseMove:
                        print("move done " + MOVE + "\n")
                except TypeError:
                    self.log("BOARD.PUSH ERROR.", MOVE)
                    print("BOARD.PUSH ERROR!.")
                    self.endgame()

                if self.GUI:
                    self.Vrefresh()

                if self.board.is_checkmate():
                    self.sendresult(self.turn)
                    return
                if self.board.is_stalemate() or self.board.is_insufficient_material():
                    self.sendresult(0.5)
                    return

                if self.board.can_claim_fifty_moves() or self.board.can_claim_threefold_repetition():

                    Wp = 0
                    Bp = 0
                    Wpc = ['P', 'R', 'N', 'B', 'Q']
                    Bpc = ['p', 'r', 'n', 'b', 'q']

                    for z in str(self.board):
                        if z in Wpc:
                            Wp += 1
                        if z in Bpc:
                            Bp += 1
                    if Wp > 1.5 * Bp:
                        self.sendresult(0.2)
                    elif Bp > 1.5 * Wp:
                        self.sendresult(0.8)
                    else:
                        self.sendresult(0.5)
                    return

                self.turn = 1 - self.turn

                self.rounds_played += 1

                try:
                    self.MACHINE[self.turn].send(MOVE)
                except BrokenPipeError:
                    print("broken pipe @ " + str(self.number) +
                          " while receiving move.")

                    if self.arena:
                        self.arena.setcounter_inactivity += 1

                    self.log("broken pipe while receiving move.", self.number)
                    self.log(self.Board, self.Board.fullmove_number)
                    self.endgame()
                    return
            else:
                print("error! Illegal move! " + MOVE)
                self.log("error! Illegal move! " + MOVE, 0)
                if self.arena:
                    self.arena.setcounter_illegalmove += 1

                # self.log(str(self.board),0)
                #self.log('engine internal board >>>>',0)

                self.MACHINE[self.turn].send("dump")
                sleep(1)
                try:
                    Hdump = self.MACHINE[
                        self.turn].receive(method="word", channel=2)

                    if self.arena:
                        FLOG = open('log/log_illegal%i.txt' %
                                    self.arena.ROUND, 'w+')
                        FLOG.write('illegal move. by %s. %s -> %s\n' %
                                   (settings.COLOR[self.turn], self.MACnames[self.turn], MOVE))
                        FLOG.write('')
                        FLOG.write(Hdump)
                        FLOG.write(str(self.board))
                        FLOG.close()

                except AttributeError:
                    pass

                self.endgame()
                return 0

        else:  # no move read this turn.
            # for line in self.movereadbuff:
            #    self.log(line,'<<<<< %i' % len(self.movereadbuff))
            self.consec_failure += 1

            if self.consec_failure % 25 == 0:
                try:
                    #self.log("requested FEN > ","%s" % str(self.board.fen()))
                    self.MACHINE[self.turn].send("echo")

                except BrokenPipeError:
                    self.log('broken pipe on bizarre inactive bug.',
                             COLOR[self.turn])
                    self.MACHINE[self.turn].dumpRecordedData()
                    self.DUMPmovehistory("inactivity")
                    if self.arena:
                        self.arena.setcounter_inactivity += 1
                    self.endgame()
            if self.GUI:
                self.setlimit["text"] = str(self.consec_failure)

            if self.consec_failure > 27:
                print("restarting due to inactivity.")
                if self.arena:
                    self.arena.setcounter_inactivity += 1
                self.consec_failure = 0

                self.endgame()

        if self.GUI and self.arena:
            if self.number <= self.arena.looplimit:
                self.setlimit["background"] = "green"
                self.Maximize['background'] = self.Color['game']
            else:
                self.setlimit["background"] = "red"

        return SUCCEED

    def endgame(self):
        self.flagged_toend = 0
        self.Damaged = 0

        self.board = chess.Board()

        self.onGame = 0
        self.initialize = 0
        if self.GUI:
            self.switch["text"] = "off"
            self.switch["command"] = self.turnon
            self.Mnames["text"] = "idle"
            self.Maximize["background"] = self.Color['nogame']
            if self.visible:
                self.visor.delete('1.0', END)

    def sendresult(self, result):
        if settings.VerboseMove:
            print("game ends @ " + str(self.number))

        if not self.arena:
            self.result = result
            self.endgame()
            return

        if result == 0.5:
            self.arena.setcounter_draws += 1
        else:
            if round(result) == result:
                self.arena.setcounter_checkmate += 1
            else:
                self.arena.setcounter_forcedwin += 1
                result = round(result)

        self.sendELO(result)

        if result > 0.5:
            result = '0-1'
            if self.GUI and self.visible:
                self.visor.insert('10.1', 'checkmate. black wins')
            #self.log('checkmate', self.MACnames[1])

        elif result < 0.5:
            result = '1-0'
            if self.GUI and self.visible:
                self.visor.insert('10.1', 'checkmate. white wins')
            #self.log('checkmate', self.MACnames[0])

        elif result == 0.5:
            result = '1/2-1/2'
            if self.GUI and self.visible:
                self.visor.insert('10.1', 'draw.')
            #self.log('draw', '1/2')

        for MAC in self.MACHINE:
            MAC.send("result %s" % result)

        self.endgame()

    def setWidgets(self):
        self.visor = Text(self, height=10, width=16, borderwidth=4,
                          relief=GROOVE, font=("Courier", 6, 'bold'))
        # self.visor.grid(column=0,row=1,rowspan=6)

        self.switch = Button(self)
        self.switch["text"] = "off"
        self.switch["command"] = self.turnon
        # self.switch.grid(column=1,row=1)

        self.play = Button(self)
        self.play["text"] = "play"
        self.play["command"] = lambda: self.log(
            "requested FEN > ", "%s" % str(self.board.fen()))
        # self.play.grid(column=1,row=2)

        self.setlimit = Button(self)
        self.setlimit["text"] = "off"
        self.setlimit["command"] = self.setlooplimit
        # self.setlimit.grid(column=1,row=3)

        self.Mnames = Label(self)
        self.Mnames["text"] = "idle"
        #self.Mnames.grid(row=0, column=0,columnspan=3)

        self.Maximize = Button(self, text='  ', command=self.shrink_maximize)
        self.Maximize.grid(row=1, column=0, sticky=NSEW)

        self.shrink = Button(self, text='x', command=self.shrink_maximize)

    def turnon(self):
        self.newmatch_thread()

        if self.GUI:
            self.switch["text"] = "on"
            self.switch["command"] = self.endgame
    def shutdown(self):
        for M in self.MACHINE:
            M.destroy()
        self.MACHINE = []
    def Vrefresh(self):
        if self.visible:
            self.visor.delete('1.0', END)
            self.visor.insert(END, self.board)

    def setlooplimit(self):
        self.arena.setlooplimit(self.number)

    def log(self, event, var):
        LOG = open("log.txt", "a+")

        LOG.write(event + " @ " + str(self.number) + " " + str(var) + " \n")

        LOG.close()

    def check_engine_health(self):
        for M in range(len(self.MACHINE)):
            try:
                PID = self.MACHINE[M].pid
            except IndexError:
                return

            MEMUSAGE = Process(pid=PID).memory_info()

            #print('checking process %s,' % PID)
            # print(CHK)
            #self.log('checking process', CHK)
            if MEMUSAGE[0] > 150000000:
                print('terminating table, memory limit overriden by %s' %
                      self.MACnames[M])
                self.log(
                    'terminating table, memory limit overriden by ', self.MACnames[M])

                self.endgame()

    def shrink_maximize(self):
        if self.visible:
            self.visor.grid_forget()
            self.Mnames.grid_forget()
            self.switch.grid_forget()
            self.play.grid_forget()
            self.setlimit.grid_forget()

            self.shrink.grid_forget()
            self.Maximize.grid(row=1, column=0)
            self.visible = 0

        else:
            self.Maximize.grid_forget()
            self.Mnames.grid(row=0, column=0, columnspan=3)
            self.visor.grid(column=0, row=1, rowspan=5)
            self.switch.grid(column=1, row=1, sticky=W + E)
            self.play.grid(column=1, row=2)
            self.setlimit.grid(column=1, row=3, sticky=W + E)

            self.shrink.grid(column=1, row=4, sticky=W + E)
            self.visible = 1

    def show_status(self):
        print(" -- table %i" % self.number)
        print(engineARGS)
        print("online: %i" % self.online)
        print(self.MACHINE)
        print("startThread: %s" % self.startThread)
        print("initialize: %i" % self.initialize)

        print(" -- ")

    def sendELO(self, winner):
        self.LoadMachineData()

        Draw = False

        if winner == 0.5:
            winner = 0
            base = 0
            Draw = True
            
            self.MACcontent[winner].STAT_PARAMETERS[1].value +=1
            self.MACcontent[1-winner].STAT_PARAMETERS[1].value +=1
        else:
            base = 28
            self.MACcontent[winner].STAT_PARAMETERS[1].value += 1
            self.MACcontent[1-winner].STAT_PARAMETERS[3].value +=1
            
        deltaELO = self.MACcontent[winner].ELO - self.MACcontent[1-winner].ELO
        deltaELO = base - deltaELO // 18
            
        self.MACcontent[winner].STAT_PARAMETERS[0].value +=1    
        self.MACcontent[1-winner].STAT_PARAMETERS[0].value +=1
        
        if not Draw and deltaELO < 0:
            deltaELO = 0


        self.MACcontent[winner].ELO += deltaELO
        self.MACcontent[1-winner].ELO -= deltaELO
                


        try:
            for macIndex in range(len(self.MACcontent)):
                if self.MACcontent[macIndex].checkExistence():
                    self.MACcontent[macIndex].write()

                else:
                    print("%s/%s not found!" % (self.MACcontent[macIndex].DIR, self.MACcontent[macIndex].filename) )
        except Exception as E:
            raise
            print("FAIL TO SEND ELO %s" % E)
            self.log('sending ELO failed.', E)

    def DUMPmovehistory(self, reason):
        CurrentRound = self.arena.ROUND if self.arena else 0
        Fname = 'log/log_%s_%i.txt' % (reason, CurrentRound)
        FLOG = open(Fname, 'w+')
        FLOG.write("%s\n" % reason)
        try:
            self.MACHINE[self.turn].send("dump")

            sleep(1)
            Hdump = self.MACHINE[self.turn].receive(method="word")

            FLOG.write('')
            FLOG.write(Hdump)

        except:
            if self.Damaged:
                FLOG.close()

            else:
                self.turn = 1 - self.turn
                self.Damaged = 1
                FLOG.write(str(self.board))
                FLOG.close()
                self.DUMPmovehistory("%s_opponent" % reason)

            return
        FLOG.write(str(self.board))
        FLOG.close()
