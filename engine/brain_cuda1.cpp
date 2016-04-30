Global void kerneliterate(struct board *workingboard,
			  struct movelist *mainmoves,
			  int PL, int DEEP, int *Test, long *_Beta) {

    long Alpha = -16900;
    long Beta = 16900;    
    int Index = blockIdx.x;

    printf("kernel online D%i.\n", DEEP);
    struct board *_board = makeparallelboard(workingboard);

    //Test[Index] = PL + DEEP;//power(Index, 2);
    //mainmove->movements[Index].score =

      //canNullMove(15, workingboard, 19, PL);


    //legal_moves(_board, mainmove, PL, 0);


    //append_move(_board, mainmove,1,3,1,-1, 0);
    //check_move_check(_board, &mainmove->movements[0], PL);
      // _board->passantJ;

    
      // 
    //Testdevice(&Test[Index]);
    move_pc(_board, &mainmoves->movements[Index]);
    

    if (DEEP > 0) {

    struct movelist *nextmoves = (struct movelist *)malloc (sizeof(struct movelist));

    legal_moves(_board, nextmoves, 1-PL, 0);
    
    //cudaDeviceSynchronize();

    kerneliterate <<<nextmoves->k,1>>>(_board, nextmoves, 1-PL, DEEP -1, Test, _Beta);

    int TOP[2];
    select_top(nextmoves->movements, nextmoves->k, TOP, 1);
    mainmoves->movements[Index].score = nextmoves->movements[TOP[0]].score;
      //reorder_movelist(mainmove);
      //findking(_board->squares, 'X', PL);
      //thinkiterate(_board, PL, DEEP, 0, Alpha, Beta);
      //evaluate(_board, mainmove, PL);
      //ifsquare_attacked(_board->squares, 3, 3, PL, 0);
      
    printf("K %i\n",Index);
	//cudaDeviceSynchronize();        
    //printf("x%ld\n", mainmove->movements[Index].score);

    free(nextmoves);
    }

    
    else {
      evalkernel <<<1,1>>> (&mainmoves->movements[Index].score, _board, mainmoves);
      //mainmoves->movements[Index].score = playerscore - enemyscore;

    }
    
    free(_board);
}

Global void evalkernel(long *VALUE, struct board *board, struct movelist *moves) {

  int playerscore = evaluate(board, moves, Machineplays);
  int enemyscore = evaluate(board, moves, Machineplays);

  *VALUE = playerscore - enemyscore;
}

Global void Testkernel(int *Test) {

  int Index = blockIdx.x;
  Test[Index] = 3 * Index;

  int *P= (int*)malloc(sizeof(int));
  printf("CAPETA %p\n", P);
}

Device void Testdevice(int *Test) {
  int x = blockIdx.x;
    *Test = x * 3;
    }
