#!/bin/python

import shlex
from time import *
import sys
from os import path, chdir
import threading
import gc

from chessArena.settings import Settings
settings = Settings()
from chessArena.arena import Arena


# TKinter interface causes memory leak issues.
# for long runs, disabling the graphical interface is mandatory.
Interface = not '--nogui' in sys.argv
GO = '--go' in sys.argv or not Interface


chdir(path.dirname(path.realpath(__file__)))


arenaArray = Arena(GraphicInterface=Interface, GO=GO)
