def initialize():
    global COLOR
    global engineARGS
    global GUI

    global VerboseMove

    global TABLECOUNT
    global TABLEonROW

    global machineDIR
    global TOPmachineDIR

    global enginebin

    global arena_showmemuse
    COLOR = {0: 'WHITE', 1: 'BLACK'}

    # path to e-vchess executable.
    enginebin = "engine/e-vchess"

    # paths to the directories where machines are stored.
    machineDIR = "machines"
    TOPmachineDIR = machineDIR + "/top_machines"  # % machineDIR

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
    # allow chessarena to plot memory usage on graphic.
    arena_showmemuse = 1
