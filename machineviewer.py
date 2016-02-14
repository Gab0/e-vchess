#!/usr/python
from tkinter import *
from evchess_evolve import *

import xml.etree.ElementTree as ET

import sys

from time import sleep
from shutil import *

from os import remove
class Application(Frame):

    def show_machine(self):
        
        self.VIEWDUMP.grid_forget()
        self.blackboard.grid(column=5,row=0, sticky=NSEW,rowspan=10)
        
        Fo = open(self.DIR+'/'+self.machines[self.N].filename)
        self.blackboard.delete('1.0',END)
        for line in Fo.readlines():
            self.blackboard.insert(END,line)
        Fo.close()
        
        self.marker["text"] = self.N+1


        self.VIEW_wins["text"] = self.machines[self.N].TPARAMETERS[1].value
        
        self.VIEW_loss["text"] = self.machines[self.N].TPARAMETERS[3].value

        self.VIEW_games["text"] = self.machines[self.N].TPARAMETERS[0].value

        self.VIEW_draws["text"] = self.machines[self.N].TPARAMETERS[2].value

        self.VIEW_elo["text"] = self.machines[self.N].TPARAMETERS[5].value

        self.macname["text"] = self.machines[self.N].filename

        self.viewK["text"] = str(self.machines[self.N].TPARAMETERS[4].value)

        self.winrate["text"] = str(round(self.machines[self.N].TPARAMETERS[1].value/(self.machines[self.N].TPARAMETERS[0].value+1)*100, 3)) + "%"
        self.drawrate["text"] = str(round(self.machines[self.N].TPARAMETERS[2].value/(self.machines[self.N].TPARAMETERS[0].value+1)*100, 3)) + "%"


        self.paramNAMES = []
        for VW in range(len(self.paramVIEWER)):
            
            self.paramVIEWER[VW][1]['text'] = self.machines[self.N].PARAMETERS[VW].value
            

            

     
    def scrollmachinesU(self):
     
        self.N+=1
        if self.N >= len(self.machines): self.N=0

        self.show_machine()

    def scrollmachinesD(self):
        self.N-=1
        if self.N < 0: self.N = len(self.machines)-1

        self.show_machine()


    def scrollbestmachines(self, ceiling, direction):
        if ceiling: ceiling = self.machines[self.N].TPARAMETERS[5].value
        else: ceiling = 66666

        VECTOR = True
        Z = 0
        index = 0
        
        if direction < 0:
            VECTOR = False
            Z = self.machines[self.N].TPARAMETERS[5].value
            while Z < ceiling:
                index = random.randrange(len(self.machines))
                Z = self.machines[index].TPARAMETERS[5].value
        
        

        for M in range(len(self.machines)):
            candidate = self.machines[M].TPARAMETERS[5].value
            
            if (((candidate > Z) and (candidate < ceiling) and (VECTOR)) or
               ((candidate < Z) and (candidate > ceiling) and not (VECTOR))):
                Z = self.machines[M].TPARAMETERS[5].value
                index = M








        self.N=index
        self.show_machine()
                

    def savemac(self):
        setmachines(self.machines, 1)
        self.show_machine()
        print('machines saved.')
        
    def mutatemac(self):
        self.machines = mutatemachines(3, self.machines)
        self.show_machine()


    def savemactext(self):
        Fo = open(self.DIR+'/'+self.machines[self.N].filename, "w")
        Fo.write(self.blackboard.get('1.0', END))
        Fo.close


    def show_attr_dump(self , attribute):

        self.blackboard.grid_forget()
        self.renew_VIEWDUMP_canvas(1)


        DUMP = read_param_dump(attribute)
        #print (DUMP)
        Label(text=DUMP[0]).grid(in_=self.VIEWDUMP,column=0,row=0)
        
        for L in range(1,len(DUMP)):

            Label(text=DUMP[L][0]).grid(in_=self.VIEWDUMP, column=0,row=L)
            Label(text=DUMP[L][1] + " | " + str(round(int(DUMP[L][1]) / (int(DUMP[L][4])+1) * 100,3))+"%", fg="green").grid(in_=self.VIEWDUMP, column=1,row=L)
            Label(text=DUMP[L][2] + " | " + str(round(int(DUMP[L][2]) / (int(DUMP[L][4])+1) * 100,3))+"%", fg="grey").grid(in_=self.VIEWDUMP, column=2,row=L)
            Label(text=DUMP[L][3] + " | " + str(round(int(DUMP[L][3]) / (int(DUMP[L][4])+1) * 100,3))+"%", fg="red").grid(in_=self.VIEWDUMP, column=3,row=L)
            Label(text=DUMP[L][4]).grid(in_=self.VIEWDUMP, column=4,row=L)
            Label(text=DUMP[L][5], fg="brown").grid(in_=self.VIEWDUMP, column=5,row=L)


            
    def createWidgets(self):
        self.Larrow = Button(self)
        self.Larrow["text"] = "<-"
        self.Larrow["fg"]   = "red"
        self.Larrow["height"] = 6
        self.Larrow["width"] = 9
        self.Larrow["command"] = self.scrollmachinesD
        self.Larrow.grid(column=0,row=0,rowspan=8)


        self.Rarrow = Button(self)
        self.Rarrow["text"] = "->"
        self.Rarrow["fg"]   = "red"
        self.Rarrow["height"] = 6
        self.Rarrow["width"] = 9
        self.Rarrow["command"] = self.scrollmachinesU
        self.Rarrow.grid(column=2,row=0,rowspan=8)
        

        self.marker = Button(self)
        self.marker["text"] = self.N
        self.marker["height"] = 3
        self.marker["width"] = 4
        self.marker.grid(column=1,row=0,rowspan=8)

        self.viewK = Button(self)
        self.viewK["text"] = "0"
        self.viewK["fg"] = "purple"
        self.viewK.grid(column=1,row=2,rowspan=6)

        self.winrate = Button(self)
        self.winrate["text"] = "x"
        self.winrate.grid(column=1,row=5)

        self.drawrate = Button(self)
        self.drawrate["text"] = 'x'
        self.drawrate["fg"] = "grey"
        self.drawrate.grid(column=1,row=6)   

        
        self.VIEW_wins = Button(self)
        self.VIEW_wins["fg"] = "green"
        self.VIEW_wins.grid(column=0,row=9)

        self.VIEW_loss = Button(self)
        self.VIEW_loss["fg"] = "red"
        self.VIEW_loss.grid(column=2,row=9)

        self.VIEW_draws = Button(self)

        
        self.VIEW_draws.grid(column=1,row=8)
        
        self.VIEW_games = Button(self)

        self.VIEW_games["fg"] = "grey"
        self.VIEW_games.grid(column=1,row=10)


        self.VIEW_elo = Button(self)
        self.VIEW_elo["command"] = lambda: self.scrollbestmachines(0,0)
        self.VIEW_elo.grid(column=1, row=7)

        self.scroll_eloL = Button(self)
        self.scroll_eloL["text"] = '<<'
        self.scroll_eloL["fg"] = 'red'
        self.scroll_eloL["command"] = lambda: self.scrollbestmachines(1,-1)
        self.scroll_eloL.grid(column=0, row=7)

        self.scroll_eloR = Button(self)
        self.scroll_eloR["text"] = '>>'
        self.scroll_eloR["fg"] = 'red'
        self.scroll_eloR["command"] = lambda: self.scrollbestmachines(1,1)
        self.scroll_eloR.grid(column=2, row=7)        




        self.macname = Button(self)
        self.macname["text"] = self.machines[self.N].filename
        self.macname["command"] = self.savemactext
        self.macname.grid(column=1,row=9, sticky=NSEW, padx=3, pady=4)
        
        self.mutate = Button(self)
        self.mutate["text"] = "mutate"
        self.mutate["command"] = self.mutatemac
        self.mutate.grid(column=5,row=10, sticky=NSEW, padx=3, pady=4)

        self.save = Button(self)
        self.save["text"] = "save machines"
        self.save["command"] = self.savemac
        self.save.grid(column=5,row=11, sticky=NSEW, padx=3, pady=4)



        self.menubar = Menu(root)
        self.popmenu = Menu(self.menubar)
        self.clearmenu = Menu(self.menubar)
        self.machinemenu = Menu(self.menubar)

        self.clearmenu.add_command(label="Clear Scores", command=self.clearscores)
        self.clearmenu.add_command(label="Clear Attr Dump", command=self.TOcleardump)
        self.menubar.add_cascade(label="CLEAR", menu = self.clearmenu)
        
        self.popmenu.add_separator()
        self.popmenu.add_command(label="DEL the Worst", command = self.TOdeltheworst)
        self.popmenu.add_command(label="CLONE the Best", command = self.TOclonethebest)
        self.popmenu.add_separator()
        self.popmenu.add_command(label="DUMP Stats", command = self.TOdumpstats)
        self.popmenu.add_separator()
        self.popmenu.add_command(label="Populate 16", command = lambda: self.TOpopulate(16))
        self.popmenu.add_command(label="Populate 128", command = lambda: self.TOpopulate(128))
        self.popmenu.add_command(label="Populate 256", command = lambda: self.TOpopulate(256))
        self.popmenu.add_separator()
        self.popmenu.add_command(label="Create 4 Hybrid", command = lambda: self.TOcreatehybrid(4))
        self.popmenu.add_command(label="Create 16 Hybrids", command = lambda: self.TOcreatehybrid(16))
        self.popmenu.add_separator()
        self.popmenu.add_command(label="Create 16 from Template", command = lambda: self.TOpopulatetemplate(16))
        self.popmenu.add_separator()
        self.popmenu.add_command(label="Routine Procedure", command = self.TOroutineprocedures)
        self.popmenu.add_command(label="send best to TOP", command = self.TOautotop)
        self.popmenu.add_separator()
        self.popmenu.add_command(label="ABORT bad machines", command = self.abort_machines)
        
   
        self.menubar.add_cascade(label="POPULATION", menu = self.popmenu)

        self.machinemenu.add_command(label="Send to TOP", command = lambda: self.sendtobest(self.N))
        self.machinemenu.add_command(label="DELETE machine", command = self.delete_machine)
        self.menubar.add_cascade(label="MACHINE", menu = self.machinemenu)

        
        self.menubar.add_command(label="QUIT", command = root.destroy)
        root.config(menu=self.menubar)

    def create_param_viewer(self):

        self.paramVIEWER = []
        self.paramNAMES = []
        for i in range(len(self.machines[0].PARAMETERS)):
            self.paramVIEWER.append([Label(self), Button(self)])
            self.paramNAMES.append(self.machines[self.N].PARAMETERS[i].name)
            
            self.paramVIEWER[i][0]["text"] = self.paramNAMES[i]
            self.paramVIEWER[i][1]['command'] = self.show_attr_dump(self.paramNAMES[i])
            
            
            self.paramVIEWER[i][0].grid(column=3, row=i)
            self.paramVIEWER[i][1].grid(column=4, row=i)
            



    def sendtobest(self, N):
        copyfile(self.DIR+'/'+self.machines[N].filename, self.DIR+'/top_machines/'+self.machines[N].filename)
        Fo = open(self.DIR+'/top_machines/machines.list', 'a+')
        Fo.write(self.machines[N].filename+'\n')
        print('machine %s sent to top.' % self.machines[N].filename)
        self.machines[N].onTOP=1
        self.machines[N].write()
        






        
