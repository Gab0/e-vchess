from os import listdir, remove


def PurgeMachines(population, machine_dir):
    print('Purging machines. is this OK? (type OK')
    RLY = input()
    if not 'OK' in RLY:
        return population
    for file in listdir(machine_dir):

        if file.endswith(".mac"):
            remove("%s/%s" % (machine_dir, file))
    remove("%s/machines.list" % machine_dir)

    population = populate([], 1, 0)
    setmachines(population)
    return population


def ReleaseOrphan(machine_dir):
    Fo = open("%s/machines.list" % machine_dir, 'r')
    mLIST = Fo.readlines()
    Fo.close

    for file in listdir(machine_dir):
        if file.endswith('.mac'):
            FOUND = 0
            for guest in mLIST:
                if file in guest:
                    FOUND = 1
            if not FOUND:
                remove("%s/%s" % (machine_dir, file))
                print('Deleted orphan: %s.' % file)

def bareDeleteMachine(machine_dir, machine_name):
    filename = "%s/%s" % (machine_dir, machine_name)
    try:
        remove(filename)
    except FileNotFoundError:
        #raise
        print("Failed to delete %s" % filename)
        return 0
    MachineListLocation = "%s/machines.list" % machine_dir
    MachineList = open(MachineListLocation, 'r').readlines()
    for L in range(len(MachineList)):
        if machine_name in MachineList[L]:
            MachineList[L] = 0
    MachineList = [x for x in MachineList if x]

    WriteToList = open(MachineListLocation, 'w')
    for line in MachineList:
        WriteToList.write(line)
    WriteToList.close()
    
# linux only;
def rebuildMachineList(machine_dir):
    FileList = listdir(machine_dir)
    MachineList = open("%s/machines.list" % machine_dir, 'w')
    for FILE in FileList:
        if FILE.endswith('.mac'):
            MachineList.write('%s\n' % FILE)


    MachineList.close()
