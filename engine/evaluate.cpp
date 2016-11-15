#include "ev_chess.h"


Device int evaluate(struct board *evalboard, struct movelist *moves, int P, int Attacker) {
  //int Index = blockIdx.x;
  //printf("E %i\n", Index);
  
  int score = 0;
    
  int i=0, j=0;
    
  int PieceIndex=0, AttackerIndex=0, DefenderIndex=0, Z=0, PieceMaterialValue=0;

  int pawnEffectiveHeight = 0;
  int piecePositionalValue = 0;
  int chaos = 1;   

  if (BRAIN.randomness) chaos = rand() % (int)(BRAIN.randomness);

  attackers_defenders(evalboard->squares, *moves, P);

    
  //int deadpiece = 0;
  int parallelatks = 0;
  int paralleldefenders = 0;
    
  int currentMovementCount= board.MovementCount;

  int PieceCount = countPieces(evalboard->squares, 0);
  forsquares {
    //this slows da thinking process by a lot.
    //score += ifsquare_attacked(evalboard->squares, i, j, P, 0) * 5 *
    //	BRAIN.boardcontrol;

      
    if (evalboard->squares[i][j] == 'x') continue;
      
    PieceIndex = getindex(evalboard->squares[i][j], Pieces[P], 6);
    if (PieceIndex < 0) continue;
    PieceMaterialValue = BRAIN.pvalues[PieceIndex];

    if (P != Machineplays)
      PieceMaterialValue *= 1 + BRAIN.opponentAddMaterialValue;
    
    if (PieceIndex==0) {
      if (P) pawnEffectiveHeight = i-1;
      else pawnEffectiveHeight = 6-i;
      PieceMaterialValue += pawnEffectiveHeight * BRAIN.pawnrankMOD;
      if (PieceCount < 5)
	PieceMaterialValue += pawnEffectiveHeight * currentMovementCount * BRAIN.endgameWeight;
      

    }

    score += PieceMaterialValue * BRAIN.seekpieces;
    
    piecePositionalValue = (BoardMiddleScoreWeight[i] + BoardMiddleScoreWeight[j]);
    	//old method: ((-power(j,2)+7*j-5) + (-power(i,2)+7*i-5)) 
    if (PieceIndex != 5)
      score += sqrt(BRAIN.pvalues[PieceIndex])  *
	piecePositionalValue * BRAIN.seekmiddle;
    else
      score += sqrt(BRAIN.pvalues[5])/5  *
	piecePositionalValue * BRAIN.seekmiddle *
	currentMovementCount * BRAIN.endgameWeight;

  }
  
 
  for (Z=0;Z<moves->kad;Z++) {
    DefenderIndex = getindex(moves->defenders[Z][0], Pieces[1-P], 6);
    if (DefenderIndex == 5) continue;
    if (P == Attacker)  {
      AttackerIndex =  getindex(moves->attackers[Z][0], Pieces[P], 6);
        
       parallelatks = ifsquare_attacked
	(evalboard->squares,moves->defenders[Z][1],
	 moves->defenders[Z][2], 1-P, 0);
                
      if (BRAIN.parallelAttacks) {
	if (parallelatks>1) 
	  score += (parallelatks * 10 * BRAIN.parallelAttacks);
	  }
     
	score += sqrt(BRAIN.pvalues[DefenderIndex]) * BRAIN.seekatk;
	score -= sqrt(BRAIN.pvalues[AttackerIndex]) * BRAIN.balanceoffense;
      
    }
    else {
      paralleldefenders = ifsquare_attacked
        (evalboard->squares,moves->defenders[Z][1],
	 moves->defenders[Z][2], P, 0);
      score += (paralleldefenders * sqrt(BRAIN.pvalues[PieceIndex])/2 * BRAIN.MODbackup);
        
    }       
         
  }
  
  score += chaos;       
  score += moves->k * BRAIN.MODmobility;
    
  return score;
    
}
