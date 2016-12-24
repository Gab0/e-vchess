#include "lampreia.h"



int think_fast(struct move *out, int PL, int DEEP, int verbose) {
    
  int i=0; int ChosenMovementIndex=0;
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

  if (moves->k == 0) {
    DUMP(_board);
    DUMP(moves);
    return -1;
  }

  int AllowCutoff = 1;
 
  
  /* if(canNullMove(DEEP, _board, moves->k, PLAYER)) {
    flip(_board->whoplays);
    BufferBoard = thinkiterate(_board, DEEP-1, verbose, -Beta, -Alpha, AllowCutoff);
    flip(_board->whoplays);
    invert(BufferBoard->score);
    if (BufferBoard->score > Alpha) Alpha = BufferBoard->score;

    DUMP(BufferBoard)
    }*/
    

  for (i=0;i<moves->k;i++) {

    move_piece(_board, &moves->movements[i], 1);    
    moves->movements[i].score = -thinkiterate_fast(_board, DEEP-1, verbose,
			       -Beta, -Alpha, AllowCutoff);      



    if (moves->movements[i].score > score) {
      score = moves->movements[i].score;
      ChosenMovementIndex=i;
    }
    if (moves->movements[i].score > Alpha)
      Alpha = moves->movements[i].score;

    move_piece(_board, &moves->movements[i], -1);
  }     
             
  replicate_move(out, &moves->movements[ChosenMovementIndex]);
  
  return ChosenMovementIndex;
   
}




Device long thinkiterate_fast(struct board *_board, int DEEP, int verbose,
				  long Alpha, long Beta, int AllowCutoff) {

  int i=0, r=0, PersistentBufferOnline=0;
    
  int enemy_score=0, machine_score=0;

  int ABcutoff = 0;


    
  long score=-INFINITE;

  struct movelist moves;

  int PLAYER =  _board->whoplays;
    
  legal_moves(_board, &moves, PLAYER, 0);
  searchNODEcount++;
    
  //If in checkmate/stalemate situation;
  if (!moves.k) {
             
    if (ifsquare_attacked(_board->squares,
			  findking(_board->squares, 'Y', PLAYER), 
			  findking(_board->squares, 'X', PLAYER),
			  1-PLAYER, 0, 0)) {
      score = -13000 + 50*(BRAIN.DEEP-DEEP);
      //if (PLAYER != Machineplays) flip(score);
    }
       
    else score = 0; 

    return score;
         
  }
    


  //IFnotGPU( Vb show_board(_board->squares); )
  if (DEEP>0) {

    reorder_movelist(&moves); 
    
    //NULL MOVE: guaranteed as long as if PL is not in check,
    //and its not K+P endgame.
    //if(DEEP > BRAIN.DEEP - 2) 
      /*      if(canNullMove(DEEP, _board, moves.k, PLAYER)) {
	flip(_board->whoplays);
	DisposableBuffer = thinkiterate(_board, DEEP-1, verbose,
					-Beta, -Alpha, AllowCutoff);
	invert(DisposableBuffer->score);
	flip(_board->whoplays);
	if (DisposableBuffer->score > Alpha)
	  Alpha = DisposableBuffer->score;

	DUMP(DisposableBuffer);
	}*/
     


    // Movelist iteration.
    for(i=0;i<moves.k;i++) {
       
      move_piece(_board, &moves.movements[i],1);  

      moves.movements[i].score = -thinkiterate_fast(_board, DEEP-1, verbose,
						    -Beta, -Alpha, AllowCutoff);

      
	if (moves.movements[i].score > score) 
	  score = moves.movements[i].score;

	if (moves.movements[i].score > Alpha) 
	  Alpha = moves.movements[i].score;
      
	move_piece(_board, &moves.movements[i], -1);
      
	if (Beta<=Alpha && AllowCutoff) 
	  break;       
       
    }

    return score;
    
  }
     
  else {/*
    int AttackerDefenderMatrix[2][8][8];

    machine_score = evaluate(_board, &moves, AttackerDefenderMatrix,PLAYER, PLAYER, 0);
    enemy_score = evaluate(_board, &moves, AttackerDefenderMatrix, 1-PLAYER, PLAYER, 0) * ( 1+BRAIN.presumeOPPaggro);

    score = machine_score - enemy_score;

	*/
    return 1;
  }

}

