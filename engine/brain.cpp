#include "ev_chess.h"


#define DUMP(B) if(B!=NULL){free(B);B=NULL;}
       

int think (struct move *out, int PL, int DEEP, int verbose) {
    
    int i=0; int r=0;
    long score = 0;
    time_t startT = time(NULL);

    struct board *_board = makeparallelboard(&board);  
    Vb printf("Master Address: %p\n", (void *)_board); 
    
    long Alpha = -1690000;
    long Beta = 1690000;
    
    struct movelist *moves = (struct movelist *) calloc(1, sizeof(struct movelist));

            
    legal_moves(_board, moves, PL, 0); 
    
    reorder_movelist(moves);
    
    
    Vb printf("thinking r:%i  k:%i DEEP:%i.\n",r,moves->k,DEEP);
    if (moves->k == 0) return -1;
    Vb printf("value of k is %i.\n",moves->k);
    
    //infoMOVE = (char *)malloc(sizeof(char)*128);
    
//#define __CUDACC__    
// cuda move evaluating.        
#ifdef __CUDACC__        
    

    struct board *GPUboard;
    struct movelist *GPUmoves;
    
    long *_Alpha;
    long *_Beta;

    
    cudaMalloc((void**) &_Alpha, sizeof(long));
    cudaMalloc((void**) &_Beta, sizeof(long));
    
    cudaMemcpy(_Alpha, &Alpha, sizeof(long), cudaMemcpyHostToDevice);
    cudaMemcpy(_Beta, &Beta, sizeof(long), cudaMemcpyHostToDevice);
    
    cudaMalloc((void**) &GPUmoves, sizeof(struct movelist));
    cudaMemcpy(GPUmoves, moves, sizeof(struct movelist), cudaMemcpyHostToDevice);
    
    
    
    //array of boards GPU allocation.
    /*cudaMalloc((void**) &GPUboard, sizeof(struct board*) * moves->k);
    for (i=0;i<moves->k;i++) {
        cudaMalloc((void**) &GPUboard[i], sizeof(struct board));
        cudaMemcpy(GPUboard[i], _board, sizeof(struct board), cudaMemcpyHostToDevice); 
        printf("I=%p\n", GPUboard[i]);  
	}*/

    cudaMalloc((void**) &GPUboard, sizeof(struct board));
    cudaMemcpy(GPUboard, _board, sizeof(struct board), cudaMemcpyHostToDevice);

    
    int *CTest = (int*)calloc(moves->k, sizeof(int));
    int *Test;
    cudaMalloc((void**) &Test, moves->k * sizeof(int));
    
    cudaMemcpy(Test, CTest, sizeof(int) * moves->k, cudaMemcpyHostToDevice);
    dim3 threadsPerBlock(1, 1); 
    dim3 numBlocks(moves->k, 1);  
    kerneliterate <<<numBlocks, threadsPerBlock>>>(GPUboard, GPUmoves, PL, DEEP, Test, _Beta);


    

    
    //Testkernel <<<moves->k, 1>>> (Test);
    //cudaDeviceSynchronize();
    //cudaFree(GPUboard);
    
    //kernel error debug;
    cudaError_t err = cudaGetLastError();
    if (err != cudaSuccess) 
    printf("Error: %s\n", cudaGetErrorString(err));

    //cudaMemcpy(CTest, Test, moves->k * sizeof(int), cudaMemcpyDeviceToHost);
    
    
    int R[2];

    printf("GPUMOVES = %p\n", GPUmoves);
    cudaMemcpy(moves, GPUmoves, sizeof(struct movelist), cudaMemcpyDeviceToHost);
    
    
    
    
    select_top(moves->movements, moves->k, R, 1);
    r = R[0];
    
    
    for (i=0;i<moves->k;i++){
      printf("score for i=%i is %i\n", i , moves->movements[i].score);
      printf("TEST %i = %i\n", i, CTest[i]);
    }
    printf("but the GPU chose move n=%i\n", r);
    
    cudaFree(GPUmoves);
    cudaFree(GPUboard);
    
// non cuda move evaluating mehthod.        
#else           
    if(canNullMove(DEEP, _board, moves->k, PL)) {
               score = thinkiterate(_board, 1-PL, DEEP-1, verbose, 
               Alpha, Beta);
               if (PL==Machineplays) Alpha = score;
               else Beta = score;
               
           }     
      for (i=0;i<moves->k;i++) {
        
        //show_board(board.squares);
     Vb printf("new tree branching. i=%i\n",i);
     Vb print_movement(&moves->movements[i],0);
     printf("Alpha = %i\n", Alpha);
     
     move_pc(_board, &moves->movements[i]);    
     moves->movements[i].score = thinkiterate(_board, 1-PL, DEEP-1, 1,
             Alpha, Beta);
     
     if (moves->movements[i].score > Alpha) {
         Alpha = moves->movements[i].score;
         r=i;

     }
     
     //show_board(_board->squares);
     undo_move(_board, &moves->movements[i]); 

     
     Vb printf("analyzed i=%i; score is %ld.\n",i, moves->movements[i].score);
     IFnotGPU( Vb if (show_info) printf(">>>>>>>>\n"); )
     IFnotGPU( if (show_info) eval_info_move(&moves->movements[i],DEEP, startT, PL); )              
      }     
             
#endif
     

    
    


    replicate_move(out, &moves->movements[r]);
    print_movement(out,1);
   
   Vb printf("r = %i\n", r);
   
   DUMP(moves);
   DUMP(_board);
   return r;
   
}