#actions called by upper menu.
    def clearscores(self):
        if self.W == 1:
            for individual in self.machines:
                for param in individual.TPARAMETERS:
                    param.value = 0
                    if param.name == "stat_elo":
                        param.value = 1000
                individual.stat_draws=0
                individual.stat_wins=0
                individual.stat_games=0
                individual.stat_loss=0
                individual.K=0


                individual.dumped_wins=0
                individual.dumped_games=0
                individual.dumped_loss=0
                individual.dumped_draws=0
                individual.dumped_K=0

                
    def TOdeltheworst(self):
        self.machines = deltheworst_clonethebest(self.machines, -1)
        

    def TOclonethebest(self):
        self.machines = deltheworst_clonethebest(self.machines, 1)
        


    def TOdumpstats(self):
        if self.W ==1:
            for individual in self.machines:
                dump_all_paramstat(individual)
        setmachines(self.machines, 1)
        print('stats dumped.')

    def TOpopulate(self, NUM):
        self.machines = populate(self.machines, NUM)
        print('created 16 standard postmutated individuals.')


    def TOroutineprocedures(self):
        self.TOdumpstats()
        self.TOdeltheworst()
        self.TOclonethebest()
        self.savemac()
        
    def TOcleardump(self):
        if os.path.isfile(self.DIR + "/paramstats.xml"):
            os.remove(self.DIR + "/paramstats.xml")
            print('dump file cleared.')



    def TOautotop(self):
        for IND in select_best_inds(self.machines):
            self.sendtobest(IND)
            

    def TOcreatehybrid(self, NUM):
        for i in range(NUM):
            CHILD = create_hybrid(self.machines)            
            if CHILD: self.machines.append(CHILD)
        self.savemac


    def TOpopulatetemplate(self, NUM):
        for i in range(NUM):
            CHILD = clone_from_template()
            if CHILD: self.machines.append(CHILD)
        self.savemac


    def renew_VIEWDUMP_canvas(self, alreadyexists):
        if alreadyexists==1:
            self.VIEWDUMP.destroy()

        self.VIEWDUMP = Canvas(self,width=1000,height=500,scrollregion=(0,0,500,500))    
        self.VIEWDUMP.config(yscrollcommand=self.VIEWDUMPscrollbar.set)
        self.VIEWDUMP.grid(in_=self,column=5,row=0, sticky=NSEW,rowspan=6)

    def delete_machine(self):
        self.machines.pop(self.N)
        self.savemac()

    def abort_machines(self):
        MARKED = []
        for X in range(len(self.machines)):
            if len(str(self.machines[X].filename)) > 7:
                print(self.machines[X].filename + ' aborted.')
                MARKED.append(X)
                break
            for i in range(3):
                
                if self.machines[X].PARAMETERS[i].value < 0:
                    print(self.machines[X].filename + ' aborted.')
                    MARKED.append(X)
                    break
                
        MARKED = list(reversed(MARKED))

        for N in range(len(MARKED)):
            remove(Fdir+'/'+self.machines[MARKED[N]].filename)
            self.machines.pop(MARKED[N])


                    
