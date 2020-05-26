## About:
    
 <p> lampreia is a simple chess engine, with the concept of loading a set of parameters to be used by the position evaluating function. So 
 to have the best strategic thinking and results, those parameters must be a cohesive set. A basic implementation of evolutionary algorythm is employed to get to this state, in the form of two satellite python scripts:</p>
 
 <p> arena_array.py creates some virtual chess tables to be played by instances of lampreia, running different parameter sets. Those sets are called a 'machine', stored in a folder as txt files.
After some hours of arena_array, machine files will have their ELO rating stored in themselves, indicating their power as a set of parameters.<br>
      machineviewer.py has tools to manage machines.</p>
      
 <p>The engine evaluates all moves, discarding some branches with alpha-beta pruning. No bitmasks, mailboxes, or opening books.
 The engine with stock parameters, mostly zero,  has a playing strenght of like 800 ELO in online chess @deep=4.
In that mode it will compute only the value of each piece on the board.
 Each parameter is actually a weight for some aspect of the evaluation function, so while evolution drifts those parameters from zero,
 the engine will account those corresponding aspects of the position into consideration: number of possible moves in given position, number of attackers, king safety, and others.</p>

## Usage:

 1) $./lampr populate 64   // creates a pool of 64 machines.<br>
 2) $./lampr arena        // runs the arena. Let it roll for 30k cycles or more, for the resultz.<br>
 3) $./lampr play        // To play against the engine. This is equivalent to:<br>
	`$xboard --fcp 'evchess --TOP --MD <path to machinedir> --deep 4 --xdeep 2'`<br>or<br>
	`$./evc play`<br>


## Commands:
 The main script is a shortcut to useful stuff:<br>

    $./lampr play //Choose a machine from the hall of fame to load with xboard.
    $./lampr view //Population manager.
    $./lampr arena //Load some dozens of engines and let them play against each other so they can evolve.
    $./lampr tournament //Launch a tournament, where hall of fame machines will play against themselves.
    $./lampr position //Launch the position evaluator, another method to evolve machines.

    
## Dependencies:

<a href="https://github.com/niklasf/python-chess">python-chess</a><br>
