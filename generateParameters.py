#!/bin/python

from evchess_evolve.current_parameters import STDPARAMETERS
from os import path, chdir

params = STDPARAMETERS()
chdir(path.dirname(path.realpath(__file__)))
Files = ['engine/include_setToZero',
         'engine/include_initBrainStruct',
         'engine/include_parameterReader']

SetToZero = []
InitBrainStruct = []
ParameterReader = []

for P in params:
    shortName = P.name.split('_')[1]
    Value = P.stdvalue if P.Enabled else 0
    SetToZero.append("Brain.%s = %.2f;" % (shortName, Value))
    InitBrainStruct.append("float %s;" %(shortName))
    if P.Enabled:
        ParameterReader.append('readparam(line, V, "%s", &Brain.%s);' %\
                               (P.name, shortName))

for F in Files:
    z=open(F, 'a').close()
    

W = open(Files[0], 'w')
W.write('\n'.join(SetToZero))
W = open(Files[1], 'w')
W.write('\n'.join(InitBrainStruct))
W = open(Files[2], 'w')
W.write('\n'.join(ParameterReader))

