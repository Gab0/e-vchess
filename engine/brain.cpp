#include "ev_chess.h"


#define DUMP(B) if(B!=NULL){free(B);B=NULL;}
       

int think (struct move *out, int PL, int DEEP, int verbose) {
    
    int i=0; int r=0;
    long score = -17000;
    time_t startT = time(NULL);
    
    struct board *_board = makeparallelboard(&board);
    _board->Nmoved=0;
    struct board *dummyboard = makeparallelboard(&board);
    
    struct board *BufferBoard;
    //Vb printf("Master Address: %p\n", (void *)_board); 
    
    long Alpha = -169000;
    long Beta = 169000;
    
    struct movelist *moves = (struct movelist *) calloc(1, sizeof(struct movelist));
    
    PL = Machineplays;
    legal_moves(_board, moves, PL, 0); 
    
    reorder_movelist(moves);

    struct board *finalboardsArray =
      (struct board*) calloc(moves->k, sizeof(struct board));
    
    //printf("thinking r:%i  k:%i DEEP:%i.\n",r,moves->k,DEEP);
    if (moves->k == 0) return -1;
    //Vb printf("value of k is %i.\n",moves->k);
    
    //infoMOVE = (char *)malloc(sizeof(char)*128);

    int AllowCutoff = 1;
    if (BRAIN.xDEEP) AllowCutoff = 0;
 
// cuda move evaluating.        
#ifdef __CUDACC__        
    #include "brain_cuda0.cpp"

// non cuda move evaluating mehthod.        
#else           
    if(canNullMove(DEEP, _board, moves->k, PL)) {
      flip(_board->whoplays);
      BufferBoard = thinkiterate(_board, 1-PL, DEEP-1, verbose, Alpha, Beta, AllowCutoff);
      flip(_board->whoplays);
               if (PL==Machineplays && BufferBoard->score > Alpha) Alpha = BufferBoard->score;
               if (PL!=Machineplays && BufferBoard->score < Beta)  Beta = BufferBoard->score;
	       DUMP(BufferBoard)
           }
    

      for (i=0;i<moves->k;i++) {
	//printf(">>AB %i; %i;\n", Alpha, Beta);        
        //show_board(board.squares);
	//Vb printf("new tree branching. i=%i\n",i);
     //Vb print_movement(&moves->movements[i],0);
     //Vb printf("Alpha = %i\n", Alpha);
     
     move_pc(_board, &moves->movements[i]);    
     BufferBoard = thinkiterate(_board, 1-PL, DEEP-1, verbose, Alpha, Beta, AllowCutoff);


     
     moves->movements[i].score = BufferBoard->score;
     cloneboard(BufferBoard, &finalboardsArray[i]);
     //printf("SCR %i %i\n", BufferBoard->score, &finalboardsArray[i].score);
     fprintf(stderr, "FNM: %i\n", BufferBoard->Nmoved);
     DUMP(BufferBoard);
     
     if (moves->movements[i].score > Alpha) Alpha = moves->movements[i].score;
 

     if (moves->movements[i].score > score) {
     score = moves->movements[i].score;
     r=i;
     }
     
     //show_board(_board->squares);
     undo_move(_board, &moves->movements[i]);
     
     DUMP(BufferBoard);
     
     //Vb printf("analyzed i=%i; score is %ld.\n",i, moves->movements[i].score);
     //IFnotGPU( Vb if (show_info) printf(">>>>>>>>\n"); )
     IFnotGPU( /*if (show_info)*/ eval_info_move(&moves->movements[i], DEEP, startT, PL); )              
      }     
             
#endif


//second-level deepness move search schematics;##################################
      int Tcutoff = 6;
      int T = moves->k;
      if (T > Tcutoff) T = Tcutoff;


      int secondTOP[T], I=0, M=0, s=0;

     long sessionSCORE=0;
     long movelistSCORE=0;

     
     struct board *BoardBuffer = makeparallelboard(_board);
     selectBestMoves(moves->movements, moves->k, secondTOP, T);

     struct movelist *nextlevelMovelist =
       (struct movelist*) calloc(T, sizeof(struct movelist));


     int R=0;
     int Lever=0;
     
     for (R=0;R<BRAIN.xDEEP;R++) {
       AllowCutoff = 1;
       if (R + 1 == BRAIN.xDEEP) AllowCutoff = 1;
     sessionSCORE = -17000;

      
     // printf("\n\n");
     for (i=0;i<T;i++) {
 
       Lever = 1;
       
       movelistSCORE = -17000;
       Alpha = -17000;
       Beta = 17000;
       I = secondTOP[i];
       
       if (finalboardsArray[I].betaCut) continue;

       fprintf(stderr, "----------------------------------------------------------\n");
       fprintf(stderr, "Nm: %i || original R: %i || current R: %i\n", finalboardsArray[I].Nmoved, I, r);
       fprintf(stderr, "Wp:%i  || I:  %i\n", finalboardsArray[I].whoplays, I);
       if (finalboardsArray[I].Nmoved % 4) {
	 //Lever -= (BRAIN.DEEP-finalboardsArray[I].Nmoved);
       printf("DISCREPANCY DETECTED ~~~~~~~~~~~~~~~~~~~%i~~~~~~~%i~~~~~\n", finalboardsArray[I].Nmoved, Lever);}
       //show_board(finalboardsArray[I].squares);

       PL = finalboardsArray[I].whoplays;

       if (finalboardsArray[I].score != moves->movements[I].score) fprintf(stderr, "DANGER wp%i    %i||%i\n", PL, finalboardsArray[I].score, moves->movements[I].score);

       //if (PL != Machineplays) exit(-1);// Lever = 2;
       
       legal_moves(&finalboardsArray[I], &nextlevelMovelist[i], PL, 0);
       reorder_movelist(&nextlevelMovelist[i]);
       dummyboard = 0;
       
       if (nextlevelMovelist[i].k) {
	 //Alpha = -17000;
       //Beta = 17000;
       for (M=0;M<nextlevelMovelist[i].k;M++) {
	 move_pc(&finalboardsArray[I], &nextlevelMovelist[i].movements[M]);

	 dummyboard =
	   thinkiterate(&finalboardsArray[I], 1-PL, DEEP-Lever, 0, Alpha, Beta, AllowCutoff);
	 
	 nextlevelMovelist[i].movements[M].score = dummyboard->score;
	 undo_move(&finalboardsArray[I], &nextlevelMovelist[i].movements[M]);


	   if (nextlevelMovelist[i].movements[M].score > movelistSCORE) {
	     cloneboard(dummyboard, BoardBuffer);
	     movelistSCORE = nextlevelMovelist[i].movements[M].score;
	     moves->movements[I].score = movelistSCORE;
	     s = M;

	   }

	   
	   DUMP(dummyboard);
       }

        cloneboard(BoardBuffer, &finalboardsArray[I]);
     }
       else {
	 movelistSCORE = moves->movements[I].score;
	 s = 63;
       }
	 
      
	 if (movelistSCORE > sessionSCORE) {
	   r = I;
	   sessionSCORE = movelistSCORE;
	   fprintf(stderr, "CHOSEN\n", r);
	 }

	 

      

       fprintf(stderr, "FNM: %i || FWP: %i\n", finalboardsArray[I].Nmoved, finalboardsArray[I].whoplays);

       
       //cloneboard(dummyboard, &finalboardsArray[I]);
       
       
       //if (nextlevelMovelist[individualINDEX].score) > sessionSCORE) {sessionSCORE = nextlevelMovelist[individualINDEX].score; r = I;}
       IFnotGPU( /*if (show_info)*/ eval_info_group_move(&moves->movements[I], &nextlevelMovelist[i].movements[s], (2+R)*DEEP, startT, PL); )
	 
              
     }

     }

     // int finalR[0];

     
     //selectBestMoves(nextlevelMovelist[i].movements, nextlevelMovelist[i].k, finalR, 1);
     
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


Device struct board *thinkiterate(struct board *feed, int PL, int DEEP, int verbose,
				  long Alpha, long Beta, int AllowCutoff) {

    int i=0, r=0;
    
    int enemy_score=0, machine_score=0;

    struct board *_board = makeparallelboard(feed);

    struct board *Buffer;


    int ABcutoff = 0;

    //if (PL != feed->whoplays) printf("INCONSISTENT PLayer=%i; Deep=%i\n", PL, DEEP);
    //struct move result;
    
    
    long score=0;
    int HoldBuffer=0;
    struct movelist moves;

    legal_moves(_board, &moves, PL, 0);

    

    
    /*IFnotGPU(
    Vb printf("DEEP = %i; K=%i Address: %p\n", DEEP,moves.k ,(void *)_board); 
    Vb printf("NullMove!\n"); 
    )*/
     //If in checkmate/stalemate situation;
     if (!moves.k) {
       if (_board->whoplays != PL) fprintf(stderr, "PL vs whoplays ERROR.\n");
       
         if (ifsquare_attacked(_board->squares, findking(_board->squares, 'Y', PL), 
                 findking(_board->squares, 'X', PL), PL, 0)) {
            score = 13000 - 50*(BRAIN.DEEP-DEEP); 
            if (PL == Machineplays) score = -score;

         }
	 else score = 0; 
	 //printf("DRAW/CHKMATEendoftheline\n");
	 //if (((PL == Machineplays && score > Alpha) || (PL != Machineplays && score < Beta))
	 
	 /* if (finalboard) {
       if( (PL == Machineplays && score > finalboard->score) ||
        (PL != Machineplays && score < finalboard->score) )  {       _board->score = score;       cloneboard(_board, finalboard);}
     }*/
	 //Vb printf("check/stalemate.\n");
	 
     _board->score = score; 
       return _board;
         
     }
    


    //IFnotGPU( Vb show_board(_board->squares); )
   if (DEEP>0) {
     struct board *BoardBuffer = makeparallelboard(feed);
     reorder_movelist(&moves); 
    
       //NULL MOVE: guaranteed as long as if PL is not in check,
       //and its not K+P endgame.
     if(DEEP > BRAIN.DEEP - 2) 
       if(canNullMove(DEEP, _board, moves.k, PL)) {
	 flip(_board->whoplays);
	 Buffer = thinkiterate(_board, 1-PL, DEEP-1, verbose,
			       Alpha, Beta, AllowCutoff);
	 flip(_board->whoplays);
	 if (PL==Machineplays && Buffer->score > Alpha) Alpha = Buffer->score;
	 if (PL!=Machineplays && Buffer->score < Beta ) Beta  = Buffer->score;
	 //DUMP(Buffer);
	 cloneboard(Buffer, BoardBuffer);

       }
	 if (PL==Machineplays) BoardBuffer->score = -17000;
	 else BoardBuffer->score = 17000;	   

   // Movelist iteration.


   //long OAlpha=0, OBeta=0;
	 
   for(i=0;i<moves.k;i++) {
     //printf("new tree;\n");
     
       move_pc(_board, &moves.movements[i]);  

       Buffer = thinkiterate(_board, 1-PL, DEEP-1, verbose, Alpha, Beta, AllowCutoff);
	//printf("%i\n", Buffer->score);
	moves.movements[i].score = Buffer->score;


	
	//printf(">>>%i\n", moves.movements[i].score);
	//if (!i) cloneboard(Buffer, BoardBuffer);
	//if (moves.movements[i].score != Buffer->score) printf("FAIL!\n");
 
       //show_board(_board->squares);
       
       
	//if (show_info)
	//eval_info_move(&moves.movements[i], DEEP, 0, PL);  
       
	if (PL==Machineplays) {
	 if (moves.movements[i].score > BoardBuffer->score) cloneboard(Buffer, BoardBuffer);
	 if (moves.movements[i].score > Alpha) {
	   //printf("*.\n");
	   //OAlpha = Alpha;
	   Alpha = moves.movements[i].score;
	   //printf("gg\n");
	   //printf("%i\n", Buffer->score);
	   //cloneboard(Buffer, BoardBuffer);
	   //if(Buffer->score != BoardBuffer->score) printf("FAIL a !\n");
	   //printf("%i\n", Buffer->score);
	   
           if (Beta<=Alpha)
	     if (AllowCutoff)
	       ABcutoff=1;

	 }
	}

	
	if (PL!=Machineplays) {
	 if (moves.movements[i].score < BoardBuffer->score) cloneboard(Buffer, BoardBuffer);
         if (moves.movements[i].score < Beta) {
	    //printf("**.\n");
	   //OBeta = Beta;    
           Beta = moves.movements[i].score;

	   //cloneboard(Buffer, BoardBuffer);
  	   //if(Buffer->score != BoardBuffer->score) printf("FAIL b !\n");

	    if (Beta<=Alpha)
	      if (AllowCutoff)
		ABcutoff=1;

      
            }
        }      
        DUMP(Buffer);
	if (ABcutoff) {
	  
	  /* asprintf(&output, "*%i --  %i -> %i | %i -> %i  ||%i\n", PL,
		   OAlpha, Alpha, OBeta, Beta, moves.movements[i].score);
		   write(1, output, strlen(output));*/
	  //if(PL==Machineplays) fprintf(stderr, "*%i\n", PL);
	  if (!AllowCutoff &&)
	  break;
	}	
	undo_move(_board, &moves.movements[i]);
	


     }

   //score = moves.movements[r].score;
   
    DUMP(_board);
    
    return BoardBuffer;
    
   }
     
     else {

     machine_score = evaluate(_board, &moves, Machineplays);
     enemy_score = evaluate(_board, &moves, 1-Machineplays);
     //show_board(_board->squares);
     
     _board->score = machine_score - enemy_score * (1 + BRAIN.presumeOPPaggro);

     //asprintf(&output, "0 %i 0 %i\n", _board->score, PL);
     //write(1, output, strlen(output));
     //DUMP(_board);

     return _board;
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
        L = getindex(evalboard->squares[i][j], Pieces[PL], 6);
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