//CUDA kernel functions.
//#####################################################################
#ifdef __CUDACC__
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
#endif
//#####################################################################


Device long thinkiterate(struct board *feed, int PL, int DEEP, int verbose,
        long Alpha, long Beta) {

    int i=0, r=0;
    
    int enemy_score=0, machine_score=0;

    struct board *_board = makeparallelboard(feed);

    int ABcutoff = 0;

    //struct move result;
    
    
    long score=0;
    
    struct movelist moves;

    legal_moves(_board, &moves, PL, 0);

    

    
    /*IFnotGPU(
    Vb printf("DEEP = %i; K=%i Address: %p\n", DEEP,moves.k ,(void *)_board); 
    Vb printf("NullMove!\n"); 
    )*/
     //If in checkmate/stalemate situation;
     if (!moves.k) {
         if (ifsquare_attacked(_board->squares, findking(_board->squares, 'Y', PL), 
                 findking(_board->squares, 'X', PL), PL, 0)) {
            score = 13000 - 50*(BRAIN.DEEP-DEEP); 
            if (PL == Machineplays) score = -score;
         }
          else score = 0;
        
       Vb printf("check/stalemate.\n");
        DUMP(_board);
       return score;
         
     }
    


    //IFnotGPU( Vb show_board(_board->squares); )
   if (DEEP>0) {
     reorder_movelist(&moves); 
    
       //NULL MOVE: guaranteed as long as if PL is not in check,
       //and its not K+P endgame.
       if(DEEP > BRAIN.DEEP - 2)
           if(canNullMove(DEEP, _board, moves.k, PL)) {
	     score = thinkiterate(_board, 1-PL, DEEP-1, verbose, 
	      Alpha, Beta);
               if (PL==Machineplays && score > Alpha) Alpha = score;
               if (PL!=Machineplays && score < Beta ) Beta  = score;}


   //Movelist iteration.    
   for(i=0;i<moves.k;i++) {

               
           
               //continue;
       
       move_pc(_board, &moves.movements[i]);  

       moves.movements[i].score = thinkiterate(_board, 1-PL, DEEP-1, verbose, 
       Alpha, Beta);
       
       //eval_info_move(&_board->movelist[i],DEEP, PL);
       //show_board(_board->squares);
       undo_move(_board, &moves.movements[i]);
       
       //if (show_info) eval_info_move(&_board->movelist[i],DEEP, PL);  
       
       if (PL==Machineplays) 
           if (moves.movements[i].score > Alpha) {
               //printf("*.\n");
               Alpha = moves.movements[i].score; r=i;
           
           if(Beta<=Alpha) ABcutoff=1;}

       if (PL!=Machineplays)
            if (moves.movements[i].score < Beta) {
               //printf("*.\n");
               Beta = moves.movements[i].score; r=i;
            
            if(Beta<=Alpha) ABcutoff=1;
            }
              
        if (ABcutoff) {
            score = moves.movements[i].score;
             DUMP(_board);
            return score;
        }
     }

     score = moves.movements[r].score;  
      DUMP(_board);
    return score;       
   }
     
     else {
     machine_score = evaluate(_board, &moves, Machineplays);
     enemy_score = evaluate(_board, &moves, 1-Machineplays);
     //show_board(_board->squares);
     score = machine_score - enemy_score * BRAIN.presumeOPPaggro;
     //printf("score = %i\n", score);
     //printf(">>>>>>%i.\n", PL);
    //printf("score of %i //     machine = %i     enemy = %i;\n", score, 
   //machine_score, enemy_score);
     IFnotGPU( Vb if (show_info) eval_info_move(&moves.movements[i], DEEP, 0, PL); )
     
     DUMP(_board);
     return score;
     }

}