#INIT
    def __init__(self, master=None):
        Frame.__init__(self, master)
        self.W=0
        self.paramVIEWER = []
        self.VIEWDUMPscrollbar = Scrollbar(self, orient="vertical")

        self.renew_VIEWDUMP_canvas(0)
     
        self.VIEWDUMPscrollbar["command"] = self.VIEWDUMP.yview
        self.VIEWDUMP['yscrollcommand'] = self.VIEWDUMPscrollbar.set
        self.VIEWDUMP.config(yscrollcommand=self.VIEWDUMPscrollbar.set)
        self.VIEWDUMPscrollbar.grid(column=6,row=0, sticky=NSEW, rowspan=10)
        
        
        self.blackboard = Text(self, font=("Helvetica",16))
        self.blackboard.grid(column=5,row=0, sticky=NSEW,rowspan=10)

        
        self.DIR = Fdir
        self.N= 0
        self.machines = loadmachines()
        setmachines(self.machines, 1)
        self.createWidgets()
        self.create_param_viewer()
        self.show_machine()
        self.W=1
        
        self.savemac()

 
root = Tk()
root.wm_title("e-vchess evolutionary machines viewer")
root.grid_columnconfigure(1, weight=1)
root.grid_rowconfigure(1, weight=1)
app = Application(master=root)
app.grid(row=1,sticky=NSEW)



root.resizable(False, False)
app.mainloop()


