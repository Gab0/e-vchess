def initialize():
    global COLOR
    global evchessARGS
    global GUI

    global TABLECOUNT
    global TABLEonROW
    
    COLOR = {0: 'WHITE', 1: 'BLACK'}

    
    # path to e-vchess executable,
    # and the directory where machines are stored, respectively.
    evchessP = "engine/e-vchess"
    machineDIR =  "machines"

    evchessARGS = [evchessP, "-MD", machineDIR, "--deep", "4"]
    GUI = 1
    
    #sets the number of simultaneous chess tables to be created and played.
    TABLECOUNT = 32
    #number of tables to be shown on each row of machines.
    TABLEonROW = 8
