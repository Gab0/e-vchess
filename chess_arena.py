#!/bin/python

import shlex
from time import *
import sys
from os import path, chdir
import threading
import gc
import optparse

from chessArena.settings import Settings
settings = Settings()
from chessArena.arena import Arena

Interface=True
GO = False
# TKinter interface causes memory leak issues.
# for long runs, disabling the graphical interface is mandatory.
parser = optparse.OptionParser()
parser.add_option('--nogui', action='store_false', dest='Interface')
parser.add_option('--go', action='store_true', dest='GO')

if not Interface: GO = True
chdir(path.dirname(path.realpath(__file__)))


arenaArray = Arena(GraphicInterface=Interface, GO=GO)
