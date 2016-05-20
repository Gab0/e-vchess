#!/bin/python

import shlex
from time import *
import sys

from psutil import *
import threading
import gc


import chessArena.settings
chessArena.settings.initialize()
from chessArena.arena import Arena
# from chessArena.table import Table


if (len(sys.argv) > 0) and ('--nogui' in sys.argv):
    global GUI
    GUI = 0

arenaArray = Arena()
