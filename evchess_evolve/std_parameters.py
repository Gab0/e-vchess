from evchess_evolve.parameter import parameter


def STDPARAMETERS():
    return [

        #parameter("param_DEEP", 0, 30, 2, bLIM=4, INCR=2, LIM=10),

        parameter("eval_randomness", 0, 30, 20, INCR=10, bLIM=1, LIM=270),

        #parameter("param_seekpieces", 0, 30, 1, bLIM=0, INCR=0.25, LIM=3),

        #parameter("param_deviationcalc", 0, 30, 0.1, INCR=0.2),

        #parameter("param_evalmethod", 0, 30, 0, aP=1, bLIM=0, LIM=0),

        parameter("param_seekatk", 0, 20, 0.4, bLIM=-0.2, INCR=0.2, LIM=0.8),

        parameter("param_seekmiddle", 0, 23, 0, LIM=5),

        #parameter("param_presumeOPPaggro", 0, 20, 0, LIM=0.1, bLIM=-0.1, INCR=0.03),

        parameter("param_pawnrankMOD", 0, 20, 0, LIM=37),

        parameter("param_parallelcheck", 0, 30, 0, LIM=21, bLIM=0),

        parameter("param_balanceoffense", 0, 30,
                  0, LIM=5, bLIM=-0.2, INCR=0.2),

        parameter("param_MODbackup", 0, 40, 0, LIM=3, bLIM=-0.2, INCR=0.2),

        parameter("param_MODmobility", 0, 40, 0, LIM=3, bLIM=-0.2, INCR=0.2),

        #parameter("param_cumulative", 0, 80, 0,LIM=1, bLIM=0, INCR=0.1),

        #parameter("param_pvalues", 0, 5, [100,500,300,300,900,2000], INCR=50, bLIM=70, LIM=2500, locked=1),

        #parameter("param_TIMEweight", 0, 30, [0.9, 0.85, 0.9, 0.85, 0.81, 0.765, 0.825, 0.789, 0.844, 0.85], LIM=1.3, bLIM=0.01, INCR = 0.05)

        parameter("param_moveFocus", 0, 40, 0, LIM=4, bLIM=0, INCR=0.1)
    ]


"""
turned off param_seekpieces to serve as an anchor/solid reference to other parameters.

turned off param_presumeOPPaggro at a point where the engine was getting
    overly agressive. AVG real ELO: 920.
   
fixed mutation process,  mutations were in a position on the evolving method
that they were not being save. also, fixed bugs in engine brain. AVG ELO still
920.

"""
