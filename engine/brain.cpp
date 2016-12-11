#include "lampreia.h"


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
  


  if (check_fivemove_repetition()) FivemoveRepetitionRisk = 1;
  
  if (moves->k == 0) {
    DUMP(_board);
    DUMP(moves);

    return -1;
    
  }
  
  reorder_movelist(moves);
  
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
  
  if (canNullMove(DEEP, _board, moves->k, PLAYER)) {
    flip(_board->whoplays);
    BufferBoard = thinkiterate(_board, DEEP-1, verbose, -Beta, -Alpha, AllowCutoff);
    flip(_board->whoplays);
    invert(BufferBoard->score);
    if (BufferBoard->score > Alpha) Alpha = BufferBoard->score;

    DUMP(BufferBoard)
      }
    

  for (i=0;i<moves->k;i++) {

    move_pc(_board, &moves->movements[i]);    
    finalboardsArray[i] = thinkiterate(_board, Brain.DEEP-1, verbose,
			       -Beta, -Alpha, AllowCutoff);      
    invert(finalboardsArray[i]->score);
    //moves->movements[i].score = BufferBoard->score;
   
    //finalboardsArray[i] = BufferBoard;

    //BufferBoard = NULL;
    
    if (Show_Info) show_moveline(finalboardsArray[i], CurrentMovementIndex, startT);
    //if (moves->movements[i].score > Alpha) Alpha = moves->movements[i].score;
      
    if (FivemoveRepetitionRisk)
      if (compare_movements(&moves->movements[i], &movehistory[hindex-4]))
	{
	  asprintf(&output, "Neutralizing score due to move repetition draw menace.\n");
	  write(1, output, strlen(output));
	  moves->movements[i].score = 0;
	}
    
    //if (finalboardsArray[i]->score > score) {
    //  score = moves->movements[i].score;
    //  ChosenMovementIndex=i;
    //}
    //printf("%i %i\n", PLAYER, finalboardsArray[i]->whoplays);


     
    //finalboardsArray[i]->score = -finalboardsArray[i]->score;
    undo_move(_board, &moves->movements[i]);
  }

  int T = 5;
  T = min(T, moves->k);
  int BEST[5];


  selectBestMoves(finalboardsArray, moves->k, BEST, T);


  if (BRAIN.xDEEP)
    {
      
      int MINIMUM_ITER = 5;
	int MAXIMUM_ITER = 16;
	int maxdepthGone = 0;
	
      int Z = 0;
      int ZI = 0;
      int Thinking = 1;
      while ( Thinking )
      {
	F(Z, T)
	  {
	    ZI = BEST[Z];

	    if (finalboardsArray[ZI]->gameEnd)
	      {
		continue;
	      }
	    undo_lastMove(finalboardsArray[BEST[Z]], 2);
	    BufferBoard = finalboardsArray[BEST[Z]];



      
	    if (finalboardsArray[BEST[Z]]->whoplays == PL)
	      {
		finalboardsArray[BEST[Z]] = thinkiterate(BufferBoard, Brain.DEEP-1, verbose,
						     Alpha, Beta, AllowCutoff);
		//	printf("same player\n");

		//invert(finalboardsArray[BEST[Z]]->score);

	      }
	      else
	      {
		finalboardsArray[BEST[Z]] = thinkiterate(BufferBoard, Brain.DEEP-1, verbose,
						     -Beta, -Alpha, AllowCutoff);
		//finalboardsArray[BEST[Z]] = thinkiterate(BufferBoard, Brain.DEEP-1, verbose,
		//					 Alpha, Beta, AllowCutoff);
		//	printf("other player\n");
		invert(finalboardsArray[BEST[Z]]->score);
	      }


	    
	    
	    if (Show_Info) show_moveline(finalboardsArray[ZI], CurrentMovementIndex, startT);
	    maxdepthGone = max(finalboardsArray[BEST[Z]]->MovementCount, maxdepthGone);
	    
	    if ( abs(finalboardsArray[ZI]->score - BufferBoard->score) > 700)
	      if (finalboardsArray[ZI]->MovementCount == maxdepthGone)
		MAXIMUM_ITER++;
	    
	    DUMP(BufferBoard);
	  }
	
	selectBestMoves(finalboardsArray, moves->k, BEST, T);
	MINIMUM_ITER--;
	MAXIMUM_ITER--;
	//	printf("bestline = %i\n", BEST[0]);

	if (MINIMUM_ITER > 0)
	  continue;

	if(MAXIMUM_ITER > 0)
	  {
	    if (finalboardsArray[BEST[0]]->MovementCount < maxdepthGone)
	      if (!finalboardsArray[BEST[0]]->gameEnd)
		continue;
	    
	    if (finalboardsArray[BEST[1]]->MovementCount < maxdepthGone)
	      if (!finalboardsArray[BEST[1]]->gameEnd)
		continue;
	  }	

	Thinking = 0;	
      }
    }
#endif



  replicate_move(out, &moves->movements[BEST[0]]);

  
   //printf("Dump Section:\n");
  for (i=0;i<moves->k;i++)
    DUMP(finalboardsArray[i]);
  //printf("Dumped each finalboard array.\n");
  DUMP(moves);
  //printf("Dumped moves.\n");
  DUMP(_board);
  //printf("Dumped the _board.\n");
  DUMP(BufferBoard);
  //printf("Dumped BufferBoard.\n");
  DUMP(finalboardsArray);


  

  return BEST[0];
   
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
    
  int enemy_score=0, player_score=0;

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
			  1-PLAYER, 0, 0)) {
      score = -13000 + 50*(BRAIN.DEEP-DEEP); 
    }
       
    else score = 0; 
       
    _board->score = score;
    _board->gameEnd = 1;
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
      moves.movements[i].score = DisposableBuffer->score;// + moves.k * 10;

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

    DUMP(_board);

    
    //PersistentBuffer->score=score
    if (PersistentBuffer == NULL) exit(0);
    return PersistentBuffer;
    
  }
     
  else {
    int AttackerDefenderMatrix[2][8][8];
    int BoardMaterialValue[8][8];
    
    GenerateAttackerDefenderMatrix(_board->squares, AttackerDefenderMatrix);
    
    player_score = evaluateMaterial(_board,
				    BoardMaterialValue, AttackerDefenderMatrix,
				    PLAYER, PLAYER, 0);

    enemy_score = evaluateMaterial(_board,
				   BoardMaterialValue, AttackerDefenderMatrix,
				   1-PLAYER, PLAYER, 0);

    
    player_score += evaluateAttack(&moves,
				     BoardMaterialValue, AttackerDefenderMatrix,
				     PLAYER, PLAYER, 0);
    
    legal_moves(_board, &moves, 1-PLAYER, 0);
    

    enemy_score += evaluateAttack(&moves,
				    BoardMaterialValue, AttackerDefenderMatrix,
				  1-PLAYER, PLAYER, 0);
    //show_board(_board->squares);

    _board->score = player_score - enemy_score;
    return _board;
  }

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
	if (ifsquare_attacked (board->squares, i, j, 1-P, 0, 0)) return 0;
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
