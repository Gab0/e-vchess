#!/usr/python
from tkinter import *
#from evchess_evolve import *
import evchess_evolve
import xml.etree.ElementTree as ET

import sys

from time import sleep
from shutil import *

from os import remove
from os import path

from evchess_evolve.core import *




class Application(Frame):

    def show_machine(self):
        
        self.VIEWDUMP.grid_forget()
        self.blackboard.grid(column=5,row=0, sticky=NSEW,rowspan=10)

        MACfile = self.DIR+'/'+self.machines[self.N].filename
        self.blackboard.delete('1.0',END)
        if os.path.isfile(MACfile):
            Fo = open(MACfile)
            
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
            if self.machines[self.N].PARAMETERS[VW].locked: self.paramVIEWER[VW][1]['bg'] = 'red'

     
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
        setmachines(self.machines)
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
        self.macname["width"] = 9
        self.macname["height"] = 2
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

        self.Logo = PhotoImage()

        self.Logo.configure(data=LOGO)
        #self.Logo = self.Logo.zoom(2,2)
        self.logoframe = Label(self, image=self.Logo)
        self.logoframe.grid(column=1,row=0,rowspan=4)

        self.menubar = Menu(root)
        self.popmenu = Menu(self.menubar)
        self.clearmenu = Menu(self.menubar)
        self.machinemenu = Menu(self.menubar)
        self.actionmenu = Menu(self.menubar)
        
        self.clearmenu.add_command(label="Clear Scores", command=self.clearscores)
        self.clearmenu.add_command(label="Clear Attr Dump", command=self.TOcleardump)
        self.menubar.add_cascade(label="CLEAR", menu = self.clearmenu)
        
        self.popmenu.add_separator()
        self.popmenu.add_command(label="DEL the Worst", command = lambda: self.TOdeltheworst(1))
        self.popmenu.add_command(label="DEL 32 Worst", command = lambda: self.TOdeltheworst(32))
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

        self.machinemenu.add_command(label="Randomize Machine", command = lambda: self.machines[self.N].randomize())
        self.machinemenu.add_command(label="Send to TOP", command = lambda: self.sendtobest(self.N))
        self.machinemenu.add_command(label="DELETE machine", command = self.delete_machine)
        self.menubar.add_cascade(label="MACHINE", menu = self.machinemenu)

        self.actionmenu.add_command(label="Switch to Statlock Cycle.", command = self.TOswitchstatlock)
        self.menubar.add_cascade(label="ACTION", menu = self.actionmenu)
        
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

                
    def TOdeltheworst(self, NUMBER):
        self.machines = deltheworst_clonethebest(self.machines, -NUMBER, 1)
        

    def TOclonethebest(self):
        self.machines = deltheworst_clonethebest(self.machines, 1, 1)
        


    def TOdumpstats(self):
        if self.W ==1:
            for individual in self.machines:
                dump_all_paramstat(individual)
        setmachines(self.machines)
        print('stats dumped.')

    def TOpopulate(self, NUM):
        self.machines = populate(self.machines, NUM)
        print('created 16 standard postmutated individuals.')


    def TOroutineprocedures(self):
        self.TOdumpstats()
        self.TOdeltheworst(1)
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
        self.savemac()


    def TOpopulatetemplate(self, NUM):
        for i in range(NUM):
            CHILD = clone_from_template()
            if CHILD: self.machines.append(CHILD)
        self.savemac()



    def TOswitchstatlock(self):
        self.machines = PrepareCyclingStatLock(self.machines)

    def TOrandomizemachine(self):
        self.machines[self.N].randomize()



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
        setmachines(self.machines)
        self.createWidgets()
        self.create_param_viewer()
        self.show_machine()
        self.W=1
        
        self.savemac()


        
LOGO = """R0lGODlhUAB6AOf/AAkGCwsHDQYMDw0KDwoMCA8MERMNDAoQEhEOEg8RDg8QGBIQFBQSFRITGhUTFxAVFxQUHBYUFxcVGRUWHRUXFBkXGhcYHxUZGxsZHBwaHhUcIhobIhgcHhscGh4cHyAcGxwdJR8dIBsfIB4fJyEfIhshJx0hIyAhH
yAhKSMhJCAkJiMjKyUjJh4lKyQlIyUlLSclKCknKygoJiIpMCUpKycoMCspLCgsLiorMywrLiYtMywtKy4sMDIrMC8tMS0uNSwwPCoxOC0xMzIwMzcvNDAxODIyMDQyNTY0OC82PDI2ODQ1PTU2NDk3Ojs5PTQ7QTc7PTk6Qjo7ODc7SD48QD0/PD0+
RkA+QjtAQkJAQ0VBP0RCRjxESkJEQUNDTEZESEhGSkZHUExHRkRJS0dJRktJTEdMTkpLU01LT0tNSk9NUExQUlFPUk9QWFBRT1JQVEpSWVVRUFRSVlZUWFtTWFJWWFRWU1RVXllWWltXVltZXV5ZWFlaZFRcY1hcXlpcWV1bXmNaX15cYGBeYltgYl9
gXl9faGRfXmJgZFpiaWViZmJjbWdjYWNlYmFlaGdmXmhmaWJpcGdocWpobGZqbG1pZ2lraG1qbmxua29tcWtvcnJvc3Rvbmxze3JzcHRydm90dnJzfHh2enZ4dXR4e3l4cHx3dnZ4gnt5fXV9hH58gIF8e3l+gHx+e3x9hoB+gnqBiYOBhYaBgH6DhYK
EgYaEiISIi4mHi4eJhoeIkYqLlI6MkImOkJKNjI2PjI6TlpGTkJSSlpKSnJWXlJmXm5SZm5mao56coJmeoKOgpZ2ipaCin5ujq6akqqKnqaqorKiqp6arrqSstK6ssLGvs6+vuayxtLGzr7Wyt7W2wLO4ura4tbm3u7a7vbm+wcC9wrjAyL7AvcPBx
b7DxcfFysHHysbIxcvIzcXKzc7L0MvNysjO0NLP1MzR08/Rzs/U19LU0dbT2NHX2dPY29nX29ja1trZ39Xb3dnf4d/k5+Tj6ePl4ufo7Ovs6e/19////yH+EUNyZWF0ZWQgd2l0aCBHSU1QACH5BAEKAP8ALAAAAABQAHoAAAj+AP8JHEiwoMGDCP/d
2lVrV6yEECNKnDjRky9as1h9uqQHTxY8lxRB2kTL1i1eFFOqnOgLWCM7f6pI2XHDhs0YNnLktDHkCptNrGIFW0m0qCdLf6QYkSHDhQycNmJAtQmDxQubV8xAUnWrqNeIR93ckAHDBYkTTm0oobKlDJUrSGywmMtihV0dWACh/Mp3IK9ZdqDciJHixNk
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"""



 
root = Tk()
root.wm_title("e-vchess evolutionary machines viewer")
root.grid_columnconfigure(1, weight=1)
root.grid_rowconfigure(1, weight=1)
app = Application(master=root)
app.grid(row=1,sticky=NSEW)



root.resizable(False, False)
app.mainloop()


