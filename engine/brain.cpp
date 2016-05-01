#include "ev_chess.h"


#define DUMP(B) if(B!=NULL){free(B);B=NULL;}
       

int think (struct move *out, int PL, int DEEP, int verbose) {
    
    int i=0; int r=0;
    long score = 0;
    time_t startT = time(NULL);

    struct board *_board = makeparallelboard(&board);
    struct board *dummyboard = makeparallelboard(&board);
    
    
    //Vb printf("Master Address: %p\n", (void *)_board); 
    
    long Alpha = -1690000;
    long Beta = 1690000;
    
    struct movelist *moves = (struct movelist *) calloc(1, sizeof(struct movelist));
    
            
    legal_moves(_board, moves, PL, 0); 
    
    reorder_movelist(moves);

    struct board *finalboardsArray =
      (struct board*) calloc(moves->k, sizeof(struct board));
    
    Vb printf("thinking r:%i  k:%i DEEP:%i.\n",r,moves->k,DEEP);
    if (moves->k == 0) return -1;
    Vb printf("value of k is %i.\n",moves->k);
    
    //infoMOVE = (char *)malloc(sizeof(char)*128);
    
//#define __CUDACC__    
// cuda move evaluating.        
#ifdef __CUDACC__        
    #include "brain_cuda0.cpp"

// non cuda move evaluating mehthod.        
#else           
    if(canNullMove(DEEP, _board, moves->k, PL)) {
      flip(_board->whoplays);
      score = thinkiterate(_board, 1-PL, DEEP-1, verbose,
			   dummyboard, Alpha, Beta);
      flip(_board->whoplays);
               if (PL==Machineplays && score > Alpha) Alpha = score;
               if (PL!=Machineplays && score < Beta)  Beta = score;
               
           }     
      for (i=0;i<moves->k;i++) {
        
        //show_board(board.squares);
     Vb printf("new tree branching. i=%i\n",i);
     //Vb print_movement(&moves->movements[i],0);
     //Vb printf("Alpha = %i\n", Alpha);
     
     move_pc(_board, &moves->movements[i]);    
     moves->movements[i].score = thinkiterate(_board, 1-PL, DEEP-1, verbose,
					      &finalboardsArray[i], Alpha, Beta);
     
     if (moves->movements[i].score > Alpha) {
         Alpha = moves->movements[i].score;
         r=i;

     }
     
     //show_board(_board->squares);
     undo_move(_board, &moves->movements[i]); 

     
     //Vb printf("analyzed i=%i; score is %ld.\n",i, moves->movements[i].score);
     //IFnotGPU( Vb if (show_info) printf(">>>>>>>>\n"); )
     IFnotGPU( /*if (show_info)*/ eval_info_move(&moves->movements[i],DEEP, startT, PL); )              
      }     
             
#endif


//second-level deepness move search schematics;##################################
      int Tcutoff = 12;
      int T = moves->k;
      if (T > Tcutoff) T = Tcutoff;

      int secondTOP[T], I=0, M=0, s=0;

     long sessionSCORE = -16000;
     long individualSCORE = 0;
     int individualINDEX = 0;
     selectBestMoves(moves->movements, moves->k, secondTOP, T);

     struct movelist *nextlevelMovelist =
       (struct movelist*) calloc(T*2, sizeof(struct movelist));
     
     printf("\n\n");
     for (i=0;i<T;i++){
       I = secondTOP[i];
       
       //printf("original R: %i\n", I);
       //show_board(finalboardsArray[I].squares);

       PL = finalboardsArray[I].whoplays;
       legal_moves(&finalboardsArray[I], &nextlevelMovelist[i], PL, 0);
       reorder_movelist(&nextlevelMovelist[i]);

       if (nextlevelMovelist[i].k)
       for (M=0;M<nextlevelMovelist[i].k;M++) {
	 move_pc(&finalboardsArray[I], &nextlevelMovelist[i].movements[M]);
	 
	 nextlevelMovelist[i].movements[M].score =
	   thinkiterate(&finalboardsArray[I], 1-PL, DEEP-1, 0,
			dummyboard, Alpha, Beta);
	 
	 undo_move(&finalboardsArray[I], &nextlevelMovelist[i].movements[M]);
	 
	   
	 if (nextlevelMovelist[i].movements[M].score > Alpha)// && PL == Machineplays)
	   Alpha = nextlevelMovelist[i].movements[M].score;
	 //if (nextlevelMovelist[i].movements[M].score < Beta && PL != Machineplays)
	 //  Beta = nextlevelMovelist[i].movements[M].score;
	 if ((nextlevelMovelist[i].movements[M].score > sessionSCORE))// && PL == Machineplays) ||
	   // (nextlevelMovelist[i].movements[M].score < sessionSCORE && PL != Machineplays))
	   {
	   sessionSCORE = nextlevelMovelist[i].movements[M].score;
	   //if (sessionSCORE > moves->movements[r].score) {//??? profitable???
	   r = I;
	   s = M;
	   //}
	   }

	  
	 
       }

       else {
	 if ((moves->movements[I].score > sessionSCORE))// && PL == Machineplays)) || (moves->movements[I].score < sessionSCORE && PL != Machineplays))
	   {
	 sessionSCORE = moves->movements[I].score;
	 r = I;
	 s = 0;
	 }
	   }


       
       //selectBestMoves(nextlevelMovelist[i].movements, nextlevelMovelist[i].k, individualINDEX, 1);
       //if (nextlevelMovelist[individualINDEX].score) > sessionSCORE) {sessionSCORE = nextlevelMovelist[individualINDEX].score; r = I;}
       IFnotGPU( /*if (show_info)*/ eval_info_group_move(&moves->movements[I], &nextlevelMovelist[i].movements[s], 2*DEEP, startT, PL); )
       
     }
    replicate_move(out, &moves->movements[r]);
    //print_movement(out,1);
   
    //Vb printf("r = %i\n", r);
   
   DUMP(moves);
   DUMP(_board);
   DUMP(finalboardsArray);
   DUMP(nextlevelMovelist);
   return r;
   
}

