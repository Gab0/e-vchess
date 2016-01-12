# e-vchess
Chess engine powered by genetic algorithm.

	E_Vchess is a chess engine made in C aiming to be powerful, fast and lightweight. Unlike any other engine, it is fed with various parameters that are used by it's simple move evaluating functions. 
	In order to do the right plays on each situation, the parameters need to be spot on and tuned to each other. A best possible combination may not exist, there can be many very good combinations, each creating it's own 'playing personality' instead.
	To get to these powerful combinations of parameters, a rather very simple implementation of evolutionary algorythm is employed, in the form of parallel utilities to be used with the engine. The arena_array.py is the most important force, it creates many virtual chess tables, each to be played by two instances of E_Vchess loaded with different parameters. The parameter kit to be loaded by E_Vchess is called a 'machine', and stored in the 'machines' folder as text files. 
After many cycles in the arena_array, machines will have their win/draw/lose ratio stored in themselves, that will indicate their power as a set of parameters. 
	Now the machineviewer.py script can manipulate these machines, cloning the best, deleting the worst, and mutating the rest. Now another arena cycle can begin.


## Dependencies:
pychess

## Instructions:
 Unpack all and build the chess engine. You can play against it using it's default parameters,  by running it alone or using a chess GUI like Xboard:

$xboard -fcp '[full path to engine] --showinfo'

To see some evolutionary magic happen, set the full path to your machines directory on Fdir variable of evchess_evolve.py. It can be empty.
Then run machineviewer.py and populate your pool of machines,
run chess_arena.py, click "show/hide all" on the menu, and set the click on the Xth machine red button to set X simultanous tables to be running.

The algorythms on evchess_evolve.py will do some automated evolution on the population, based on win/draw/losses of each machine.

You can then play against the machines you evolved:

$xboard -fcp '[full path to engine] -MD [full path to machine dir] --showinfo'

Of course you can just run the engine with these parameters to play, but it's a shitty way.


## TODO:
-> E_Vchess engine understands most chess rules, but castling still fails [fixed].

-> Better evolutionary approach on the machines, probably using DEAP module.

 
