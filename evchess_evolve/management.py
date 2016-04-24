def PurgeMachines(population):
    print('Purging machines. is this OK? (type OK')
    RLY = input()
    if not 'OK' in RLY: return population
    for file in os.listdir(Fdir):

        if file.endswith(".mac"): os.remove("%s/%s" % (Fdir,file))
    os.remove(Fdir+'/machines.list')

    population = populate([],1, 0)
    setmachines(population)
    return population

def ReleaseOrphan():
    Fo = open("%s/machines.list" % Fdir, 'r')
    mLIST = Fo.readlines()
    Fo.close

    
    for file in os.listdir(Fdir):
        if file.endswith('.mac'):
            FOUND = 0
            for guest in mLIST:
                if file in guest: FOUND = 1
            if not FOUND:
                os.remove("%s/%s" % (Fdir, file))
                print('Deleted orphan: %s.' % file)


