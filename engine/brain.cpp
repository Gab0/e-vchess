#include "ev_chess.h"


#define DUMP(B) if(B!=NULL){free(B);B=NULL;}

int think (struct move *out, int PL, int DEEP, int verbose) {
    
  int i=0; int ChosenMovementIndex=0;
  int FivemoveRepetitionRisk=0;
  long score = -INFINITE;

  time_t startT = time(NULL);
  
  searchNODEcount = 0;
  
  struct board *_board = makeparallelboard(&board);
  _board->MovementCount=0;
  int CurrentMovementIndex = _board->MovementCount;

  
  long Alpha = -INFINITE;
  long Beta = INFINITE;
    
  struct movelist *moves =
    (struct movelist *) calloc(1, sizeof(struct movelist));
    
  int PLAYER = Machineplays;
  legal_moves(_board, moves, PLAYER, 0);
  
  if (board.MovementCount == 0) { //primitive opening book:
    int moveBook[4] = {9, //e2e4
		       7, //d2d4
		       19, //g1f3
		       5,}; //c2c4
      
       srand ( rndseed() );
       ChosenMovementIndex = moveBook[rand() % 4];


      replicate_move(out, &moves->movements[ChosenMovementIndex]);
      
    DUMP(moves);
    DUMP(_board);
    return ChosenMovementIndex;
		       
  }
  
  reorder_movelist(moves);

  if (check_fivemove_repetition()) FivemoveRepetitionRisk = 1;
  if (moves->k == 0) {
    DUMP(_board);
    DUMP(moves);
    return -1;
  }
    
  struct board **finalboardsArray =
    (struct board**) calloc(moves->k, 8);
    
  struct board *dummyboard;
  struct board *BufferBoard;

  int AllowCutoff = 1;
  if(BRAIN.xDEEP) AllowCutoff = 1;

 
  // cuda move evaluating.        
#ifdef __CUDACC__        
#include "brain_cuda0.cpp"

  // non cuda move evaluating mehthod.        
#else           
  if(canNullMove(DEEP, _board, moves->k, PLAYER)) {
    flip(_board->whoplays);
    BufferBoard = thinkiterate(_board, DEEP-1, verbose, -Beta, -Alpha, AllowCutoff);
    flip(_board->whoplays);
    invert(BufferBoard->score);
    if (BufferBoard->score > Alpha) Alpha = BufferBoard->score;

    DUMP(BufferBoard)
      }
    

  for (i=0;i<moves->k;i++) {

    move_pc(_board, &moves->movements[i]);    
    BufferBoard = thinkiterate(_board, DEEP-1, verbose,
			       -Beta, -Alpha, AllowCutoff);      
    invert(BufferBoard->score);
    moves->movements[i].score = BufferBoard->score;
   
    finalboardsArray[i] = BufferBoard;

    BufferBoard = NULL;
    
    if (Show_Info) show_moveline(finalboardsArray[i], CurrentMovementIndex, startT);
    //if (moves->movements[i].score > Alpha) Alpha = moves->movements[i].score;
      
    if (FivemoveRepetitionRisk)
      if (compare_movements(&moves->movements[i], &movehistory[hindex-4])){
	asprintf(&output, "Downgrading score due to move repetition draw menace.\n");
	write(1, output, strlen(output));
	  moves->movements[i].score = 0;
      }
    
    if (moves->movements[i].score > score) {
      score = moves->movements[i].score;
      ChosenMovementIndex=i;
    }
    //printf("%i %i\n", PLAYER, finalboardsArray[i]->whoplays);
    if (BRAIN.xDEEP)
      undo_lastMove(finalboardsArray[i], 2);

    //finalboardsArray[i]->score = -finalboardsArray[i]->score;
    undo_move(_board, &moves->movements[i]);
  }     
             
#endif
  
  //second-level deepness move search schematics;##################################

  if (BRAIN.xDEEP) {
    int MaximumT = (int) BRAIN.yDEEP;

    int T = moves->k;
    if (T > MaximumT) T = MaximumT;


    int secondTOP[T], I=0, M=0;

    long sessionSCORE=-9990000;
    long movelistSCORE=-9990000;
    
    long satellite = 0;
    int OtherPlayer=0;
    int PLAYER=0;
    selectBestMoves(moves->movements, moves->k, secondTOP, T);

    struct movelist *nextlevelMovelist =
      (struct movelist*) calloc(T, sizeof(struct movelist));

    int R=0;
    
    for (R=0;R<BRAIN.xDEEP;R++) {
      AllowCutoff = 1;
      if (R + 1 == BRAIN.xDEEP) AllowCutoff = 1;

	sessionSCORE = -INFINITE;
	
	Alpha = -INFINITE;
	Beta = INFINITE;      

      for (i=0;i<T;i++) {


	I = secondTOP[i];
	OtherPlayer = 0;
	Vb fprintf(stderr, "----------------------------------------------------------\n");
	Vb fprintf(stderr,
		   "Nm: %i || original R: %i || current R: %i\n",
		   finalboardsArray[I]->MovementCount, I,
		   ChosenMovementIndex);
	Vb fprintf(stderr, "Wp:%i  || I:  %i\n", finalboardsArray[I]->whoplays, I);

	PLAYER = finalboardsArray[I]->whoplays;
	movelistSCORE = -INFINITE;

	legal_moves(finalboardsArray[I], &nextlevelMovelist[i], PLAYER, 0);
	reorder_movelist(&nextlevelMovelist[i]);


	if (nextlevelMovelist[i].k) {

	  for (M=0;M<nextlevelMovelist[i].k;M++) {

	    move_pc(finalboardsArray[I], &nextlevelMovelist[i].movements[M]);


	    if (PLAYER == Machineplays)	{
	    dummyboard = thinkiterate(finalboardsArray[I], DEEP-1,
			   0, -Beta, -Alpha, AllowCutoff);
	    invert(dummyboard->score);
	  }
	    else dummyboard = thinkiterate(finalboardsArray[I], DEEP-1, 0, Alpha, Beta, AllowCutoff);


	    nextlevelMovelist[i].movements[M].score = dummyboard->score;
	    
	    undo_move(finalboardsArray[I], &nextlevelMovelist[i].movements[M]);


	    if (nextlevelMovelist[i].movements[M].score > movelistSCORE
		||BufferBoard==NULL) {
	      
	      BufferBoard = dummyboard;
	      dummyboard = NULL;
	      movelistSCORE = nextlevelMovelist[i].movements[M].score;
	      moves->movements[I].score = movelistSCORE;
   
	    }
	    else
	      DUMP(dummyboard);
	  }
       
	  if (BufferBoard==NULL){printf("BufferBoard not found failure!\n");exit(0);}
	  DUMP(finalboardsArray[I]);

	  finalboardsArray[I] = BufferBoard;
	  //finalboardsArray[I]->score = movelistSCORE;
	  BufferBoard = NULL;
	}
	else {
	  movelistSCORE = moves->movements[I].score;
	  //	  if(PLAYER == Machineplays)
	  // invert(movelistSCORE);

	}

	if (movelistSCORE > sessionSCORE) {
	  satellite = satellite_evaluation(&moves->movements[I]);
	  if (movelistSCORE + satellite > sessionSCORE) {
	   
	    ChosenMovementIndex = I;
	    sessionSCORE = movelistSCORE + satellite;
	    printf("changed the mind. %ld\n", satellite);
	    
	  }
    printf("%i %i\n", PLAYER, finalboardsArray[i]->whoplays);

	}
	//printf("verbosing: sessionscore: %ld; movelistscore: %ld\n", sessionSCORE, movelistSCORE);
	
	Vb fprintf(stderr, "FNM: %i || FWP: %i\n", finalboardsArray[I]->MovementCount, finalboardsArray[I]->whoplays);
	
	if (Show_Info) show_moveline(finalboardsArray[I], CurrentMovementIndex, startT);
        if(R+1<BRAIN.xDEEP)
	  undo_lastMove(finalboardsArray[I], 2);
      }

    }

    DUMP(nextlevelMovelist);
    //closing bracket for 'if brain.xDEEP';
  }


  replicate_move(out, &moves->movements[ChosenMovementIndex]);

  
  //printf("Dump Section:\n");
  for (i=0;i<moves->k-1;i++)
    DUMP(finalboardsArray[i]);
  //printf("Dumped each finalboard array.\n");
  DUMP(moves);
  //printf("Dumped moves.\n");
  DUMP(_board);
  //printf("Dumped the _board.\n");
  DUMP(BufferBoard);
  //printf("Dumped BufferBoard.\n");
  DUMP(finalboardsArray);



  return ChosenMovementIndex;
   
}