Device int evaluate(struct board *evalboard, struct movelist *moves, int PL) {
  //int Index = blockIdx.x;
  //printf("E %i\n", Index);
  
    int score = 0;
    
    int i=0, j=0;
    
    int L=0,Z=0,K=0;
    
#ifdef __CUDA_ARCH__
    int chaos = 1;   
#else
    int chaos = rand() % (int)(BRAIN.randomness);

    attackers_defenders(evalboard->squares, *moves, PL);
#endif
    
    //int deadpiece = 0;
    int parallelatks = 0;
    int paralleldefenders = 0;
    
    

    
    forsquares {
        if (evalboard->squares[i][j] == 'x') continue;
        L = getindex(evalboard->squares[i][j], Pieces[PL],6);
        if (L < 0) continue;
        K = BRAIN.pvalues[L];
        
        if (L==0) {
         if (PL) K += i * BRAIN.pawnrankMOD;
         else K += (7-i) * BRAIN.pawnrankMOD;
        }
            
            
   
     score += K * BRAIN.seekpieces + 
        ((-power(j,2)+7*j-5)+(-power(i,2)+7*i-5)) * BRAIN.seekmiddle;
          

}
        
        
        
     for (Z=0;Z<moves->kad;Z++) {
        L = getindex(moves->defenders[Z][0],Pieces[1-PL],6); 
        
        parallelatks = ifsquare_attacked
        (evalboard->squares,moves->defenders[Z][1],
                            moves->defenders[Z][2], 1-PL,0);
                
     if (L==5 && BRAIN.parallelcheck) {
          if (parallelatks>1) 
            score = score + (parallelatks) * BRAIN.parallelcheck;}
     else {
        paralleldefenders = ifsquare_attacked
        (evalboard->squares,moves->defenders[Z][1],
                            moves->defenders[Z][2], PL,0);
        
       score = score - paralleldefenders * BRAIN.MODbackup;
         
         
     }       
                               
         score = score + BRAIN.pvalues[L]*BRAIN.seekatk;
         
         L = getindex(moves->attackers[Z][0],Pieces[PL],6);
         //score = score - BRAIN.pvalues[L] * BRAIN.balanceoffense;
         
         
         }
    score = score + chaos;       
    score = score + moves->k * BRAIN.MODmobility;
    
    
    //printf("score: %i\n", score);

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
    
    
    /*  if (method == 3) {

      modifier = BRAIN.TIMEweight[(int)(BRAIN.DEEP-(float)DEEP)];
        
      }*/
    
    
    
    
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
                   if (ifsquare_attacked (board->squares,i,j,P,0)) return 0;                   }
       
	   return NullMove;       
               
}
