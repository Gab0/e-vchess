def initialize():
    global COLOR
    global engineARGS
    global GUI

    global VerboseMove

    global TABLECOUNT
    global TABLEonROW
    
    COLOR = {0: 'WHITE', 1: 'BLACK'}

    
    # path to e-vchess executable.    
    enginebin = "engine/e-vchess"
    
    # path to the directory where machines are stored.
    machineDIR =  "machines"

    # path and arguments to run those chess engines.
    engineARGS = [enginebin, "-MD", machineDIR, "--deep", "4", "--xdeep", "0"]

    # if each movement done should be logged on stdout.
    VerboseMove = 0

    # if arenaArray should load it's graphical interface.
    GUI = 1
    
    # sets the number of simultaneous chess tables to be created and played.
    TABLECOUNT = 32
    # number of tables to be shown on each row of machines.
    TABLEonROW = 8
