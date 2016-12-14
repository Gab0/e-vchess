#include "lampreia.h"


Device int evaluateMaterial(struct board *evalboard,
			    int BoardMaterialValue[8][8],  int AttackerDefenderMatrix[2][8][8], 
		    int P, int Attacker, int Verbose)
{
  //int Index = blockIdx.x;
  //printf("E %i\n", Index);
  
  int score = 0;
    
  int i=0, j=0, z=0, M=0, Z=0;
    
  int PieceIndex=0, AttackerIndex=0, DefenderIndex=0, PieceMaterialValue=0;

  int pawnEffectiveHeight = 0;
  int piecePositionalValue = 0;
  int chaos = 1;   

  if (BRAIN.randomness) chaos = rand() % (int)(BRAIN.randomness);

    
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

  int AttackerDefenderBalanceValue = 0;
  int BruteDefenseValue = 0;
  if (PieceCount < 5) endgameModeOn = 1;


  //if ((P && ownKingPos[0]) == 0 || (!P && ownKingPos[0] == 7))
  //  score += 150 * BRAIN.kingPanic;


  int KingSafespaceScore = 0;
  forsquares
    {

    //this slows da thinking process by a lot.
    // score += ifsquare_attacked(evalboard->squares, i, j, P, 0, 0) * 5 *
    // 	BRAIN.boardcontrol;

      // detect if square is inside king safespace. evaluation will be resumed later;
    if ( abs(ownKingPos[0] - i) < 2 && abs(ownKingPos[1] - j) < 2 )
      {
	if (getindex(evalboard->squares[i][j], Pieces[P], 6) < 0)
	  {
	    KingSafespaceScore -= 20 * BRAIN.kingPanic;
	    continue;
	  }
     
	isInKingSafespace = 1;
      }
    else
      isInKingSafespace = 0;
	
    // skip empty square for efficiency;
    if (evalboard->squares[i][j] == 'x'){
      BoardMaterialValue[i][j] = 0;
      continue;
    }
    PieceIndex = getindex(evalboard->squares[i][j], Pieces[P], 6);
    if (PieceIndex < 0) continue;
    
    PieceMaterialValue = BRAIN.pvalues[PieceIndex];



    // evaluate pawns;
      if (PieceIndex == 0)
	{
	  if (isInKingSafespace)
	    KingSafespaceScore += 50 * BRAIN.kingPanic;
	  if (P)
	    {
	    pawnEffectiveHeight = i-1;
	    if (evalboard->squares[i-1][j] == 'p')
	      PieceMaterialValue -= PieceMaterialValue / 10 * BRAIN.pawnIssue;
	    if ( (j>1 && evalboard->squares[i-1][j-1] == 'p') ||
		 (j<7 && evalboard->squares[i-1][j+1] == 'p') )
	      PieceMaterialValue += (PieceMaterialValue / 15) * BRAIN.pawnIssue;
	  }
	else
	  {
	    pawnEffectiveHeight = 6-i;
	    if (evalboard->squares[i+1][j] == 'P')
	      PieceMaterialValue -= PieceMaterialValue / 10 * BRAIN.pawnIssue;
	    if ( (j>1 && evalboard->squares[i+1][j-1] == 'P') ||
		 (j<7 && evalboard->squares[i+1][j+1] == 'P') )
	      PieceMaterialValue += (PieceMaterialValue / 15) * BRAIN.pawnIssue;
	  }
	
	pawnEffectiveHeight = pow(pawnEffectiveHeight, 1.2);
	if (AttackerDefenderMatrix[P][i][j] > AttackerDefenderMatrix[1-P][i][j])
	  pawnEffectiveHeight *= 3;
	  
	PieceMaterialValue += pawnEffectiveHeight * BRAIN.pawnrankMOD;
	if (endgameModeOn)
	  PieceMaterialValue += pawnEffectiveHeight * sqrt(currentMovementCount) * BRAIN.endgameWeight;
      }

      // evaluate movement possibilities;
      /*F(M, moves->k)
	if (moves->movements[M].from[0] == i && moves->movements[M].from[1] == j)
	  PieceMaterialValue += 3 * BRAIN.MODmobility;
      */



      // evaluate safe position of piece;
      if (PieceIndex != 5)
	{
	if (AttackerDefenderMatrix[P][i][j] > AttackerDefenderMatrix[1-P][i][j])
	  BruteDefenseValue += sqrt(PieceMaterialValue) * pow((AttackerDefenderMatrix[P][i][j] - AttackerDefenderMatrix[1-P][i][j]), BRAIN.MODbackup);// * BRAIN.MODbackup;

      // evaluate piece placement;
	PieceMaterialValue += sqrt(PieceMaterialValue) * (BoardMiddleScoreWeight[abs(7*(1-P)-i)] + BoardMiddleScoreWeight[j]) * BRAIN.seekmiddle ;
	PieceMaterialValue += sqrt(PieceMaterialValue) * BoardInvaderScoreWeight[abs(7*(1-P)-i)] * BRAIN.seekInvasion;
	  

	}
    else
      {
	xrayAttackers = ifsquare_attacked(evalboard->squares, i, j, 1-P, 1, 0);
	PieceMaterialValue -= 5 * pow(xrayAttackers, BRAIN.kingPanic);
      }
      
      score += PieceMaterialValue;
	BoardMaterialValue[i][j] = PieceMaterialValue;    
    }
  



  score += BruteDefenseValue;
  score += KingSafespaceScore;
  if (Verbose)
    {
      printf("per square score of player %i is %i.\n\n", P, score);
      printf("King Safespace Score is %i.\n", KingSafespaceScore);
      //      printf("KAD %i\n", moves->kad);
      printf("Brute Defense Value = %i\n", BruteDefenseValue);
    }
  

  return score;
}
Device int evaluateAttack(//struct board *evalboard,
			  struct movelist *moves,
			  int BoardMaterialValue[8][8],
			  int AttackerDefenderMatrix[2][8][8],
			  int P, int Attacker, int Verbose)
{
#define AO0 moves->attackers[Z][1]
#define AO1 moves->attackers[Z][2]
#define AO [moves->attackers[Z][1]][moves->attackers[Z][2]]
#define AT [moves->defenders[Z][1]][moves->defenders[Z][2]]
  
  int DefenderIndex = 0;
  int AttackerDefenderBalanceValue = 0;
  int score=0;
  int Z=0;

  int FreePiece=0;
  F(Z, moves->kad)
    {
     
      //printf("%c .%i...%i.........\n", moves->defenders[Z][0], moves->defenders[Z][1], moves->defenders[Z][2]);
      
      DefenderIndex = getindex(moves->defenders[Z][0], Pieces[1-P], 6);
      if (DefenderIndex == 5||DefenderIndex == -1)
	{
	  continue;
	}

      if (AttackerDefenderMatrix[P]AT > AttackerDefenderMatrix[1-P]AT || BoardMaterialValue AT > BoardMaterialValue AO)
	{
	  AttackerDefenderBalanceValue = max( sqrt(BoardMaterialValue AT)
					      * pow(AttackerDefenderMatrix[P]AT - AttackerDefenderMatrix[1-P]AT, 0.9) * BRAIN.seekatk, 0);
	  if (P == Attacker)
	    	{
		  if (!AttackerDefenderMatrix[1-P]AT)
		    {
		      if (FreePiece < BoardMaterialValue AT * BRAIN.balanceoffense)
			FreePiece = BoardMaterialValue AT * BRAIN.balanceoffense;
		    }
		  else if
		       (FreePiece < (BoardMaterialValue AT - BoardMaterialValue AO) * BRAIN.balanceoffense)
		    FreePiece = (BoardMaterialValue AT - BoardMaterialValue AO) * BRAIN.balanceoffense;
		  
		}

	//AttackerDefenderBalanceValue -= sqrt(BoardMaterialValue AO)/2 * AttackerDefenderMatrix[1-P]AT * BRAIN.balanceoffense;
	}
      else

	AttackerDefenderBalanceValue = 0;
      if (Verbose && AttackerDefenderBalanceValue > 100)
	printf("loaded attacker/defender caulculus! s=%i;\n", AttackerDefenderBalanceValue);


      if (Verbose)
	{
	//TODO:: PRINT SCORE 8X8 MATRIX.
	  printf("%c -----atks-----> %c %i\n", moves->attackers[Z][0], moves->defenders[Z][0], AttackerDefenderBalanceValue);
	}

      //if (P != Attacker)
	//AttackerDefenderBalanceValue *= BRAIN.balanceoffense;

      score += AttackerDefenderBalanceValue;
      
    }
  if (Verbose)
    printf("FreePiece bonus = %i\n", FreePiece);
  score += FreePiece;
  return score;
    
}


Host void GenerateAttackerDefenderMatrix(char squares[8][8], int AttackerDefenderMatrix[2][8][8])
{
  int P=0, i=0,j=0;

  F(P,2)
    forsquares
    {
      if (squares[i][j] != 'x')
	{
	  AttackerDefenderMatrix[P][i][j] = ifsquare_attacked(squares, i, j, P, 0, 0);
	  //printf("%i\n", AttackerDefenderMatrix[P][i][j]);

	}
      else
	AttackerDefenderMatrix[P][i][j] = 0;
    }
  
}
