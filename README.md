##About:

 <p> lampreia is a simple chess engine, with the concept of running each time with a different set of parameters to be used by the position evaluating function. So 
 to have the best strategic thinking and results, those parameters must be a cohesive set. A basic implementation of evolutionary algorythm is employed to get to this state, in the form of two satellite python scripts:</p>
 
 <p> arena_array.py creates some virtual chess tables, each to be played by two instances of evchess loaded with different parameters. The parameter kit to be loaded by evchess is called a 'machine', and stored in the 'machines' folder as text files. 
After some thousands cycles in the arena_array, machine files will have their ELO rating stored in themselves, that will indicate their power as a set of parameters.<br>
      machineviewer.py helps organizing the machine set.</p>
      
 <p>The engine is basic, it evaluates all moves, discarding some branches with alpha-beta pruning. Bitmasks, mailboxes, opening books and other modern chess engine stuff are not implemented.<br>
 The simplest machine (standard.mac) has a playing strenght of like 800 ELO in online chess (@deep=4).
 It is set to ignore every aspect of position evaluation besides the value of each piece on the board.
 Each parameter is actually a weight for some aspect of the evaluation function, so while evolution drifts those parameters from zero,
 the engine will account those corresponding aspects of the position into consideration, like the number of possible moves in given position, number of attackers, king safety, etc...</p>

##Usage:
 You can just play against the engine on Xboard.<br>
   Otherwise, to evolve machines:<br>

 1) Use machineviewer.py to create a pool of 64 machines.<br>
 2) Run chess_arena.py for 30k cycles or more, on a 32-tabled session.<br>
 3) Run xboard or other chess interface, loading the engine with --TOP arg.<br>
	`$xboard --fcp 'evchess --TOP --MD <path to machinedir> --deep 4 --xdeep 2'`<br>or<br>
	`$./evc play`<br>
 4) Good luck. Best machines can play as 1250 ELO @ 4 plies.<br>

##Commands:
 The main script is a shortcut to useful stuff:<br>

    $./lampr play //Choose a machine from the hall of fame to load with xboard.
    $./lampr view //Population manager.
    $./lampr arena //Load some dozens of engines and let them play against each other so they can evolve.
    $./lampr tournament //Launch a tournament, where hall of fame machines will play against themselves.

    
##Dependencies:

<a href="https://github.com/niklasf/python-chess">python-chess</a><br>


##CUDA:

 The engine code is filled with strange stuff such as 'Host', 'Device' macro keywords, and strange functions. They are related to alternative evaluation method I tried to implement with
NVIDIA CUDA.<br>
 Turned out that CUDA can't handle traditional chess engine processing with those small cores. Waiting for CUDA 8 and going for neural networks someday.

