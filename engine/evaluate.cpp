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

  //attackers_defenders(evalboard->squares, *moves, Attacker);

    
  int parallelAttackers = 0;
  int parallelDefenders = 0;
  int deltaAttackersDefenders = 0;
  
  int currentMovementCount= board.MovementCount;
  int endgameModeOn = 0;
  int PieceCount = countPieces(evalboard->squares, 0);

  int DefenderNegatedAttackBuffer = 0;
  int AttackedScoreLoss = 0;
  int ownKingPos[2] = { findking(evalboard->squares, 'Y', P),
		     findking(evalboard->squares, 'X', P) };
  int xrayAttackers = 0;
  int isInKingSafespace = 0;
  if (PieceCount < 5) endgameModeOn = 1;


  if ((P && ownKingPos[0]) == 0 || (!P && ownKingPos[0] == 7))
    score += 150 * BRAIN.kingPanic;
  
  forsquares
    {

    //this slows da thinking process by a lot.
    // score += ifsquare_attacked(evalboard->squares, i, j, P, 0, 0) * 5 *
    // 	BRAIN.boardcontrol;

      // detect if square is inside king safespace. evaluation will be resumed later;
    if ( abs(ownKingPos[0] - i) < 2 && abs(ownKingPos[1] - j) < 2 )
      isInKingSafespace = 1;
    else
      isInKingSafespace = 0;
    
    // skip empty square for efficiency;
    if (evalboard->squares[i][j] == 'x') continue;
    
    PieceIndex = getindex(evalboard->squares[i][j], Pieces[P], 6);
    if (PieceIndex < 0) continue;
    PieceMaterialValue = BRAIN.pvalues[PieceIndex];


    // pawn feature calculations;
    if (PieceIndex == 0)
      {
	if (P)
	  {
	    pawnEffectiveHeight = i-1;
	    if (evalboard->squares[i-1][j] == 'p')
	      PieceMaterialValue -= PieceMaterialValue/5 * BRAIN.pawnIssue;
	    if ( (j>1 && evalboard->squares[i-1][j-1] == 'p') || (j<7 && evalboard->squares[i-1][j+1] == 'p') )
	      PieceMaterialValue += PieceMaterialValue/5 * BRAIN.pawnIssue;
	  }
	else
	  {
	    pawnEffectiveHeight = 6-i;
	    if (evalboard->squares[i+1][j] == 'P')
	      PieceMaterialValue -= PieceMaterialValue/5 * BRAIN.pawnIssue;
	    if ( (j>1 && evalboard->squares[i+1][j-1] == 'P') || (j<7 && evalboard->squares[i+1][j+1] == 'P') )
	      PieceMaterialValue += PieceMaterialValue/5 * BRAIN.pawnIssue;
	  }
	
	pawnEffectiveHeight = pow(pawnEffectiveHeight, 2);
	PieceMaterialValue += pawnEffectiveHeight * BRAIN.pawnrankMOD;
	if (endgameModeOn)
	  PieceMaterialValue += pawnEffectiveHeight * sqrt(currentMovementCount) * BRAIN.endgameWeight;
      }
    
    if (P != Machineplays)
      PieceMaterialValue *= ( 1 + BRAIN.opponentAddMaterialValue );
 


    
    piecePositionalValue = (BoardMiddleScoreWeight[i] + BoardMiddleScoreWeight[j]);
    	//old method: ((-power(j,2)+7*j-5) + (-power(i,2)+7*i-5)) 

    if (PieceIndex == 5 && endgameModeOn)
      PieceMaterialValue += sqrt(BRAIN.pvalues[5])/5  *
	piecePositionalValue * BRAIN.seekmiddle *
	currentMovementCount * BRAIN.endgameWeight;
    else if (PieceIndex == 5)
      PieceMaterialValue -= sqrt(BRAIN.pvalues[5])/5 *
	piecePositionalValue * BRAIN.seekmiddle;

    else
      PieceMaterialValue += sqrt(BRAIN.pvalues[PieceIndex])/4  *
	piecePositionalValue * BRAIN.seekmiddle;

    score += PieceMaterialValue * BRAIN.seekpieces;
    
    parallelAttackers = ifsquare_attacked(evalboard->squares, i, j, 1-P, 0, 0);
    parallelDefenders = ifsquare_attacked(evalboard->squares, i, j, P, 0, 0);
    
    deltaAttackersDefenders = parallelAttackers - parallelDefenders;
      
      //KING'S xray attackers
      if (PieceIndex == 5) {
	xrayAttackers = ifsquare_attacked(evalboard->squares, i, j, 1-P, 1, 0) - parallelAttackers;
	//if (P == Attacker) xrayAttackers -= 1;
	if (xrayAttackers > 0)
	  score -= 7 * xrayAttackers * BRAIN.kingPanic;
	//score = score;

      }
      
      else
	if (isInKingSafespace)
	  if (deltaAttackersDefenders > 0)
	    score -= 7 * BRAIN.kingPanic * pow(deltaAttackersDefenders, 2);
      
      if (PieceIndex == 5) continue;
      if (parallelAttackers > 0)
	{

	  //AttackedScoreLoss = (sqrt(deltaAttackersDefenders)-0.3) * sqrt(PieceMaterialValue) * BRAIN.seekatk;
	  AttackedScoreLoss = parallelAttackers * sqrt(PieceMaterialValue) * BRAIN.seekatk;

	  score -= AttackedScoreLoss;
	  
	  if (AttackedScoreLoss > DefenderNegatedAttackBuffer)
	    DefenderNegatedAttackBuffer = AttackedScoreLoss;
	  //if (parallelAttackers > parallelDefenders && Attacker == P)
	  //	score += sqrt(BRAIN.pvalues[PieceIndex])
	}
      
      if (parallelDefenders > 0)
	{
	  //invert(deltaAttackersDefenders);
	  //score += (sqrt(deltaAttackersDefenders)-0.3) * sqrt(PieceMaterialValue) * BRAIN.seekatk;
	  score += parallelDefenders * sqrt(PieceMaterialValue) * BRAIN.MODbackup;
	}
  }
  
  //if (P == Attacker) score += DefenderNegatedAttackBuffer;

  
  /* for (Z=0;Z<moves->kad;Z++) {
    DefenderIndex = getindex(moves->defenders[Z][0], Pieces[1-Attacker], 6);
    if (DefenderIndex == 5) continue;

    AttackerIndex = getindex(moves->attackers[Z][0], Pieces[Attacker], 6);
    
    parallelAttackers = ifsquare_attacked(evalboard->squares,
					  moves->defenders[Z][1],
					  moves->defenders[Z][2],
					  Attacker,
					  0);
    
    parallelDefenders = ifsquare_attacked(evalboard->squares,
					  moves->defenders[Z][1],
					  moves->defenders[Z][2],
					  1 - Attacker,
					  0);
    
     if (P == Attacker)  {
       if (parallelAttackers > parallelDefenders)
       score += sqrt(BRAIN.pvalues[DefenderIndex])/2  * BRAIN.seekatk;

	 /* if (BRAIN.parallelAttacks) {
	if (parallelAttackers > 1) 
	  score += (parallelAttackers * 10 * BRAIN.parallelAttacks);
	  }*/
      
       /* parallelDefenders = ifsquare_attacked(evalboard->squares,moves->defenders[Z][1], moves->defenders[Z][2], 1-Attacker, 0);
      if (parallelDefenders < parallelAttackers){ 
	score += sqrt(BRAIN.pvalues[DefenderIndex]) * BRAIN.seekatk;
	//	  score -= sqrt(BRAIN.pvalues[AttackerIndex]) * BRAIN.balanceoffense;
	}

	 
	
    }
    else {
      if (parallelDefenders > parallelAttackers) score += sqrt(BRAIN.pvalues[DefenderIndex]) / 10 * BRAIN.MODbackup;

      //score += (sqrt(BRAIN.pvalues[DefenderIndex]) * BRAIN.MODbackup);
	//score += sqrt(BRAIN.pvalues[AttackerIndex])/4 * BRAIN.MODbac;
      i=0;
             
    }
}*/
  score += chaos;       
  score += moves->k * BRAIN.MODmobility;
    
  return score;
    
}