//CUDA kernel functions.
//#####################################################################
#ifdef __CUDACC__
#include "brain_cuda1.cpp"
#endif
//#####################################################################


Device long thinkiterate(struct board *feed, int PL, int DEEP, int verbose,
			 struct board *finalboard,  long Alpha, long Beta) {

    int i=0, r=0;
    
    int enemy_score=0, machine_score=0;

    struct board *_board = makeparallelboard(feed);

    int ABcutoff = 0;

    if (PL != feed->whoplays) printf("INCONSISTENT PLayer=%i; Deep=%i\n", PL, DEEP);
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
	 //printf("DRAW/CHKMATEendoftheline\n");
	 if (((PL == Machineplays && score > Alpha) || (PL != Machineplays && score < Beta))
	     && finalboard) cloneboard(_board, finalboard);
	 //Vb printf("check/stalemate.\n");
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
	     flip(_board->whoplays);
	     score = thinkiterate(_board, 1-PL, DEEP-1, verbose,
				  0, Alpha, Beta);
	     flip(_board->whoplays);
               if (PL==Machineplays && score > Alpha) Alpha = score;
               if (PL!=Machineplays && score < Beta ) Beta  = score;}


   //Movelist iteration.    
   for(i=0;i<moves.k;i++) {

               
           
               //continue;
       
       move_pc(_board, &moves.movements[i]);  

       moves.movements[i].score = thinkiterate(_board, 1-PL, DEEP-1, verbose,
					       finalboard, Alpha, Beta);
       
       //eval_info_move(&_board->movelist[i],DEEP, PL);
       //show_board(_board->squares);
       
       
       //if (show_info) eval_info_move(&_board->movelist[i],DEEP, PL);  
       
       if (PL==Machineplays) 
           if (moves.movements[i].score > Alpha) {
               //printf("*.\n");
	    
               Alpha = moves.movements[i].score; r=i;
           
           if(Beta<=Alpha) ABcutoff=1;

	   if ((DEEP==1||ABcutoff)&&finalboard) cloneboard(_board, finalboard);
	   }

       if (PL!=Machineplays)
            if (moves.movements[i].score < Beta) {
               //printf("*.\n");
               Beta = moves.movements[i].score; r=i;

            if(Beta<=Alpha) ABcutoff=1;
	    if ((DEEP==1||ABcutoff)&&finalboard) cloneboard(_board, finalboard);
            }
              
        if (ABcutoff) {
            score = moves.movements[i].score;
            DUMP(_board);
            return score;
        }

	undo_move(_board, &moves.movements[i]);
     }

     score = moves.movements[r].score;  
      DUMP(_board);
    return score;       
   }
     
     else {
     machine_score = evaluate(_board, &moves, Machineplays);
     enemy_score = evaluate(_board, &moves, 1-Machineplays);
     //show_board(_board->squares);
     score = machine_score - enemy_score * (1 + BRAIN.presumeOPPaggro);
     //printf("score = %i\n", score);
     //Vb printf(">>>>>>%i.\n", PL);
     //Vb printf("score of %i //     machine = %i     enemy = %i;\n", score, 
     //machine_score, enemy_score);
     //IFnotGPU( Vb if (show_info) eval_info_move(&moves.movements[i], DEEP, 0, PL); )
     
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
        ((-power(j,2)+7*j-5) + (-power(i,2)+7*i-5)) * BRAIN.seekmiddle;
          

}
        
        
        
     for (Z=0;Z<moves->kad;Z++) {
        L = getindex(moves->defenders[Z][0],Pieces[1-PL],6); 
        
        parallelatks = ifsquare_attacked
        (evalboard->squares,moves->defenders[Z][1],
                            moves->defenders[Z][2], 1-PL,0);
                
     if (L==5 && BRAIN.parallelcheck) {
          if (parallelatks>1) 
            score += parallelatks * BRAIN.parallelcheck;}
     else {
        paralleldefenders = ifsquare_attacked
        (evalboard->squares,moves->defenders[Z][1],
                            moves->defenders[Z][2], PL,0);
        
       score = score - paralleldefenders * BRAIN.MODbackup;
         
         
     }       
                               
         score += BRAIN.pvalues[L]*BRAIN.seekatk;
         
         L = getindex(moves->attackers[Z][0],Pieces[PL],6);
         score -= BRAIN.pvalues[L] * BRAIN.balanceoffense;
         
         
         }
    score += chaos;       
    score += moves->k * BRAIN.MODmobility;
    
    
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
