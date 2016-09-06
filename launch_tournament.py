#!/bin/python
import sys
from os import path, listdir, chdir
from chessArena.tournament import Tournament
import curses

chdir(path.dirname(path.realpath(__file__)))

if len(sys.argv) == 1:
    
    #x = curses.wrapper(Tournament, 1, 1)
    TOURNAMENT = Tournament(None, 1, 1)

else:
    while True:
        FILES = listdir('machines/top_machines')
        MACHINES = [m for m in FILES if m.endswith('.mac')]
        if len(MACHINES) < 5:
            break
        
        TOURNAMENT = Tournament(None, 1, 1)
            
    
