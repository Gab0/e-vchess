##About:

 E-Vchess is a simple chess engine.<br>
 It's move evaluating algorith is fed with various parameters.<br>
 To have the best strategic thinking and results, the parameters must be tuned with each other...<br>
 To get to the viable combinations of parameters, a basic implementation of evolutionary algorythm is employed, in the form of two python scripts.<br>
 Arena_array.py creates some virtual chess tables, each to be played by two instances of E-Vchess loaded with different parameters. The parameter kit to be loaded by E_Vchess is called a 'machine', and stored in the 'machines' folder as text files. 
After many cycles in the arena_array, machines will have their ELO rating stored in themselves, that will indicate their power as a set of parameters.<br>
 Machineviewer.py script can manipulate machine files and the whole population.<br>
 The engine is basic, uses alpha-beta pruning to evaluate the pool of moves. No bitmasks, opening books or other advanced chess engine apparatus.<br>
 The most basic machine (standard.mac) has a playing strenght of like 900 ELO in online chess. It is set to ignore every aspect of position evaluation besides the value of each piece on the board. When the evolution starts to drift most
of the parameters from zero, the engine will also take number of possible moves in given position, number of attackers, and other stuff into consideration.<br>

##Usage:
 You can just play against the engine on Xboard.<br>
   But if you want to evolve machines:<br>

 1) Create a population of 64 individuals using machineviewer.py.<br>
 2) Run chess_arena.py for ten thousand cycles, on a 32-tabled session, then look at the population on the Machineviewer.<br>
 3) Send some machines on the 1200's or 1300's ELO to the top machines.<br>
 3) Run xboard or other chess interface, loading the engine with --TOP arg.<br>
	>>$xboard --fcp 'evchess --TOP --MD <path to machinedir> --deep 6'<br>
 4) If you can't win, try to understand this repo and contribute to it. If you can, run 10k more cycles of chess_arena and try again.<br>
 
##Dependencies:

chess_arena.py requires:<br>
<a href="https://github.com/niklasf/python-chess">python-chess</a><br>
psutil<br>


##TODO:

-> Better evolutionary approach on the machines, probably using DEAP module.<br>
-> Make the engine show score for each move along with the full considered moveline (on Xboard). It shows only the first move.<br>
 
