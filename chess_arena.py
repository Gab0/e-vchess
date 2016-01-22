
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

from evchess_evolve import *

from random import randrange

from psutil import *

import gc

# path to e-vchess executable and the directory where machines are stored, respectively.
evchessP = "/home/gabs/NetBeansProjects/CppApplication_1/dist/Release/GNU-Linux/e-vchess"
machineDIR =  "/home/gabs/Desktop/e-vchess/machines"

evchessARGS = [evchessP, "-MD", machineDIR]


GUI = 1

COLOR = {0: 'WHITE', 1: 'BLACK'}
TABLEBOARD = []


if (len(sys.argv) > 0) and ('--nogui' in sys.argv):
    GUI = 0






class Application(Frame):

    def __init__(self, master=None):
        Frame.__init__(self, master)

        self.Cycle = False
        self.looplimit = 0

        #sets the number of simultaneous chess tables to be created and played. Estimations based on your RAM: wisely set max tables in increments of 8 for each GB on your machine.
        #for very long runs, the number of simultaneous engines (two per table) should be multiplied by the maximum engine size allowed(std 120mb), so 20 tables should occupy 6gb of ram.
        TABLECOUNT = 16
        #number of tables to be shown on each row of machines.
        TABLEonROW = 8

        self.TIME = time()
        k=0
        j=0
        for i in range(TABLECOUNT):
            TABLEBOARD.append(table(self, master=master))
            if GUI: TABLEBOARD[i].grid(column=k,row=j,stick=NSEW)
            k+=1
            if k == TABLEonROW:
                k=0
                j+=1
        
      
        self.menubar= Menu(master)
      
        self.Ccycle = self.menubar.add_command(label='cycle thru', command = self.startcycle)
        self.menubar.add_command(label="Kill 'em All", command = self.killemall)
        self.menubar.add_command(label='show/hide ALL', command = self.showhideall)

        if GUI: master.config(menu=self.menubar)
        self.showhide_ = 0




        #self.config(menu=self.menubar)
        self.setlooplimit(0)


        if (len(sys.argv) > 0) and ('--go' in sys.argv):
            self.setlooplimit(TABLECOUNT-1)
            self.startcycle()




    def startcycle(self):

        self.CYCLE = threading.Thread(target=self.gocycle)
        self.CYCLE.start()
        
    def gocycle(self):
        SLEEPTIME = 1.3
        if SLEEPTIME < 1: SLEEPTIME = 1.3
        i=0
        self.Cycle = True
        
        if GUI:
            self.menubar.entryconfigure(1, label='stop cycle')
            self.menubar.entryconfigure(1, command=self.killcycle)
            
        TIME = time()
        TABLEBOARD[0].log("STARTING CYCLE", str(strftime("%d/%b - %H:%M:%S")))

        while self.Cycle:
            TIME = time()-TIME
            if i % 10 == 0:
                system('clear')


            #each N rounds, do maintenance management in order to get best evolving performance.
            if (i % 5000 == 0) and (i != 0):
                self.routine_pop_management()
            
            
            print("ROUND " + str(i) + " t= " + str(TIME) + " >>>>>>>>>>")
            for t in range (self.looplimit+1):
                if virtual_memory()[2] > 91: self.memorylimit=1
                else: self.memorylimit=0
                
                if self.Cycle:
                    if TABLEBOARD[t].online == 1:
                        TABLEBOARD[t].readmove()
                    else:
                        if not self.memorylimit:
                            TABLEBOARD[t].newmatch_thread(0)
                            if virtual_memory()[2] > 92: self.memorylimit=1
            if i == 0:
                sleep(6)
            i+=1

            if i % 16 == 0: gc.collect() #free memory used by executed newmatch threads.
            if i > 100000: i=0
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
                table.turnoff()

    def killunused(self):
        for T in range(len(TABLEBOARD)):
            if T > self.looplimit:
                TABLEBOARD[T].turnoff()


    def routine_pop_management(self):
        population = loadmachines()

        #for individual in population:
            #dump_all_paramstat(individual)
        
        population = deltheworst_clonethebest(population, -1)
        population = deltheworst_clonethebest(population, 1)
        mutatemachines(3, population)

        setmachines(population,1)

        print('routine management done.')
        TABLEBOARD[0].log('routine management done.',0)


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



        
class table(Frame):
    def __init__(self, arena, master=None):
        if GUI: Frame.__init__(self, master)
        self.board = chess.Board()
        self.online = 0
        self.movelist=[]


        self.arena = arena


        
        self.consec_failure=0
  
        self.turn = 0
        self.number = len(TABLEBOARD)

        self.rounds_played=0
        
        self.visible = 0
        #TABLEBOARD.append(self)

        self.flagged_toend = 0
        if GUI: self.setWidgets()
        #self.newmatch()

        self.startThread = None

        self.initialize=0
    def newmatch_thread(self, kill):
        if not kill:
            if (self.startThread) and(self.initialize == 0):
                self.startThread.join()
                
            else:
                try:
                    self.startThread = threading.Thread(target=self.newmatch)
                    self.startThread.start()
                except RuntimeError:
                    print()
        else:
            if self.startThread:
                self.startThread.join()
                
                self.startThread = None

                
    def newmatch(self):
        if self.initialize: return
        

        
        self.MACHINE = []
        self.MACcontent = []
        
        self.rounds_played=0
        
        self.initialize = 1

        
        try:
            self.MACHINE.append(Popen(evchessARGS, stdin=PIPE, stdout=PIPE))

            self.MACHINE.append(Popen(evchessARGS, stdin=PIPE, stdout=PIPE))
        except OSError:
            self.arena.shrinkloop()
            for M in self.MACHINE:
                M.kill()
            return -1

        sleep(4)

        if GUI: self.Maximize["background"] = "grey"
        self.MACnames = ['zero','zero']

        flags = fcntl(self.MACHINE[0].stdout, F_GETFL) # get current p.stdout flags
        fcntl(self.MACHINE[0].stdout, F_SETFL, flags | O_NONBLOCK)
        fcntl(self.MACHINE[1].stdout, F_SETFL, flags | O_NONBLOCK)#fcntl(self.Bmachine.stdout, F_SETFL, flags | O_NONBLOCK) >>>>>?


        
        self.board.reset()
        
        
        #self.Pout(self.Wmachine.stdout.readlines())
        #self.Pout(self.Bmachine.stdout.readlines())
        
        #sleep(1)
        self.startuplog = []
        try:
            #self.startuplog.append(self.MACHINE[1].stdout.read())
            #self.startuplog.append(self.MACHINE[0].stdout.read())
            #self.MACHINE[0].stdout.flush()
            
            #self.MACHINE[1].stdout.flush()

            for i in [0,1]:
                for line in self.MACHINE[i].stdout.readlines():
                    #print(line.decode('utf-8'))
                    if "line > " in line.decode('utf-8'):
                        self.MACnames[i] = line.decode('utf-8')[7:-1] 

            sleep(1)            


            
            self.MACHINE[1].stdin.write(bytearray('new\n','utf-8'))
            self.MACHINE[1].stdin.flush()
            
            self.MACHINE[0].stdin.write(bytearray('new\n','utf-8'))
            self.MACHINE[0].stdin.write(bytearray('black\nwhite\n','utf-8'))
            self.MACHINE[0].stdin.write(bytearray('go\n','utf-8'))                
            self.MACHINE[0].stdin.flush()
            

        except BrokenPipeError:
            print("broken pipe @ "+str(self.number) + " while starting.")
            self.log("broken pipe", "setup.")
            
            if (self.startuplog[0]): self.log(self.startuplog[0].decode('utf-8'), 'BLACK')
            if (self.startuplog[1]): self.log(self.startuplog[1].decode('utf-8'), 'WHITE')
            self.initialize = 0
            return

        
        self.online = 1
        self.turn = 0        


        for NAME in self.MACnames: self.MACcontent.append(open('machines/%s' % NAME, 'r').readlines())



        self.initialize=0

        if GUI:
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
        if self.initialize==1: return
        print(COLOR[self.turn] + " @  table " + str(self.number))
        
        if GUI:
            self.setlimit["background"] = "white"
            self.Maximize["background"] = "black"
            
        self.movelist = []




        if (self.rounds_played % 17 == 0) and (self.rounds_played>3):
            print('chk')
            self.check_engine_health()

        
        
        try:
            self.MACHINE[self.turn].stdout.flush()
        except BrokenPipeError:
            print("broken pipe @ "+str(self.number) + " while gaming.")
            self.log("broken pipe", self.number)
            self.endgame()
            return
        except IndexError:
            self.log("read of move with unintialized machines.", self.number)
            self.turnoff()
            return
        
        SUCCESS = 0   
        for line in self.MACHINE[self.turn].stdout.readlines():
            #print(line.decode('utf-8'))
            
            L = line.decode('utf-8')[:-1].split(" ")
            if (L[0]=="move") and (len(L)>1):
                for move in self.board.legal_moves:
                    
                    self.movelist.append(str(move))
                    #print(str(move))


                if L[1] in self.movelist:
                    SUCCESS=1
                    self.consec_failure=0
                    if GUI: self.setlimit["text"] = "0"
                    
                    print("move done " + L[1] + "\n")



                    try:
                        self.board.push(chess.Move.from_uci(L[1]))
                    except TypeError:
                        self.log("BOARD.PUSH ERROR.", L[1])
                        print("BOARD.PUSH ERROR!.")
                        self.endgame()
                        
                    if GUI: self.Vrefresh()
                    
                    if self.board.is_checkmate():
                        self.sendresult(self.turn)
                        return
                    if self.board.is_stalemate() or self.board.is_insufficient_material() or self.board.can_claim_fifty_moves(): #or self.board.can_claim_threefold_repetition() 
                        self.sendresult(0.5)
                        return
                    
                    self.turn = 1-self.turn
                    self.MACHINE[self.turn].stdin.write(bytearray(L[1]+'\n','utf-8'))
                    self.rounds_played+=1

                    try:
                        self.MACHINE[self.turn].stdin.flush()
                    except BrokenPipeError:
                        print("broken pipe @ " + str(self.number) + " while receiving move.")
                        self.log("broken pipe while receiving move.", self.number)
                        self.turnoff()
                        return
                else:
                    print("error! illegal move! "+ L[1])
                    self.log('illegal move. by %s.' % COLOR[self.turn], self.MACnames[self.turn] + " " + L[1])
                    self.log(L[0],L[1])
                    self.log(str(self.board),0)
                    self.log('engine internal board >>>>',0)
                    
                    self.MACHINE[self.turn].stdin.write(bytearray('show\n','utf-8'))
                    self.MACHINE[self.turn].stdin.flush()
                    sleep(1)
                    self.log(self.MACHINE[self.turn].stdout.read().decode('utf-8'),0)
                                                 

                    self.turnoff()
                    return
        if SUCCESS==0:
            self.consec_failure+=1
            if GUI:
                self.setlimit["text"] = str(self.consec_failure)
            
            if self.consec_failure > 14:
                print("restarting due to inactivity.")
                self.consec_failure = 0
                
                self.turnoff()
                
        if GUI:
            if self.number <= app.looplimit:
                self.setlimit["background"] = "green"
                self.Maximize['background'] = "grey"
            else: self.setlimit["background"] = "red"
                

        
    def endgame(self):
        self.flagged_toend=0
        for machine in self.MACHINE:
            machine.terminate()
            call(['kill', '-9', str(machine.pid)])
            self.MACHINE = []


        self.online = 0
        if GUI:
            self.setlimit["text"] = "off"        
            self.Maximize["background"] = "light grey"

    def sendresult(self, result):
        print("game ends @ " + str(self.number))

        self.sendELO(result)
        
        if result==1:
            result = '0-1'
            if GUI: self.visor.insert('10.1', 'checkmate. black wins')
            self.log('checkmate', self.MACnames[1])
            
        if result ==0:
            result = '1-0'
            if GUI: self.visor.insert('10.1', 'checkmate. white wins')
            self.log('checkmate', self.MACnames[0])
            
        if result == 0.5:
            result = '1/2-1/2'
            if GUI: self.visor.insert('10.1', 'draw.')
            self.log('draw', '1/2')
            

        for MAC in self.MACHINE:    
            MAC.stdin.write(bytearray('result '+ result + '\n', 'utf-8'))
            MAC.stdin.flush()
        self.online = 0
        
        #self.endgame()
        
    def setWidgets(self):
        self.visor = Text(self, height=10,width=16, borderwidth=4, relief=GROOVE)
        #self.visor.grid(column=0,row=1,rowspan=6)

        self.switch = Button(self)
        self.switch["text"] = "off"
        self.switch["command"] = self.turnon
        #self.switch.grid(column=1,row=1)

        self.play = Button(self)
        self.play["text"] = "play"
        self.play["command"] = self.readmove
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
        self.newmatch()
        
        if GUI:
            self.switch["text"] = "on"
            self.switch["command"] = self.turnoff
        
    def turnoff(self):
        self.endgame()
        
        if GUI:
            self.switch["text"] = "off"
            self.switch["command"] = self.turnon
            self.Mnames["text"] = "idle"
            self.Maximize["background"] = "light grey"
            self.visor.delete('1.0',END)

        
    def Vrefresh(self):
        self.visor.delete('1.0',END)
        self.visor.insert(END, self.board)


    def setlooplimit(self):
        app.setlooplimit(self.number)    


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
            if MEMUSAGE[0] > 110000000:
                print('terminating table, memory limit overriden by %s' % self.MACnames[M])
                self.log('terminating table, memory limit overriden by ', self.MACnames[M])
                
                self.turnoff()
            


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
            self.visor.grid(column=0,row=1,rowspan=6)
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

            

            for C in range(len(self.MACcontent)):
                self.MACcontent[C][index[C]] = "stat_elo = %i\n"  % ELO[C]
                F = open('machines/%s' % self.MACnames[C], 'w')
                for line in self.MACcontent[C]:
                    F.write(line)
                F.close()



            

            print(ELO)
                               
        
if GUI:
        
    root = Tk()
    root.wm_title("e-vchess arenaArray")
    root.grid_columnconfigure(8, weight=1)
    root.grid_rowconfigure(4, weight=1)
    app = Application(master=root)
    app.grid(sticky=NSEW)

    root.resizable(False, False)
    root.mainloop()

else: app = Application()
