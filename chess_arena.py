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


# TKinter interface causes memory leak issues.
# for long runs, disabling the graphical interface is mandatory.
parser = optparse.OptionParser()
parser.add_option('--nogui', action='store_false', dest='Interface', default=True)
parser.add_option('--go', action='store_true', dest='GO')

(options, args) = parser.parse_args()
if not options.Interface: options.GO = True
chdir(path.dirname(path.realpath(__file__)))


arenaArray = Arena(GraphicInterface=options.Interface, GO=options.GO)
