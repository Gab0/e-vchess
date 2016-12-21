#!/bin/python

import hashlib
import sys.argv

files = ['engine/brain.cpp', 'engine/evaluate.cpp', 'engine/lampreia.h']

O = b''
for w in files:
    O += open(w).read().encode('utf-8')
O = hashlib.md5(O).hexdigest()
if "tail" in sys.argv:
    print(O[-3:])

else:
    print(O)
