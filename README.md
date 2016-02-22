##About:

 E_Vchess is a chess engine made in C aiming to be powerful, fast and lightweight. Unlike any other engine, 
it is fed with various parameters that act on it's simple move evaluating algorithm.<br>
 In order to do the right plays on each situation, the parameters need to be spot on and tuned to each other. A best possible combination may not exist, there can be many very good combinations, each creating it's own 'playing personality' instead.
 To get to these powerful combinations of parameters, an rather ATM crude implementation of evolutionary algorythm is employed, in the form of parallel utilities to be used with the engine. The arena_array.py is the most important force, it creates many virtual chess tables, each to be played by two instances of E_Vchess loaded with different parameters. The parameter kit to be loaded by E_Vchess is called a 'machine', and stored in the 'machines' folder as text files. 
After many cycles in the arena_array, machines will have their win/draw/lose ratio stored in themselves, that will indicate their power as a set of parameters. 
 Now the machineviewer.py script can manipulate these machines, cloning the best, deleting the worst, and mutating the rest. Now another arena cycle can begin.

##Usage:

 1) Create a population of 256 individuals using machineviewer.py.<br>
 2) Run chess_arena.py for ten thousand cycles, on a 64-tabled session, then look at the population on the Machineviewer.<br>
 3) Send some machines on the 1200's or 1300's ELO to the top machines.<br>
 3) Run xboard or other chess interface, loading the engine with --TOP arg.<br>
 4) If you can't win, try to understand this repo and contribute to it. If you can, run 30k more cycles of chess_arena and try again.<br>
 
##Dependencies:

chess_arena.py requires the following python modules:<br>
<a href="https://github.com/niklasf/python-chess">python-chess</a><br>
psutil<br>


##TODO:

-> Engine may castle without having rights (rarely happens). Need fix.<br>
-> Better evolutionary approach on the machines, probably using DEAP module.<br>
-> Engine can't show score for full evaluated moveline, showing only it's first move.<br>
 
