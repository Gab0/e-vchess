#!/bin/python

import shlex

from time import *



#from os import *

import sys

from evchess_evolve.core import *
from evchess_evolve.management import *
from evchess_evolve.advanced import * 
from random import randrange


from psutil import *
import threading
import gc






import chessArena.settings
chessArena.settings.initialize()
from chessArena.arena import Arena
# from chessArena.table import Table


if (len(sys.argv) > 0) and ('--nogui' in sys.argv):
    GUI = 0

arenaArray = Arena()
