#!/bin/python

from evchess_evolve.current_parameters import STDPARAMETERS


params = STDPARAMETERS()


SetToZero = []
InitBrainStruct = []
ParameterReader = []

for P in params:
    shortName = P.name.split('_')[1]
    SetToZero.append("Brain.%s = %.2f;" % (shortName, P.stdvalue))
    InitBrainStruct.append("float %s;" %(shortName))
    ParameterReader.append('readparam(line, V, "%s", &Brain.%s);' % (P.name, shortName))


W = open('engine/include_setToZero', 'w')
W.write('\n'.join(SetToZero))
W = open('engine/include_initBrainsSruct', 'w')
W.write('\n'.join(InitBrainStruct))
W = open('engine/include_parameterReader', 'w')
W.write('\n'.join(ParameterReader))

