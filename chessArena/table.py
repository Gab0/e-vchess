from tkinter import *

from threading import Thread
from subprocess import Popen, PIPE, call
import chess

from time import sleep
from fcntl import fcntl, F_GETFL, F_SETFL
from os import O_NONBLOCK, read, system

from chessArena.settings import *

class Table(Frame):
    def __init__(self, arena, master=None, forceNoGUI=False):
        
        if GUI and not forceNoGUI:
            Frame.__init__(self, master)
            self.GUI = 1
        else:
            self.GUI = 0
            
        self.board = chess.Board()
        self.online = 0
        self.movelist=[]


        self.arena = arena 

        self.MACHINE = []
        
        self.consec_failure=0

        self.Damaged = 0
        
        self.turn = 0
        self.number = len(arena.TABLEBOARD) if self.arena else 0

        if not self.arena:
            self.result = None

        self.rounds_played=0
        
        self.visible = 0

        self.LastFlush = ""

        self.flagged_toend = 0
        
        if self.GUI:
            self.setWidgets()


        self.startThread = None

        self.initialize=0
        
    def newmatch_thread(self, specificMatch=None):

        if not self.online and not self.initialize and not self.startThread:
            try:
                self.startThread = Thread(target=self.newmatch, kwargs={'specificMatch': specificMatch})
                self.startThread.start()
            except RuntimeError:
                print('Error starting match.')
                
        elif not self.initialize:
            self.endgame()
            if self.startThread:
                self.startThread.join()
                self.startThreead = None
                
    def newmatch(self, specificMatch=None):
        if self.initialize:
            return
        
        while len(self.MACHINE)>0: self.MACHINE[0].kill()
        
        self.MACHINE = []
        self.MACcontent = []
        
        self.rounds_played=0
        
        self.initialize = 1
        if self.GUI:
            self.Maximize["background"] = "purple"

        if specificMatch:
            MachineDirectory = "machines/top_machines/"
        else:
            MachineDirectory = "machines/"

        try:
            if specificMatch:
                CURRENTengineARGS = engineARGS + ["-TOP", "--specific", specificMatch[0]]
                # print(' '.join(CURRENTengineARGS))
            else:
                CURRENTengineARGS = engineARGS
                
            self.MACHINE.append(Popen(CURRENTengineARGS, stdin=PIPE, stdout=PIPE))

            if specificMatch:
                CURRENTengineARGS = engineARGS + ["-TOP", "--specific", specificMatch[1]]
            else:
                CURRENTengineARGS = engineARGS
                
            self.MACHINE.append(Popen(CURRENTengineARGS, stdin=PIPE, stdout=PIPE))
        except Exception as e:
            self.log('exception', '#1')
            print("Initializing failed.")
            print(e.strerror)

            self.endgame()
            return

        sleep(4)
        if self.GUI:
            self.Maximize["background"] = "gold"
        try:
            self.MACnames = ['zero','zero']
        
            flags = fcntl(self.MACHINE[0].stdout, F_GETFL) # get current p.stdout flags
            fcntl(self.MACHINE[0].stdout, F_SETFL, flags | O_NONBLOCK)
            fcntl(self.MACHINE[1].stdout, F_SETFL, flags | O_NONBLOCK)#fcntl(self.Bmachine.stdout, F_SETFL, flags | O_NONBLOCK) >>>>>?

            self.board.reset()
        except:
            self.log('exception', '#2')
            self.initialize=0
            self.log('Uknown startup error.','')
            self.endgame()
            # raise #MODF
            return
        
        
        
        if self.GUI: self.Maximize["background"] = "brown"
        #self.Pout(self.Wmachine.stdout.readlines())
        #self.Pout(self.Bmachine.stdout.readlines())
        
        sleep(1)
        self.startuplog = []
        try:
            #self.startuplog.append(self.MACHINE[1].stdout.read())
            #self.startuplog.append(self.MACHINE[0].stdout.read())
            #self.MACHINE[0].stdout.flush()
            
            #self.MACHINE[1].stdout.flush()

            for i in [0,1]:
                for line in self.MACHINE[i].stdout.readlines():
                    L = line.decode('utf-8')
                    # print(L)
                    if "MACname > " in L:
                        self.MACnames[i] = L[10:-1]
                        
                    if "opening machine:" in L:
                        self.MACnames[i] = L.split('/')[-1][:-1]

            sleep(5)            


            
            self.MACHINE[1].stdin.write(bytearray('new\n','utf-8'))
            self.MACHINE[1].stdin.flush()
            
            self.MACHINE[0].stdin.write(bytearray('new\n','utf-8'))
            self.MACHINE[0].stdin.flush()
            sleep(0.6)
            self.MACHINE[0].stdin.write(bytearray('white\n','utf-8'))
            self.MACHINE[0].stdin.flush()
            sleep(0.6)
            self.MACHINE[0].stdin.write(bytearray('go\n','utf-8'))                
            self.MACHINE[0].stdin.flush()
            

        except BrokenPipeError:
            print("broken pipe @ "+str(self.number) + " while starting.")
            #self.log("broken pipe %s %s", "setup." % (self.MACnames[0],self.MACnames[1]))
            try:
                if self.startuplog[0]:
                    self.log(self.startuplog[0].decode('utf-8'), 'BLACK')
                if self.startuplog[1]:
                    self.log(self.startuplog[1].decode('utf-8'), 'WHITE')
            except:
                print("Startuplog logging failed.")
                pass
                
            self.initialize = 0
            return

        if self.GUI: self.Maximize["background"] = "brown"

        if not self.ReadMachineTextContent(MachineDirectory):
            return


        if self.GUI:
            self.Maximize["background"] = "grey"
            self.Mnames["text"] = self.MACnames[0] + " X " + self.MACnames[1]
            self.visor.delete('1.0', END)
            self.visor.insert('1.0', self.board)
            self.switch["command"]
            self.setlimit["text"] = "0"

        self.online = 1
        self.turn = 0      
        self.initialize=0
            

        self.startThread = None

    def ReadMachineTextContent(self, MachineLocation):
        try:
            self.MACcontent = []
            for NAME in self.MACnames:
                MachinePath = "%s%s" % (MachineLocation, NAME)
                self.MACcontent.append(open(MachinePath, 'r').readlines())
            return 1
        except FileNotFoundError:
            self.log(
                "filename not found ( %s ). " % MachinePath +\
                "Maybe coudn't be read properly by arena." , "" )
            
            self.initialize = 0
            return 0

        
    def Pout(self, readobj):
        for line in readobj:
            print(line.decode('utf-8'))

        
    def readmove(self):
        SUCCEED = 0
        
        if self.flagged_toend == 1:
            self.endgame()
            return
        if self.initialize==1:
            self.log('denied by', 'positive initialize')
            return
        if len(self.MACHINE) < 2:
            print('machine breakdown @ %i' % self.number)
            self.endgame()
            return

        if VerboseMove:
            print(COLOR[self.turn] + " @  table " + str(self.number))
        
        if self.GUI:
            self.setlimit["background"] = "white"
            self.Maximize["background"] = "black"
            
        self.movelist = []

        try:
            self.LastFlush = self.MACHINE[self.turn].stdout.readlines()
            self.MACHINE[self.turn].stdout.flush()

        except BrokenPipeError:
            print("broken pipe @ "+str(self.number) + " while gaming.")
            self.log("broken pipe", self.number)
            self.endgame()
            return
        except IndexError:
            self.log("read of move with unintialized machines.", self.number)
            self.endgame()
            return        
        MOVE = 0

        self.movereadbuff = []


        # for line in self.MACHINE[self.turn].stdout.readlines():
        for line in self.LastFlush:
            #print(line.decode('utf-8'))
            line = line.decode('utf-8', 'ignore')[:-1]
            if "move" not in line:
                continue

            L = line.split(" ")
            
            if ("move" in L[0]) and (len(L)>1):
                    #print(">>>> %s"%L[1])
                    MOVE = L[1]
                    break
            elif ("move" in L[1]):
                MOVE = L[-1]
                break
            else:
                self.movereadbuff.append(line)
                    

        if MOVE:
            for move in self.board.legal_moves:
                self.movelist.append(str(move))
            if MOVE in self.movelist:
                try:
                    self.board.push(chess.Move.from_uci(MOVE))
                    SUCCEED = 1
                    if self.arena:
                        self.arena.move_read_reliability += 1
                    self.consec_failure=0
                    if self.GUI:
                        self.setlimit["text"] = "0"
                    if VerboseMove:
                        print("move done " + MOVE + "\n")
                except TypeError:
                    self.log("BOARD.PUSH ERROR.", MOVE)
                    print("BOARD.PUSH ERROR!.")
                    self.endgame()
                    
                if self.GUI: self.Vrefresh()
                
                if self.board.is_checkmate():
                    self.sendresult(self.turn)
                    return
                if self.board.is_stalemate() or self.board.is_insufficient_material():
                    self.sendresult(0.5)
                    return

                if self.board.can_claim_fifty_moves() or self.board.can_claim_threefold_repetition():

                    Wp = 0
                    Bp = 0
                    Wpc = ['P','R','N','B','Q']
                    Bpc = ['p','r','n','b','q']
                    
                    for z in str(self.board):
                        if z in Wpc: Wp+=1
                        if z in Bpc: Bp+=1
                    if Wp > 1.5 * Bp:
                        self.sendresult(0.2)
                    elif Bp > 1.5 * Wp:
                        self.sendresult(0.8)
                    else:
                        self.sendresult(0.5)
                    return    
                
                self.turn = 1-self.turn
                self.MACHINE[self.turn].stdin.write(bytearray(MOVE+'\n','utf-8'))
                self.rounds_played+=1

                try:
                    self.MACHINE[self.turn].stdin.flush()
                except BrokenPipeError:
                    print("broken pipe @ " + str(self.number) + " while receiving move.")

                    if self.arena:
                        self.arena.setcounter_inactivity += 1
                        
                    self.log("broken pipe while receiving move.", self.number)
                    self.log(self.Board, self.Board.fullmove_number)
                    self.endgame()
                    return
            else:
                print("error! Illegal move! "+ MOVE)
                self.log("error! Illegal move! "+ MOVE,0)
                if self.arena:
                    self.arena.setcounter_illegalmove+=1
                
                #self.log(str(self.board),0)
                #self.log('engine internal board >>>>',0)
                
                self.MACHINE[self.turn].stdin.write(bytearray('dump\n','utf-8'))
                self.MACHINE[self.turn].stdin.flush()
                sleep(1)    
                try:
                    Hdump = self.MACHINE[self.turn].stdout.read().decode('utf-8')

                    if self.arena:
                        FLOG = open('log/log_illegal%i.txt' % self.arena.ROUND,'w+')
                        FLOG.write('illegal move. by %s. %s -> %s\n' % (COLOR[self.turn], self.MACnames[self.turn], MOVE))
                        FLOG.write('')
                        FLOG.write(Hdump)
                        FLOG.write(str(self.board))
                        FLOG.close()
                    
                except AttributeError:
                    pass

                self.endgame()
                return 0

        else:#no move read this turn.
            #for line in self.movereadbuff:
            #    self.log(line,'<<<<< %i' % len(self.movereadbuff))
            self.consec_failure+=1

            if self.consec_failure % 25 == 0:
                try:
                    #self.log("requested FEN > ","%s" % str(self.board.fen()))
                    self.MACHINE[self.turn].stdin.write(bytearray('echo\n', 'utf-8'))
                    self.MACHINE[self.turn].stdin.flush()
                    self.MACHINE[1-self.turn].stdin.write(bytearray('sorry\n', 'utf-8'))
                except BrokenPipeError:
                    self.log('broken pipe on bizarre inactive bug.',COLOR[self.turn])
                    self.DUMPmovehistory("inactivity")
                    if self.arena:
                        self.arena.setcounter_inactivity += 1
                    self.endgame()
            if self.GUI:
                self.setlimit["text"] = str(self.consec_failure)
            
            if self.consec_failure > 27:
                print("restarting due to inactivity.")
                if self.arena:
                    self.arena.setcounter_inactivity+=1
                self.consec_failure = 0
                
                self.endgame()
                
        if self.GUI and self.arena:
            if self.number <= self.arena.looplimit:
                self.setlimit["background"] = "green"
                self.Maximize['background'] = "grey"
            else: self.setlimit["background"] = "red"
                
        return SUCCEED
        
    def endgame(self):
        self.flagged_toend=0
        self.Damaged = 0
        for machine in self.MACHINE:
            #print('killing %s' % machine.pid)
            call(['kill', '-9', str(machine.pid)])
            machine.terminate()
        self.MACHINE = []



        self.online = 0
        self.initialize = 0
        if self.GUI:
            self.switch["text"] = "off"
            self.switch["command"] = self.turnon
            self.Mnames["text"] = "idle"
            self.Maximize["background"] = "light grey"
            self.visor.delete('1.0',END)
            
    def sendresult(self, result):
        print("game ends @ " + str(self.number))

        if not self.arena:
            self.result = result
            self.endgame()
            return
        
        if result == 0.5:
            self.arena.setcounter_draws+=1
        else:
            if round(result) == result:
                self.arena.setcounter_checkmate+=1
            else:
                self.arena.setcounter_forcedwin+=1
                result = round(result)
            
        
        self.sendELO(result)
        
        if result > 0.5:
            result = '0-1'
            if self.GUI: self.visor.insert('10.1', 'checkmate. black wins')
            #self.log('checkmate', self.MACnames[1])
            
        elif result < 0.5:
            result = '1-0'
            if self.GUI: self.visor.insert('10.1', 'checkmate. white wins')
            #self.log('checkmate', self.MACnames[0])
            
        elif result == 0.5:
            result = '1/2-1/2'
            if self.GUI: self.visor.insert('10.1', 'draw.')
            #self.log('draw', '1/2')
            

        for MAC in self.MACHINE:    
            MAC.stdin.write(bytearray('result '+ result + '\n', 'utf-8'))
            MAC.stdin.flush()
        self.online = 0
        
        self.endgame()
        
    def setWidgets(self):
        self.visor = Text(self, height=10,width=16, borderwidth=4, relief=GROOVE, font=("Courier",6,'bold'))
        #self.visor.grid(column=0,row=1,rowspan=6)

        self.switch = Button(self)
        self.switch["text"] = "off"
        self.switch["command"] = self.turnon
        #self.switch.grid(column=1,row=1)

        self.play = Button(self)
        self.play["text"] = "play"
        self.play["command"] = lambda: self.log("requested FEN > ","%s" % str(self.board.fen()))
        #self.play.grid(column=1,row=2)

        self.setlimit = Button(self)
        self.setlimit["text"] = "off"
        self.setlimit["command"] = self.setlooplimit
        #self.setlimit.grid(column=1,row=3)

        self.Mnames = Label(self)
        self.Mnames["text"] = "idle"
        #self.Mnames.grid(row=0, column=0,columnspan=3)


        self.Maximize = Button(self, text='  ', command = self.shrink_maximize)
        self.Maximize.grid(row=1,column=0,sticky=NSEW)
        
        self.shrink = Button(self, text='x', command = self.shrink_maximize)
        
        
    def turnon(self):
        self.newmatch_thread()
        
        if self.GUI:
            self.switch["text"] = "on"
            self.switch["command"] = self.endgame
        


        
    def Vrefresh(self):
        self.visor.delete('1.0',END)
        self.visor.insert(END, self.board)


    def setlooplimit(self):
        self.arena.setlooplimit(self.number)    


    def log(self, event, var):
        LOG = open("log.txt", "a+")

        LOG.write(event + " @ " + str(self.number) + " " + str(var)+ " \n")
        
        LOG.close()



    def check_engine_health(self):
        for M in range(len(self.MACHINE)):
            try:
                PID = self.MACHINE[M].pid
            except IndexError:
                return

            MEMUSAGE = Process(pid=PID).memory_info()


            #print('checking process %s,' % PID)
            #print(CHK)
            #self.log('checking process', CHK)
            if MEMUSAGE[0] > 150000000:
                print('terminating table, memory limit overriden by %s' % self.MACnames[M])
                self.log('terminating table, memory limit overriden by ', self.MACnames[M])
                
                self.endgame()
            


    def shrink_maximize(self):
        if self.visible:
            self.visor.grid_forget()
            self.Mnames.grid_forget()
            self.switch.grid_forget()
            self.play.grid_forget()
            self.setlimit.grid_forget()

            self.shrink.grid_forget()
            self.Maximize.grid(row=1,column=0)
            self.visible = 0

        else:
            self.Maximize.grid_forget()
            self.Mnames.grid(row=0, column=0,columnspan=3)
            self.visor.grid(column=0,row=1,rowspan=5)
            self.switch.grid(column=1,row=1, sticky=W+E)
            self.play.grid(column=1,row=2)
            self.setlimit.grid(column=1,row=3,sticky=W+E)

            self.shrink.grid(column=1,row=4, sticky=W+E)
            self.visible = 1
            
    def show_status(self):
        print(" -- table %i" % self.number)
        print(engineARGS)
        print("online: %i" %self.online)
        print(self.MACHINE)
        print("startThread: %s" % self.startThread)
        print("initialize: %i" % self.initialize)
            
        print(" -- ")
              

    #def log_wrongmove(self):


    def sendELO(self, winner):

        ELO = []
        index = []
        
        if not self.ReadMachineTextContent("machines/"):
            print("ERROR reading machine text content.")
            
        for macIndex in range(len(self.MACcontent)):
            for lineIndex in range(len(self.MACcontent[macIndex])):
                if 'stat_elo' in self.MACcontent[macIndex][lineIndex].split(' = ')[0]:
                    ELO.append(int(self.MACcontent[macIndex][lineIndex].split(' = ')[1][:-1]))
                    index.append(lineIndex)
                    

        Draw = False
        if len(ELO) == 2:
            if winner == 0.5:
                winner = 0
                base = 0
                Draw = True

            else:
                base = 28
                
                
            deltaELO = ELO[winner] - ELO[1-winner]

            deltaELO = base - deltaELO//18

            newELO = [ 0, 0 ]
            if not Draw and deltaELO < 0:
                deltaELO = 0

            else:
                newELO[winner] = ELO[winner] + deltaELO
                newELO[1-winner] = ELO[1-winner] - deltaELO

            ScoreString = [ ]
            for score in [ 0,1 ]:
                ScoreString.append("%s (%i -> %i)" % (COLOR[score],
                                                      ELO[score],
                                                      newELO[score]))

            if Draw:
                print("Draw. %s; %s.    delta: %i"\
                    % (ScoreString[winner],
                     ScoreString[1-winner],
                     deltaELO) )
            else:                       
                print("Winner is %s. %s lost.    delta: %i"\
                      % (ScoreString[winner],
                         ScoreString[1-winner],
                         deltaELO) )
            
            try:
                for macIndex in range(len(self.MACcontent)):
                    self.MACcontent[macIndex][index[macIndex]] = "stat_elo = %i\n"  % newELO[macIndex]
                    F = open('machines/%s' % self.MACnames[macIndex], 'w')
                    for line in self.MACcontent[macIndex]:
                        F.write(line)
                    F.close()

            except:
                self.log('sending ELO failed.', '')

            

    def DUMPmovehistory(self, reason):
        
        Fname = 'log/log_%s_%i.txt' % (reason, self.arena.ROUND)
        FLOG = open(Fname, 'w+')
        FLOG.write("%s\n" % reason)
        try:
            self.MACHINE[self.turn].stdin.write(bytearray('dump\n','utf-8'))
            self.MACHINE[self.turn].stdin.flush()
            sleep(1)    
            Hdump = self.MACHINE[self.turn].stdout.read().decode('utf-8')

            FLOG.write('')
            FLOG.write(Hdump)

        except:
            if self.Damaged:
                FLOG.close()

            else:
                self.turn = 1-self.turn
                self.Damaged = 1
                FLOG.write(str(self.board))
                FLOG.close()
                self.DUMPmovehistory("%s_opponent" % reason)
                
            return
        FLOG.write(str(self.board))
        FLOG.close()
