#!/bin/python
import sys
from os import path, listdir, chdir
from chessArena.tournament import Tournament
import curses
chdir(path.dirname(path.realpath(__file__)))



if "-t" in sys.argv:
    TournamentType = "tournament"
else:# "-k" in sys.argv:
    TournamentType = "killemall"

TOURNAMENT = Tournament(None, TournamentType, 1)

