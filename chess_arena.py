#!/bin/python

import chess
import shlex
from subprocess import *
from time import *

from fcntl import fcntl, F_GETFL, F_SETFL
from os import O_NONBLOCK, read, system
#from os import *

from tkinter import *

import threading

import sys

from evchess_evolve.core import *

from random import randrange

from psutil import *

import gc

# path to e-vchess executable and the directory where machines are stored, respectively.
evchessP = "engine/dist/Release/GNU-Linux/e-vchess"
machineDIR =  "machines"

evchessARGS = [evchessP, "-MD", machineDIR, "--deep", "2"]


GUI = 1

COLOR = {0: 'WHITE', 1: 'BLACK'}
TABLEBOARD = []


if (len(sys.argv) > 0) and ('--nogui' in sys.argv):
    GUI = 0






class Application():

    def __init__(self):

        self.Cycle = False
        self.looplimit = 0

        #sets the number of simultaneous chess tables to be created and played.
        self.TABLECOUNT = 32
        #number of tables to be shown on each row of machines.
        TABLEonROW = 8

        self.TIME = time()
        k=0
        j=0

        self.root = Tk()


        
        for i in range(self.TABLECOUNT):
            TABLEBOARD.append(table(self, master=self.root))
            if GUI: TABLEBOARD[i].grid(column=k,row=j,stick=NSEW)
            k+=1
            if k == TABLEonROW:
                k=0
                j+=1
        
      
        self.menubar= Menu(self.root)
      
        self.Ccycle = self.menubar.add_command(label='cycle thru', command = self.startcycle)
        self.menubar.add_command(label="Kill 'em All", command = self.killemall)
        self.menubar.add_command(label='show/hide ALL', command = self.showhideall)

        if GUI: self.root.config(menu=self.menubar)
        self.showhide_ = 0


        self.move_read_reliability = 0

        #self.config(menu=self.menubar)
        self.setlooplimit(0)

        self.setcounter_illegalmove=0
        self.setcounter_draws=0
        self.setcounter_checkmate=0
        self.setcounter_forcedwin=0
        self.setcounter_inactivity=0


        if (len(sys.argv) > 0) and ('--go' in sys.argv):
            self.setlooplimit(self.TABLECOUNT-1)
            self.startcycle()

        self.Title = "arenaArray"
        self.root.wm_title(self.Title)
        self.root.resizable(False, False)
        self.root.mainloop()

    def startcycle(self):

        self.CYCLE = threading.Thread(target=self.gocycle)
        self.CYCLE.start()
        
    def gocycle(self):
        SLEEPTIME = 1
        if SLEEPTIME < 1: SLEEPTIME = 1.3
        self.ROUND=0
        self.Cycle = True
        
        if GUI:
            self.menubar.entryconfigure(1, label='stop cycle')
            self.menubar.entryconfigure(1, command=self.killcycle)
            
        TIME = time()
        TABLEBOARD[0].log("\n\n\nSTARTING CYCLE", str(strftime("%d/%b - %H:%M:%S")))

        #arena/cycle main loop. Important functions and values.
        while self.Cycle:
            TIME = time()-TIME
            TABLERESPONSE = self.move_read_reliability/(self.looplimit+1)
    

            
            if TABLERESPONSE < 0.42: SLEEPTIME += 0.1
            if (TABLERESPONSE > 0.81) and (SLEEPTIME > 0.5): SLEEPTIME -= 0.1
            SLEEPTIME = round(SLEEPTIME,1)
            
            #update window name to reflect current values.
            if self.ROUND % 3 == 0:                
                self.root.wm_title(self.Title + "  T=%s|R=%s|I=%i" % (round(SLEEPTIME,1),self.ROUND,self.setcounter_inactivity))

            #each N rounds, do maintenance management in order to get best evolving performance.
            #also prints running info to log.
            if self.ROUND != 0:
                LEVEL = ""
                
                if not self.ROUND %  375: LEVEL += "A"
                if not self.ROUND % 1250: LEVEL += "B"
                if not self.ROUND % 2500: LEVEL += "C" 

                if len(LEVEL): self.routine_pop_management(LEVEL)
            
            self.move_read_reliability = 0
            print("ROUND %i  T=%i  S=%f  APR=%i%% >>>>>>>>" % (self.ROUND,round(TIME), SLEEPTIME, TABLERESPONSE*100))
            for t in range(self.looplimit+1):
                if virtual_memory()[2] > 97: self.memorylimit=1
                else: self.memorylimit=0
                
                if self.Cycle:
                    if TABLEBOARD[t].online == 1:
                        TABLEBOARD[t].readmove()
                    else:
                        if not self.memorylimit:
                            
                            TABLEBOARD[t].newmatch_thread()
                            if virtual_memory()[2] > 97: self.memorylimit=1
            if self.ROUND == 0:
                sleep(7)
            self.ROUND+=1

            if self.ROUND > 100000: i=0
            sleep(SLEEPTIME)
        return

    def killcycle(self):
        self.Cycle = False
        self.CYCLE.join(0)
        self.CYCLE=None
        self.menubar.entryconfigure(1, label='cycle thru')#self.cycle["text"] = "cycle thru"
        self.menubar.entryconfigure(1, command=self.startcycle)#self.cycle["command"] = self.gocycle

        

    def setlooplimit(self, limit):
        #if self.Cycle == True: return

        self.looplimit = limit
        if GUI:
            for i in range(len(TABLEBOARD)):
                if i <= self.looplimit:
                    TABLEBOARD[i].setlimit["background"] = "green"
                else:
                    TABLEBOARD[i].setlimit["background"] = "brown"

                    

    def killemall(self):
        for table in TABLEBOARD:
            if table.online == 1:
                table.endgame()

    def killunused(self):
        for T in range(len(TABLEBOARD)):
            if T > self.looplimit:
                TABLEBOARD[T].endgame()

    def StatLock_routine_management(self):
        population = loadmachines()
        population = CyclingStatLock(population)

        self.log('')
        self.log('>>>>>STATLOCK ROUTINE MANAGEMENT')
        self.log('')

        setmachines(population)
        
    def routine_pop_management(self, LEVEL):
        population = loadmachines()

        originalPOPLEN = len(population)
        #for individual in population:
            #dump_all_paramstat(individual)

        #for k in range(8):
        #    CHILD = create_hybrid(population)
        #    if CHILD: population.append(CHILD)


        if "A" in LEVEL:
            for k in range(2): population = mutatemachines(1,population)

        if "B" in LEVEL:
            for k in range(4): population = mutatemachines(1,population)
       
        if "C" in LEVEL:
            population = populate(population, round(originalPOPLEN/8))

            MODscorelimit = 2

            for k in range(3):
                population = replicate_best_inds(population, 3)
            
            for k in range(2): population = mutatemachines(1, population)

            NUM = len(population) - originalPOPLEN
            if NUM > 0:
                population = deltheworst_clonethebest(population, -NUM, MODscorelimit)

        totalgames = (self.setcounter_illegalmove
                     +self.setcounter_forcedwin
                     +self.setcounter_checkmate
                     +self.setcounter_draws+1)   

        setmachines(population)
        self.log('')
        self.log('>>>>ROUTINE MANAGEMENT %s' % LEVEL)
        self.log("ROUND = %i. checkmate-> %i; forced wins-> %i; draws-> %i; illegal moves-> %i"
                 % (self.ROUND, self.setcounter_checkmate, self.setcounter_forcedwin, self.setcounter_draws, self.setcounter_illegalmove))
        self.log("initial population size-> %i; final population size-> %i"
                 % (originalPOPLEN,len(population)))
        self.log('Illegal move percentage is %f %%.'
                 % (self.setcounter_illegalmove*100/totalgames))
        self.log('')
        print('routine management done.')


    def showhideall(self):
        for T in TABLEBOARD:
            if (T.visible) and (self.showhide_):
                T.shrink_maximize()

            elif not (T.visible) and not (self.showhide_):
                T.shrink_maximize()


        self.showhide_= 1 - self.showhide_




    def shrinkloop(self):
        self.looplimit-=3
        self.killunused()



    def log(self, event):
        LOG = open("log.txt", "a+")

        LOG.write(event+" \n")
        
        LOG.close()

        
