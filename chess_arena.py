#!/bin/python

import shlex
from time import *
import sys
from os import path, chdir
import threading
import gc


import chessArena.settings
chessArena.settings.initialize()
from chessArena.arena import Arena
# from chessArena.table import Table


if (len(sys.argv) > 0) and ('--nogui' in sys.argv):
    global GUI
    GUI = 0
    
chdir(path.dirname(path.realpath(__file__)))
arenaArray = Arena()
