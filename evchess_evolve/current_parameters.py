from evchess_evolve.parameter import parameter

standardINCR = 0.05

def STDPARAMETERS():
    return [

        parameter("eval_randomness", 0, 10, 20,
                  INCR=10, bLIM=1, LIM=40, promoter='R'),

        #parameter("param_seekpieces", 0, 30, 1, bLIM=0, INCR=0.25, LIM=3),

        #parameter("param_deviationcalc", 0, 30, 0.1, INCR=0.2),

        #parameter("param_evalmethod", 0, 30, 0, aP=1, bLIM=0, LIM=0),

        parameter("param_seekatk", 0, 12, 0.3,
                  bLIM=0.2, INCR=standardINCR, LIM=0.5, promoter='A'),

        parameter("param_seekmiddle", 0, 23, 1,
                  bLIM= 1, LIM=10, promoter='m'),

        #parameter("param_presumeOPPaggro", 0, 20, 0, LIM=0.1, bLIM=-0.1, INCR=0.03),

        parameter("param_pawnrankMOD", 0, 20, 3,
                  LIM=15, bLIM=3, promoter='p'),


        parameter("param_parallelAttacker", 0, 12, 0.3,
                  LIM=1.5, bLIM=0, INCR=standardINCR, promoter='c'),

        parameter("param_balanceoffense", 0, 6, 0.8,
                  LIM=1, bLIM=0.0, INCR=standardINCR, promoter='o'),

        parameter("param_MODbackup", 0, 20, 0.4,
                  LIM=1.6, bLIM=0.4, INCR=standardINCR, promoter='B'),

        parameter("param_MODmobility", 0, 15, 2.0,
                  LIM=2, bLIM=0.2, INCR=standardINCR, promoter='M'),

        #parameter("param_cumulative", 0, 80, 0,LIM=1, bLIM=0, INCR=0.1),

        #parameter("param_pvalues", 0, 5, [100,500,300,300,900,2000], INCR=50, bLIM=70, LIM=2500, locked=1),

        #parameter("param_moveFocus", 0, 12, 0,
        #          LIM=4, bLIM=0, INCR=0.1, promoter='F'),

        #parameter("param_boardcontrol", 0, 12, 0,
        #          LIM=3, bLIM=0, INCR=0.25, promoter='C'),

        #parameter("param_endgameWeight", 0, 12, 0.2,
        #          LIM=3, bLIM=0.0, INCR=0.1, promoter="E"),

        #parameter("param_opponentAddMaterialValue", 0, 15, 0,
        #          LIM=-0.1, bLIM=-0.6, INCR=0.1, promoter="A")

        parameter("param_kingPanic", 0, 25, 0.0,
                  LIM=0.6, bLIM=0, INCR=standardINCR, promoter='P'),
        
        parameter("param_kingAreaPanic", 0, 25, 0.0,
                  LIM=0.6, bLIM=0, INCR=standardINCR, promoter='a'),
        
        parameter("param_kingAreaTower", 0, 25, 0.0,
                  LIM=0.6, bLIM=0, INCR=standardINCR, promoter='t'),
        
        parameter("param_kingAreaSecure", 0, 25, 0.0,
                  LIM=0.6, bLIM=0, INCR=standardINCR, promoter='s'),

        parameter("param_pawnIssue", 0, 15, 0.5,
                  LIM=0.5, bLIM=0, INCR=standardINCR, promoter='W'),
        
        parameter("param_seekinvasion", 0, 15, 0.2,
                  LIM=1, bLIM=0, INCR=standardINCR, promoter='I'),
        parameter("param_freepiecevalue", 0, 6, 0.8,
                  LIM=1, bLIM=0.0, INCR=standardINCR, promoter='F'),
        parameter("param_offensevalue", 0, 6, 0.8,
                  LIM=1, bLIM=0.0, INCR=standardINCR, promoter='O'),
        parameter("param_limitDefender", 0, 6, 0.3,
                  LIM=0.4, bLIM=0.2, INCR=standardINCR, promoter='L'),
        parameter("param_castlebonus", 0, 6, 0.6, 
                  LIM=1.5, bLIM=0.2, INCR=standardINCR, promoter='T')
    ]

def ScoreParameters():
    return [        
            parameter("stat_games", 1, 0, 0),
            parameter("stat_wins", 1, 0, 0),
            parameter("stat_draws", 1, 0, 0),
            parameter("stat_loss", 1, 0, 0),
            parameter("stat_K", 1, 0, 0),
            parameter("real_world_score", 1, 0, 0)
                    ]

"""
turned off param_seekpieces to serve as an anchor/solid reference to other parameters.

turned off param_presumeOPPaggro at a point where the engine was getting
    overly agressive. AVG real ELO: 920.
   
fixed mutation process,  mutations were in a position on the evolving method
that they were not being save. also, fixed bugs in engine brain. AVG ELO still
920.
Engine and evolution mechanics fixed. AVG ELO 1000. Now starting real evolution @ 21/sep

Evaluation mechanics fixed. AVG real ELO 1100. @ 8/nov

EndgameWeight added. Applies to pawn rank and king position. @ 8/nov

Modified Evaluation Mechanism. AVG ELO 990 @ 16/nov

Recreating Evaluation mechanism from scratch. Eval only taking material value into account: AVG ELO 880 @ 25/nov

Evaluation mechanism recreated, now done in two steps. after extensive training (100k rounds and corresponding tournaments) AVG ELO 1125 @ 20/dez

AVG ELO 1205 21/dez

Added six more parameters, AVG ELO 1500 28/02
"""