//CUDA kernel functions.
//#####################################################################
#ifdef __CUDACC__
#include "brain_cuda1.cpp"
#endif
//#####################################################################


Device struct board *thinkiterate(struct board *feed, int DEEP, int verbose,
				  long Alpha, long Beta, int AllowCutoff) {

  int i=0, r=0, PersistentBufferOnline=0;
    
  int enemy_score=0, machine_score=0;

  struct board *_board = makeparallelboard(feed);

  struct board *DisposableBuffer;


  int ABcutoff = 0;

  //if (PL != feed->whoplays) printf("INCONSISTENT PLayer=%i; Deep=%i\n", PL, DEEP);
  //struct move result;
    
    
  long score = -INFINITE;
  int HoldBuffer=0;
  struct movelist moves;

  int PLAYER =  _board->whoplays;
    
  legal_moves(_board, &moves, PLAYER, 0);
  searchNODEcount++;
    
  /*IFnotGPU(
    Vb printf("DEEP = %i; K=%i Address: %p\n", DEEP,moves.k ,(void *)_board); 
    Vb printf("NullMove!\n"); 
    )*/
  //If in checkmate/stalemate situation;
  if (!moves.k) {
             
    if (ifsquare_attacked(_board->squares,
			  findking(_board->squares, 'Y', PLAYER), 
			  findking(_board->squares, 'X', PLAYER),
			  1-PLAYER, 0)) {
      score = -13000 + 50*(BRAIN.DEEP-DEEP); 
    }
       
    else score = 0; 
       
    _board->score = score; 
    return _board;
         
  }
    


  //IFnotGPU( Vb show_board(_board->squares); )
  if (DEEP>0) {
    struct board *PersistentBuffer;

    reorder_movelist(&moves); 
    
    //NULL MOVE: guaranteed as long as if PL is not in check,
    //and its not K+P endgame.
    if(DEEP > BRAIN.DEEP - 2) 
      if(canNullMove(DEEP, _board, moves.k, PLAYER)) {
	flip(_board->whoplays);
	DisposableBuffer = thinkiterate(_board, DEEP-1, verbose,
					-Beta, -Alpha, AllowCutoff);
	invert(DisposableBuffer->score);
	flip(_board->whoplays);
	if (DisposableBuffer->score > Alpha)
	  Alpha = DisposableBuffer->score;

	DUMP(DisposableBuffer);
      }
     

    score = -INFINITE;


    // Movelist iteration.

    
    for(i=0;i<moves.k;i++) {
       
      move_pc(_board, &moves.movements[i]);  

      DisposableBuffer = thinkiterate(_board, DEEP-1, verbose, -Beta, -Alpha, AllowCutoff);

      invert(DisposableBuffer->score);
      moves.movements[i].score = DisposableBuffer->score;

      
      if (moves.movements[i].score > score) {
	if (PersistentBufferOnline)
	  DUMP(PersistentBuffer);
	PersistentBuffer = DisposableBuffer;
	score = moves.movements[i].score;
	DisposableBuffer=NULL;
	PersistentBufferOnline = 1;
	
      }
      if (moves.movements[i].score > Alpha) 
	Alpha = moves.movements[i].score;
	
      if (Beta<=Alpha) 
	if(AllowCutoff)
	  if (PersistentBufferOnline) {
	    DUMP(DisposableBuffer);  
	    break;
	  }
	  
    

      undo_move(_board, &moves.movements[i]);
       
      DUMP(DisposableBuffer);  
       
    }

    //score = moves.movements[r].score;

    DUMP(_board);
    //PersistentBuffer->score=score
    if (PersistentBuffer == NULL) exit(0);
    return PersistentBuffer;
    
  }
     
  else {

    machine_score = evaluate(_board, &moves, PLAYER, PLAYER);
    enemy_score = evaluate(_board, &moves, 1-PLAYER, PLAYER);
    //show_board(_board->squares);

    _board->score = machine_score - (enemy_score * (1 + BRAIN.presumeOPPaggro));
    return _board;
  }

}

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
    
    
  forsquares {
    //this slows da thinking process by a lot.
    //score += ifsquare_attacked(evalboard->squares, i, j, P, 0) * 5 *
    //	BRAIN.boardcontrol;

      
    if (evalboard->squares[i][j] == 'x') continue;
      
    PieceIndex = getindex(evalboard->squares[i][j], Pieces[P], 6);
    if (PieceIndex < 0) continue;
    PieceMaterialValue = BRAIN.pvalues[PieceIndex];
        
    if (PieceIndex==0) {
      if (P) pawnEffectiveHeight = i;
      else pawnEffectiveHeight = 7-i;
      PieceMaterialValue += pawnEffectiveHeight * BRAIN.pawnrankMOD;
      PieceMaterialValue += pawnEffectiveHeight * board.MovementCount * BRAIN.endgameWeight;
      

    }

    score += PieceMaterialValue * BRAIN.seekpieces;

    piecePositionalValue = (BoardMiddleScoreWeight[i] + BoardMiddleScoreWeight[j]);
    	//old method: ((-power(j,2)+7*j-5) + (-power(i,2)+7*i-5)) 
    if (PieceIndex != 5)
      score += sqrt(BRAIN.pvalues[PieceIndex])  *
	piecePositionalValue * BRAIN.seekmiddle;
    else
      score += sqrt(BRAIN.pvalues[5])  *
	piecePositionalValue * BRAIN.seekmiddle *
	board.MovementCount * BRAIN.endgameWeight;

  }
  
 
  for (Z=0;Z<moves->kad;Z++) {
    DefenderIndex = getindex(moves->defenders[Z][0], Pieces[1-P], 6); 
    if (P == Attacker)  {
      AttackerIndex =  getindex(moves->attackers[Z][0], Pieces[P], 6);
        
      /*      parallelatks = ifsquare_attacked
	(evalboard->squares,moves->defenders[Z][1],
	 moves->defenders[Z][2], 1-P, 0);
                
      if (BRAIN.parallelcheck) {
	if (parallelatks>1) 
	  score += (parallelatks * 10 * BRAIN.parallelcheck);
	  }*/
      if (DefenderIndex != 5) {	
	score += sqrt(BRAIN.pvalues[DefenderIndex]) * BRAIN.seekatk;
	score -=  sqrt(BRAIN.pvalues[AttackerIndex]) *2 * BRAIN.balanceoffense;
      }
    }
    else if (DefenderIndex != 5){
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

Host Device float scoremod (int DEEP, int method) {
    
  float modifier = 0;
  float helper = 0;
    
  if (method>2) method = 0;
    
    
  if (method == 0) modifier = 1;
    
  if (method == 1) modifier = 2*((DEEP+BRAIN.deviationcalc)/BRAIN.DEEP);
    
  if (method == 2) {
    modifier = -power(DEEP,2)+BRAIN.DEEP*DEEP+2*BRAIN.DEEP;
        

    helper = BRAIN.DEEP/2;
        
    helper = -power(helper,2)+BRAIN.DEEP*helper+2*BRAIN.DEEP;
            
    modifier = modifier/(helper/1.1);
  }
    
        
  return modifier;
}


Device int canNullMove (int DEEP, struct board *board, int K, int P) {
  int i=0,j=0,NullMove=0;
  if (DEEP>BRAIN.DEEP-2&&K>5) 
    forsquares {
      if (board->squares[i][j]!='x'&& 
	  board->squares[i][j]!=Pieces[P][0]&&
	  !is_in(board->squares[i][j],Pieces[1-P],6)) NullMove = 1;
      if (board->squares[i][j]==Pieces[P][5])
	if (ifsquare_attacked (board->squares, i, j, 1-P, 0)) return 0;
    }
  //OFF!!!
  //return NullMove;       
  return 0;             
}


Device int satellite_evaluation (struct move *movement) {
  int Result = 0;
  if (!Brain.moveFocus) return 0;

  if (movement->lostcastle && !movement->iscastle)
    Result -= 10 * Brain.moveFocus;

  if (movement->iscastle) Result += 50 * Brain.moveFocus;

  return Result;
}

Device int check_fivemove_repetition (void) {
  
  int k=0,v=0;
  F(k,3){
    v = -k * 4;
    if (hindex > k * 4 + 4){
      if (!compare_movements(&movehistory[hindex-k], &movehistory[hindex-k-4]))
	return 0;
    }
    else return 0;
  }
  return 1;

}
Device int compare_movements (struct move *move_A, struct move *move_B) {
  int i=0;
  
  F(i,2) {
    if (move_A->from[i] != move_B->from[i]) return 0;
    if (move_A->to[i] != move_B->to[i]) return 0;
    }
  return 1;
}
