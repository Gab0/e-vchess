#include "lampreia.h"


Device int evaluate(struct board *evalboard, struct movelist *moves,
		    int defenderMatrix[2][8][8], int P, int Attacker, int Verbose)
{
  //int Index = blockIdx.x;
  //printf("E %i\n", Index);
  
  int score = 0;
    
  int i=0, j=0, z=0;
    
  int PieceIndex=0, AttackerIndex=0, DefenderIndex=0, Z=0, PieceMaterialValue=0;

  int pawnEffectiveHeight = 0;
  int piecePositionalValue = 0;
  int chaos = 1;   

  if (BRAIN.randomness) chaos = rand() % (int)(BRAIN.randomness);


  attackers_defenders(moves);

    
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


  //if ((P && ownKingPos[0]) == 0 || (!P && ownKingPos[0] == 7))
  //  score += 150 * BRAIN.kingPanic;
  
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



    // evaluate pawns;
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
	
	pawnEffectiveHeight = pow(pawnEffectiveHeight, 1.2);
	PieceMaterialValue += pawnEffectiveHeight * BRAIN.pawnrankMOD;
	if (endgameModeOn)
	  PieceMaterialValue += pawnEffectiveHeight * sqrt(currentMovementCount) * BRAIN.endgameWeight;
      }










    
    score += PieceMaterialValue;
    if (PieceIndex != 5)
	     score += sqrt(PieceMaterialValue) * pow(defenderMatrix[P][i][j], 0.9) * BRAIN.MODbackup;

    else
      {
	xrayAttackers = ifsquare_attacked(evalboard->squares, i, j, 1-P, 1, 0);
	score -= 100 * xrayAttackers * BRAIN.kingPanic;




      }
    
    }


  if (Verbose) printf("per square score of player %i is %i.\n\n", P, score);
  F(Z, moves->kad)
    {
      //      printf("%c .%i...%i.........\n", moves->defenders[Z][0], moves->defenders[Z][1], moves->defenders[Z][2]);
      DefenderIndex = getindex(moves->defenders[Z][0], Pieces[1-P], 6);
      if (DefenderIndex == 5||DefenderIndex == -1)
	{

	  continue;
	}
      score += sqrt(BRAIN.pvalues[DefenderIndex]) * BRAIN.seekatk;
      score -= sqrt(BRAIN.pvalues[getindex(moves->attackers[Z][0], Pieces[P], 6)])/10 *
	defenderMatrix[1-P][moves->defenders[Z][1]][moves->defenders[Z][2]] *
	BRAIN.balanceoffense;

	}
    
  if (Verbose) printf("adding attackers/defenders [kad=%i], score of player %i is %i.\n\n",
		      moves->kad, P, score);
      /*
    if (P != Machineplays)
      PieceMaterialValue *= ( 1 + BRAIN.opponentAddMaterialValue );
 


    
    piecePositionalValue = (BoardMiddleScoreWeight[i] + BoardMiddleScoreWeight[j]);
    	//old method: ((-power(j,2)+7*j-5) + (-power(i,2)+7*i-5)) 

    if (PieceIndex == 5 && endgameModeOn)
      {
      PieceMaterialValue += 100  *
	piecePositionalValue * BRAIN.seekmiddle *
	currentMovementCount * BRAIN.endgameWeight;
      }
    else
      {
	if (PieceIndex == 5)
	  PieceMaterialValue -= 1000 *
	    piecePositionalValue * BRAIN.seekmiddle;

	else
	  PieceMaterialValue += sqrt(BRAIN.pvalues[PieceIndex])/8  *
	    piecePositionalValue * BRAIN.seekmiddle;
      }
    
    score += PieceMaterialValue * BRAIN.seekpieces;
    
    parallelAttackers = ifsquare_attacked(evalboard->squares, i, j, 1-P, 0, 0);
    parallelDefenders = ifsquare_attacked(evalboard->squares, i, j, P, 0, 0);
    
    deltaAttackersDefenders = parallelAttackers - parallelDefenders;
      
      //KING'S xray attackers
      if (PieceIndex == 5)
	{
	xrayAttackers = ifsquare_attacked(evalboard->squares, i, j, 1-P, 1, 0) - parallelAttackers;
	//if (P == Attacker) xrayAttackers -= 1;
	if (xrayAttackers > 0)
	  score -= 7 * xrayAttackers * BRAIN.kingPanic;
	//score = score;

      }
      
      else
	if (isInKingSafespace)
	  {
	  if (deltaAttackersDefenders > 0)
	    score -= 7 * BRAIN.kingPanic * pow(deltaAttackersDefenders, 1.2);

	  score += sqrt(PieceMaterialValue) /2 * BRAIN.kingPanic;
	  }
      if (PieceIndex == 5)
	{
	  if (parallelAttackers) score -= 100;
	  continue;
	}




      
      /* if (parallelAttackers > 0)
	{

	  //AttackedScoreLoss = (sqrt(deltaAttackersDefenders)-0.3) * sqrt(PieceMaterialValue) * BRAIN.seekatk;



	  AttackedScoreLoss = parallelAttackers * PieceMaterialValue * BRAIN.seekatk;

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
	  score += parallelDefenders * sqrt(PieceMaterialValue)/2 * BRAIN.MODbackup;
	}
  
    }


        if (P == Attacker)
	{
	  F(Z,moves->kad)
	    {
	      DefenderIndex = getindex(moves->defenders[Z][0], Pieces[1-Attacker], 6);
	      score += sqrt(BRAIN.pvalues[DefenderIndex]) * BRAIN.seekatk;
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
  // score += chaos;       
  //score += moves->k * BRAIN.MODmobility;



  
  
  return score;
    
}


Host void GenerateDefenderMatrix(char squares[8][8], int DefenderMatrix[2][8][8])
{
  int P=0, i=0,j=0;

  F(P,2)
    forsquares
    {
      if (is_in(squares[i][j], Pieces[P], 6))
	{
	  DefenderMatrix[P][i][j] = ifsquare_attacked(squares, i, j, P, 0, 0);

	  //if (DefenderMatrix[P][i][j] > 3)
	  //  {
	  //    printf("invalid defender matrix [%i].\n", DefenderMatrix[P][i][j]);
	      //exit(0);
	  //  }
	}
    }
  
}
