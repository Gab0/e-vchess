##About:

 <p>evchess is a simple chess engine, loaded with a bizarre concept of running each time with a different set of parameters, to be used by the position evaluating function. So 
 to have the best strategic thinking and results, those parameters must be in tune with each other. A basic implementation of evolutionary algorythm is employed to get to this state, in the form of two satellite python scripts:</p>
 
 <p> arena_array.py creates some virtual chess tables, each to be played by two instances of evchess loaded with different parameters. The parameter kit to be loaded by evchess is called a 'machine', and stored in the 'machines' folder as text files. 
After some thousands cycles in the arena_array, machines will have their ELO rating stored in themselves, that will indicate their power as a set of parameters.<br>
      machineviewer.py helps organizing the machine set.</p>
      
 <p>The engine is basic, it evaluates all moves, discarding some with alpha-beta pruning. No bitmasks, mailboxes, opening books nor other modern chess engine stuff are employed.<br>
 The simplest machine (standard.mac) has a playing strenght of like 800 ELO in online chess (@deep=4).
 It is set to ignore every aspect of position evaluation besides the value of each piece on the board.
 Each parameter is actually a weight for some aspect of the evaluation function, so while evolution drifts those parameters from zero,
 the engine will account more aspects of the position into consideration, like the number of possible moves in given position, number of attackers...</p>

##Usage:
 You can just play against the engine on Xboard.<br>
   Otherwise, to evolve machines:<br>

 1) Use machineviewer.py to create a pool of 64 machines.<br>
 2) Run chess_arena.py for 30k cycles or more, on a 32-tabled session, then look at the population on the Machineviewer.<br>
 3) Run xboard or other chess interface, loading the engine with --TOP arg.<br>
	`$xboard --fcp 'evchess --TOP --MD <path to machinedir> --deep 4 --xdeep 2'`<br>
 4) Good luck. evchess is still weak on the board.. WIP.<br>

##Commands:
 The main shell script will call useful stuff:<br>
    `$ cd path/to/repo`
    `$ ./evc play //Load xboard with the engine, and choose a machine from the hall of fame.`
    `$ ./evc view //Population manager.`
    `$ ./evc arena //Load some dozens of engines and let them play against each other so they can evolve.`
    
##Dependencies:

chess_arena.py requires <a href="https://github.com/niklasf/python-chess">python-chess</a>.<br>


##CUDA:

 The engine code is filled with strange stuff such as 'Host', 'Device' macro keywords, and strange functions. They are related to alternative evaluation method I tried to implement with
NVIDIA CUDA.<br>
 Turned out that CUDA can't handle traditional chess engine processing with those small cores. Waiting for CUDA 8 or going for neural networks someday.