class table(Frame):
    def __init__(self, arena, master=None):
        if GUI: Frame.__init__(self, master)
        self.board = chess.Board()
        self.online = 0
        self.movelist=[]


        self.arena = arena

        self.MACHINE = []
        
        self.consec_failure=0
  
        self.turn = 0
        self.number = len(TABLEBOARD)

        self.rounds_played=0
        
        self.visible = 0
        #TABLEBOARD.append(self)

        self.flagged_toend = 0
        if GUI: self.setWidgets()


        self.startThread = None

        self.initialize=0
    def newmatch_thread(self):

        if (self.initialize == 0) and (self.online == 0) and (self.startThread):
            self.startThread.join(0)
            self.startThread = None
            
        elif not (self.startThread) and (self.initialize):
            self.initialize = 0
        

        elif (self.startThread) and (self.initialize == 0):
            self.startThread.join(0)
            self.startThread = None
            
        else:
            try:
                if not (self.startThread) and not (self.online):
                    self.startThread = threading.Thread(target=self.newmatch)
                    self.startThread.start()
            except RuntimeError:
                print('Error starting match.')

            
    def newmatch(self):
        if self.initialize: return
        

        while len(self.MACHINE)>0: self.MACHINE[0].kill()
        
        self.MACHINE = []
        self.MACcontent = []
        
        self.rounds_played=0
        
        self.initialize = 1
        if GUI: self.Maximize["background"] = "purple"
        
        try:
            self.MACHINE.append(Popen(evchessARGS, stdin=PIPE, stdout=PIPE))

            self.MACHINE.append(Popen(evchessARGS, stdin=PIPE, stdout=PIPE))
        except:
            self.log('exception', '#1')
            for M in self.MACHINE:
                M.kill()
            self.initialize=0
            self.endgame()
            return

        sleep(4)
        if GUI: self.Maximize["background"] = "gold"
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
            return
        
        
        
        if GUI: self.Maximize["background"] = "brown"
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
                    #print(line.decode('utf-8'))
                    if "MACname > " in line.decode('utf-8'):
                        self.MACnames[i] = line.decode('utf-8')[10:-1] 

            sleep(3)            


            
            self.MACHINE[1].stdin.write(bytearray('new\n','utf-8'))
            self.MACHINE[1].stdin.flush()
            
            self.MACHINE[0].stdin.write(bytearray('new\n','utf-8'))
            self.MACHINE[0].stdin.flush()
            sleep(0.3)
            self.MACHINE[0].stdin.write(bytearray('white\n','utf-8'))
            self.MACHINE[0].stdin.flush()
            sleep(0.3)
            self.MACHINE[0].stdin.write(bytearray('go\n','utf-8'))                
            self.MACHINE[0].stdin.flush()
            

        except BrokenPipeError:
            print("broken pipe @ "+str(self.number) + " while starting.")
            #self.log("broken pipe %s %s", "setup." % (self.MACnames[0],self.MACnames[1]))
            
            if (self.startuplog[0]): self.log(self.startuplog[0].decode('utf-8'), 'BLACK')
            if (self.startuplog[1]): self.log(self.startuplog[1].decode('utf-8'), 'WHITE')
            self.initialize = 0
            return

        if GUI: self.Maximize["background"] = "brown"
  

        try:
            for NAME in self.MACnames: self.MACcontent.append(open('machines/%s' % NAME, 'r').readlines())
        except FileNotFoundError:
            self.log("filename not found. Maybe coudn't be read properly by arena.","")
            self.initialize = 0
            return

        self.online = 1
        self.turn = 0      
        self.initialize=0

        if GUI:
            self.Maximize["background"] = "grey"
            self.Mnames["text"] = self.MACnames[0] + " X " + self.MACnames[1]
            self.visor.delete('1.0', END)
            self.visor.insert('1.0', self.board)
            self.switch["command"]
            self.setlimit["text"] = "0"

            

        self.startThread = None
        

        
    def Pout(self, readobj):
        for line in readobj:
            print(line.decode('utf-8'))

        
    def readmove(self):
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

        
        print(COLOR[self.turn] + " @  table " + str(self.number))
        
        if GUI:
            self.setlimit["background"] = "white"
            self.Maximize["background"] = "black"
            
        self.movelist = []




        """if (self.rounds_played % 17 == 0) and (self.rounds_played>3):
            print('chk')
            self.check_engine_health()"""

        
        
        try:
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


        for line in self.MACHINE[self.turn].stdout.readlines():
            #print(line.decode('utf-8'))
            line = line.decode('utf-8')[:-1]

            L = line.split(" ")
            if ("move" in L[0]) and (len(L)>1):
                    #print(">>>> %s"%L[1])
                    MOVE = L[1]
                    break
            else: self.movereadbuff.append(line)
                    

        if MOVE:
            for move in self.board.legal_moves:
                self.movelist.append(str(move))
            if MOVE in self.movelist:

                try:
                    self.board.push(chess.Move.from_uci(MOVE))
                    self.arena.move_read_reliability += 1
                    self.consec_failure=0
                    if GUI: self.setlimit["text"] = "0"
                    print("move done " + MOVE + "\n")
                except TypeError:
                    self.log("BOARD.PUSH ERROR.", MOVE)
                    print("BOARD.PUSH ERROR!.")
                    self.endgame()
                    
                if GUI: self.Vrefresh()
                
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
                    self.log("broken pipe while receiving move.", self.number)
                    self.endgame()
                    return
            else:
                print("error! Illegal move! "+ MOVE)
                self.log("error! Illegal move! "+ MOVE,0)
                self.arena.setcounter_illegalmove+=1
                
                #self.log(str(self.board),0)
                #self.log('engine internal board >>>>',0)
                
                self.MACHINE[self.turn].stdin.write(bytearray('dump\n','utf-8'))
                self.MACHINE[self.turn].stdin.flush()
                sleep(1)    
                try:
                    Hdump = self.MACHINE[self.turn].stdout.read().decode('utf-8')
                    
                    FLOG = open('log/log_illegal%i.txt' % self.arena.ROUND,'w+')
                    FLOG.write('illegal move. by %s. %s -> %s\n' % (COLOR[self.turn], self.MACnames[self.turn], MOVE))
                    FLOG.write('')
                    FLOG.write(Hdump)
                    FLOG.write(str(self.board))
                    FLOG.close()
                    
                except AttributeError:
                    pass

                self.endgame()
                return
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
                    self.endgame()
            if GUI:
                self.setlimit["text"] = str(self.consec_failure)
            
            if self.consec_failure > 27:
                print("restarting due to inactivity.")
                self.arena.setcounter_inactivity+=1
                self.consec_failure = 0
                
                self.endgame()
                
        if GUI:
            if self.number <= self.arena.looplimit:
                self.setlimit["background"] = "green"
                self.Maximize['background'] = "grey"
            else: self.setlimit["background"] = "red"
                

        
    def endgame(self):
        self.flagged_toend=0
        for machine in self.MACHINE:
            #print('killing %s' % machine.pid)
            call(['kill', '-9', str(machine.pid)])
            machine.terminate()
            self.MACHINE = []


        self.online = 0
        self.initialize = 0
        if GUI:
            self.switch["text"] = "off"
            self.switch["command"] = self.turnon
            self.Mnames["text"] = "idle"
            self.Maximize["background"] = "light grey"
            self.visor.delete('1.0',END)
            
    def sendresult(self, result):
        print("game ends @ " + str(self.number))

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
            if GUI: self.visor.insert('10.1', 'checkmate. black wins')
            #self.log('checkmate', self.MACnames[1])
            
        elif result < 0.5:
            result = '1-0'
            if GUI: self.visor.insert('10.1', 'checkmate. white wins')
            #self.log('checkmate', self.MACnames[0])
            
        elif result == 0.5:
            result = '1/2-1/2'
            if GUI: self.visor.insert('10.1', 'draw.')
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
        
        if GUI:
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


    #def log_wrongmove(self):


    def sendELO(self, winner):

        ELO = []
        index = []
        for C in range(len(self.MACcontent)):
            for L in range(len(self.MACcontent[C])):
                if 'stat_elo' in self.MACcontent[C][L].split(' = ')[0]:
                    ELO.append(int(self.MACcontent[C][L].split(' = ')[1][:-1]))
                    index.append(L)
        if len(ELO) == 2:
            if winner == 0.5:
                winner = 0
                base = 0
            else: base = 32
                
                
            DIF = ELO[winner] - ELO[1-winner]

            DIF = -round(DIF/16)

            DIF += base

            ELO[winner] += DIF
            ELO[1-winner] -= DIF

            
            try:
                for C in range(len(self.MACcontent)):
                    self.MACcontent[C][index[C]] = "stat_elo = %i\n"  % ELO[C]
                    F = open('machines/%s' % self.MACnames[C], 'w')
                    for line in self.MACcontent[C]:
                        F.write(line)
                    F.close()

            except:
                self.log('sending ELO failed.', '')

            

            print(ELO)
                               
        
if GUI:
    print('good')    
    #root = Tk()
    #root.wm_title("e-vchess arenaArray")
    #root.grid_columnconfigure(8, weight=1)
    #root.grid_rowconfigure(4, weight=1)
    #app = Application(master=root)
    #app.grid(sticky=NSEW)

    #root.resizable(False, False)
    #root.mainloop()
    app = Application()

else: app = Application()
