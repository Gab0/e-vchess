#include "lampreia.h"


Device int evaluateMaterial(struct board *evalboard,
			    int BoardMaterialValue[64],  int AttackerDefenderMatrix[2][64], 
			    int P, int Attacker, int Verbose)
{
  float score = 0;
    
  int i=0, j=0, z=0, M=0, Z=0;
    
  int PieceIndex=0, AttackerIndex=0, DefenderIndex=0;

  float PieceMaterialValue=0;

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
  

  int deltaDefender = 0;
  
  float AttackerDefenderBalanceValue = 0;
  float BruteDefenseValue = 0;
  
  if (PieceCount < 5) endgameModeOn = 1;

  int KingSafespaceScore = 0;

  int cSQR = 0;
  forsquares
    {
      cSQR = SQR(i, j);
      //this slows da thinking process by a lot.
      // score += ifsquare_attacked(evalboard->squares, i, j, P, 0, 0) * 5 *
      // 	BRAIN.boardcontrol;
      
      // detect if square is inside king safespace. evaluation will be resumed later;
      /*     if ( abs(ownKingPos[0] - i) < 2 && abs(ownKingPos[1] - j) < 2 )
	{
	  if (getindex(evalboard->squares[ cSQR ], Pieces[P], 6) < 0)
	    {
	      KingSafespaceScore -= 20 * BRAIN.kingPanic;
	      //continue;
	    }
	  
	  isInKingSafespace = 1;
	  }*/
      //      else
      //isInKingSafespace = 0;
    
      // skip empty square for efficiency;
      if (evalboard->squares[ cSQR ] == 'x')
	{
	  BoardMaterialValue[ cSQR ] = 0;
	  continue;
	}
      PieceIndex = getindex(evalboard->squares[ cSQR ], Pieces[P], 6);
      if (PieceIndex < 0)
	continue;
      
      PieceMaterialValue = BRAIN.pvalues[PieceIndex];

      
      // evaluate pawns;
      if (PieceIndex == 0)
	{
	  if (P)
	    {
	      pawnEffectiveHeight = i-1;
	      if (evalboard->squares[ SQR((i-1), j) ] == 'p')
		PieceMaterialValue -= PieceMaterialValue / 10 * BRAIN.pawnIssue;
	      if ( (j>1 && evalboard->squares[ SQR((i-1), (j-1)) ] == 'p') ||
		   (j<7 && evalboard->squares[ SQR((i-1), (j+1)) ] == 'p') )
		PieceMaterialValue += (PieceMaterialValue / 15) * BRAIN.pawnIssue;
	    }
	  else
	    {
	      pawnEffectiveHeight = 6-i;
	      if (evalboard->squares[ SQR(i+1, j) ] == 'P')
		PieceMaterialValue -= PieceMaterialValue / 10 * BRAIN.pawnIssue;
	      if ( (j>1 && evalboard->squares[ SQR((i+1), (j-1)) ] == 'P') ||
		   (j<7 && evalboard->squares[ SQR((i+1), (j+1)) ] == 'P') )
		PieceMaterialValue += (PieceMaterialValue / 15) * BRAIN.pawnIssue;
	    }

	  pawnEffectiveHeight = powf(BRAIN.pawnrankMOD, pawnEffectiveHeight);
	  
	  if (AttackerDefenderMatrix[P][ cSQR ] > AttackerDefenderMatrix[1-P][ cSQR ])
	    pawnEffectiveHeight *= 2 * BRAIN.pawnSafeMarch;
	  
	  PieceMaterialValue += pawnEffectiveHeight;// * BRAIN.pawnrankMOD;
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
	  // evaluate piece placement;
	  PieceMaterialValue += sqrt(PieceMaterialValue) *
	    (BoardMiddleScoreWeight[abs(7*(1-P)-i)] + BoardMiddleScoreWeight[j])
	    * BRAIN.seekmiddle ;
	  PieceMaterialValue += sqrt(PieceMaterialValue) *
	    BoardInvaderScoreWeight[abs(7*(1-P)-i)] * BRAIN.seekInvasion;
	  
	  deltaDefender = AttackerDefenderMatrix[P][cSQR] > AttackerDefenderMatrix[1-P][cSQR];
	  if (deltaDefender > 0)
	    {
	      deltaDefender = min(deltaDefender, BRAIN.limitDefender * 10);
	      
	      deltaDefender = max(deltaDefender, 0);
	      BruteDefenseValue += logf(PieceMaterialValue) * powf(deltaDefender, BRAIN.MODbackup);
	
	    }
	
	
	}
    else
      {

	KingSafespaceScore = EvaluateKingSecurity(evalboard, i, j, P);
      }
      
      score += PieceMaterialValue;
      //      if (!PieceMaterialValue)
      //	printf("WNO PIECE MATERIAL VALUE @ DESIGNATION\n");
      BoardMaterialValue[cSQR] = static_cast<int>(PieceMaterialValue);
    
      

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
  
  
  return static_cast<int>(score);
  
    }
Device int evaluateAttack(struct board *evalboard,
  struct movelist *moves,
  int BoardMaterialValue[64],
  int AttackerDefenderMatrix[2][64],
  int P, int Attacker, int Verbose)
{
#define AO [ moves->attackers[Z][1] ]
#define AT [ moves->defenders[Z][1] ]
  
  
  int DefenderIndex = 0;
  
  float AttackerDefenderBalanceValue = 0;
  float score=0;
  
  int Z=0;
  int deltaAttacker=0;
  int FreePiece=0;

  float multiplier=0;
  F(Z, moves->kad)
    {
      AttackerDefenderBalanceValue = 0;

      //printf("%c .%i...%i.........\n", moves->defenders[Z][0], moves->defenders[Z][1], moves->defenders[Z][2]);
      
      DefenderIndex = getindex(moves->defenders[Z][0], Pieces[1-P], 6);
      if (DefenderIndex == 5||DefenderIndex == -1)
	{
	  continue;
	}
      
      deltaAttacker = AttackerDefenderMatrix[P]AT - AttackerDefenderMatrix[1-P]AT;
      if (deltaAttacker > 0 || BoardMaterialValue AT > BoardMaterialValue AO)
	{
	  //printf("%i\n", max(deltaAttacker,1));
	  AttackerDefenderBalanceValue = logf(BoardMaterialValue AT) * BRAIN.seekatk;
	  
	  if (deltaAttacker > 1)
	    {
	      multiplier = powf(deltaAttacker, BRAIN.parallelAttacker);
	      //multiplier = deltaAttacker * BRAIN.parallelAttacker;
	      AttackerDefenderBalanceValue *= multiplier;
	    }
	  
	  if (P == Attacker)
	    	{
		  if (!AttackerDefenderMatrix[1-P]AT)
		    FreePiece = max(FreePiece, BoardMaterialValue AT);
		  
		  else
		    FreePiece = max(FreePiece, (BoardMaterialValue AT - BoardMaterialValue AO));
		}
	}
      
      else
	AttackerDefenderBalanceValue = 0;
      
      // the following line boosts playing strenght by 100 ELO (?)
      //if (AttackerDefenderMatrix[1-P]AT)
      /*if(!BoardMaterialValue AO)
	{
	  asprintf(&output, "NO PIECE MATERIAL VALUE @ VERIFICATION\n");
	  write(2, output, strlen(output));

	  asprintf(&output, "%c@%i v %i -> %c@%i v %i\n",
		   moves->attackers[Z][0], moves->attackers[Z][1], BoardMaterialValue AO, moves->defenders[Z][0], moves->defenders[Z][1], BoardMaterialValue AT);
	write(2, output, strlen(output));
	
	asprintf(&output, "machineplays = %i, evalplayer = %i\n", machineplays, P);
	write(2, output, strlen(output));
	show_board_matrix(BoardMaterialValue);
	show_board(evalboard->squares);
	show_movelist(moves);
	show_moveline(evalboard, 0, evalboard->MovementCount);
	exit(0);
	}*/
      if (BoardMaterialValue AO - BoardMaterialValue AT > 50)
	AttackerDefenderBalanceValue -= logf(BoardMaterialValue AO - BoardMaterialValue AT) * min(0, -deltaAttacker) * BRAIN.balanceoffense;
      //AttackerDefenderBalanceValue = max(AttackerDefenderBalanceValue, 0);      
      

      score += AttackerDefenderBalanceValue * BRAIN.offensevalue;
      
      if (Verbose && AttackerDefenderBalanceValue > 100)
	printf("loaded attacker/defender caulculus! s=%i;\n", AttackerDefenderBalanceValue);


      if (Verbose)
	{
	//TODO:: PRINT SCORE 8X8 MATRIX.
	  printf("%c -----atks-----> %c %i\n", moves->attackers[Z][0], moves->defenders[Z][0], AttackerDefenderBalanceValue);
	}

      //      if (P != Attacker)	AttackerDefenderBalanceValue *= BRAIN.balanceoffense;


      
    }
  if (Verbose)
    printf("FreePiece bonus = %i\n", FreePiece);
  
  score += FreePiece * BRAIN.freepiecevalue;

  score += logf(moves->k) * BRAIN.MODmobility;

  
  return static_cast<int>(score);

    
}

Device int EvaluateKingSecurity(struct board *evalboard, int i, int j, int P)
{
  int lenSafespace = 8;
  int Safespace[lenSafespace] = {-7, -8, -9, 7, 8, 9, -1, 1};
  int SafespaceSize = lenSafespace;
  
  float SafespaceScore = 0;
  int xrayAttackers = 0;
  
  int cSQR = SQR(i,j);
  int K = 0, SS = 0;
  float KingSecurityScore = 0;

  xrayAttackers = ifsquare_attacked(evalboard->squares, i, j, 1-P, 1, 0);
  KingSecurityScore -= 20 * powf(xrayAttackers, BRAIN.kingPanic);
  
  if (i==0 || i==7)
  if ((j<4 && cSQR < 63 && evalboard->squares[cSQR+1] == Pieces[P][1]) ||
      (j>4 && cSQR > 0 && evalboard->squares[cSQR-1] == Pieces[P][1]))
    KingSecurityScore += 50 * BRAIN.kingAreaTower;

  F (K, lenSafespace)
    {
      SS = cSQR + Safespace[K];
      if (0 < SS && SS < 64)
	{
	  if (evalboard->squares[ SS ] != 'x')
	    {
	      if (is_in(evalboard->squares[ SS ], Pieces[P], 6))
		SafespaceScore += 100 * BRAIN.kingAreaSecure;
	      else
		SafespaceScore -= 200 * BRAIN.kingAreaPanic;
	    }
	}
      else
	SafespaceSize--;
    }
  
  SafespaceScore = (SafespaceScore / SafespaceSize) * 8;
  KingSecurityScore += SafespaceScore;
  
  return KingSecurityScore;
}


Host void GenerateAttackerDefenderMatrix(char squares[64], int AttackerDefenderMatrix[2][64])
{
  int P=0, i=0,j=0;

  F(P,2)
    forsquares
    {
      if (squares[SQR(i, j)] != 'x')
	{
	  AttackerDefenderMatrix[P][SQR(i, j)] = ifsquare_attacked(squares, i, j, P, 0, 0);
	  //printf("%i\n", AttackerDefenderMatrix[P][SQR(i, j)]);

	}
      else
	AttackerDefenderMatrix[P][SQR(i, j)] = 0;
    }
  
}
