#!/bin/python

from subprocess import call

from random import randrange

from json import loads, dumps

from chessArena import settings

settings.initialize()


def loadscores():
    try:
        F = open(settings.TOPmachineDIR + '/scoreData')
        Content = F.read()
        ScoreData = loads(Content)
        return ScoreData
    except:
        return {}
        pass

def savescores(DATA):
    F = open(settings.TOPmachineDIR + '/scoreData', 'w')

    F.write(dumps(DATA))
    F.close()
    
def ModifyScore(DATA, MacName, Operator):
    try:
        DATA[MacName] += Operator
    except:
        DATA[MacName] = Operator

    return DATA

#print(VerboseMove)

ScoreData = loadscores()
print("""
loading machines from Hall of Fame.

  Choose your opponent:
              """)

MachineListLocation = settings.TOPmachineDIR + '/machines.list'
MachineList = open(MachineListLocation, 'r').readlines()


LegalMachines = ['random']
for line in MachineList:
    if '.mac' in line:
        LegalMachines.append(line[:-1])

for M in range(len(LegalMachines)):
    print('%i - %s' % (M, LegalMachines[M]))
print("")

userchoice = None
while userchoice == None:
    userchoice = input(">>>")
    try:
        N = int(userchoice)
        assert(N <= len(LegalMachines))
    except:
        userchoice = None
        print("Invalid inpuyt. Try again.")
        pass
    
if N == 0:
    N = randrange(1,len(LegalMachines))


print('Loading %s. glhf' % LegalMachines[N])
Callargs = [ settings.enginebin, "--deep", '4', '--xdeep', '1' ] + [ '--specific', LegalMachines[N] ] 
engineCALL = " ".join(Callargs)

Command = ['xboard', '-fcp', engineCALL]

X = call(Command)

print("Did this machine have win the game? [y/n]")

FeedBack = None

while FeedBack == None:
    FeedBack = input('>>>').lower()
    try:
        assert(FeedBack in ['y', 'n'])
    except:
        FeedBack = None
        print("Invalid inpuyt. Try again.")
        pass

if FeedBack == 'y':
    ScoreData = ModifyScore(ScoreData, LegalMachines[N], 1)
    print("logically.")
else:
    ScoreData = ModifyScore(ScoreData, LegalMachines[N], -1)
    print("a bad day for the computer age.")


savescores(ScoreData)
