#!/bin/bash
FILES=( "main" "brain_fast" "brain" "board" "operation" "evolution" "interface" "evaluate")
if [ "$1" = "cuda" ]
then

TOOL="/opt/cuda/bin/nvcc -x cu -arch=sm_50 -rdc=true --resource-usage -Xcompiler -std=c++98"
LINKTOOL="/opt/cuda/bin/nvlink -arch=sm_50"
if [ "$2" = "debug" ]
then
TOOL="$TOOL -g -G"
fi
EXTENSION=".cpp"
#for F in ${FILES[@]}; do
#cp $F.cpp $F.cu
#done
else
TOOL="gcc -Ofast -g -DDEBUG"
LINKTOOL="gcc -Ofast"
EXTENSION=".cpp"
fi

#for F in ${FILES[@]}; do
#${TOOL} -c ${F}$EXTENSION
#done
#${LINKTOOL} -o e-vchess main.o board.o operation.o evolution.o interface.o brain.o


${TOOL} -o e-vchess main.cpp board.cpp operation.cpp evolution.cpp interface.cpp brain.cpp brain_fast.cpp evaluate.cpp
