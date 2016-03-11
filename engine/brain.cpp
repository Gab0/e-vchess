#include "ev_chess.h"


int think (struct move *out, int PL, int DEEP, int verbose) {
    int i=0; int r=0;
    int last[2]={0,-32760};

    int bI = 0;
    Vb printf("thinking r:%i  k:%i DEEP:%i.\n",r,board.k,DEEP);
    

    struct board *_board = makeparallelboard(&board);  
    
    
    long Alpha = -1690000;
    long Beta = 1690000;
    
        
    legal_moves(_board, PL, 0); 
    
    if (_board->k == 0) return -1;
    Vb printf("value of k is %i.\n",_board->k);
    
   
    for (i=0;i<_board->k;i++) {
    //infoMOVE = (char *)malloc(sizeof(char)*128);
        
        //show_board(board.squares);
     Vb printf("new tree branching. i=%i\n",i);
     move_pc(_board, &_board->movelist[i]);    
     _board->movelist[i].score = thinkiterate(_board, 1-PL, DEEP-1, 0,
             Alpha, Beta);
     
     if (_board->movelist[i].score > Alpha) {
         Alpha = _board->movelist[i].score;
         r=i;
     }
     
     //show_board(_board->squares);
     undo_move(_board, &_board->movelist[i]); 

     
     Vb printf("analyzed i=%i; score is %li.\n",i, _board->movelist[i].score);
     Vb if (show_info) printf(">>>>>>>>\n");
     if (show_info) eval_info_move(&_board->movelist[i],DEEP, PL);              
             
             

     }

    

    //r = last[0];

    replicate_move(out, &_board->movelist[r]);

    
   Vb printf("r = %i\n", r);

   
   return r;
   free(_board);
}

long thinkiterate(struct board *feed, int PL, int DEEP, long chainscore,
        long Alpha, long Beta) {


    int i=0, j=0, t=0, W=0, r=0;
    
    int enemy_score=0,machine_score=0;
    

    struct board *_board = makeparallelboard(feed);
    
   
    int ABcutoff = 0;

    //struct move result;
   
    long score=0, NullMove=0;
   
     
    legal_moves(_board, PL, 0);
    

     if (_board->k == 0) {
         if (ifsquare_attacked(_board->squares, findking(_board->squares, 'Y', PL), 
                 findking(_board->squares, 'X', PL), PL, 0)) {
            score = 13000 - 50*(Brain.DEEP-DEEP); 
            if (PL == machineplays) score = -score;
            
         }
             
         else score = 0;

       free(_board);
       return score;
         
     }
    
     else reorder_movelist(_board);  


    
   if (DEEP>0) {
       //NULL MOVE: guaranteed as long as if PL is not in check,
       //and its not K+P endgame.
       if (DEEP>Brain.DEEP-2&&_board->k>5) {
       forsquares {
               if (_board->squares[i][j]!='x'&& 
                   _board->squares[i][j]!=pieces[PL][0]&&
                   !is_in(_board->squares[i][j],pieces[1-PL],6)) NullMove = 1;
               if (_board->squares[i][j]==pieces[PL][5])
                   if (ifsquare_attacked (_board->squares,i,j,PL,0)){
                        NullMove=0;break;break;
                   }
           }
               
           if(NullMove) {
               NullMove = thinkiterate(_board,1-PL,DEEP-1,0,Alpha,Beta);
            if(PL==machineplays) if (NullMove > Alpha) Alpha = NullMove;
            else if (NullMove < Beta) Beta = NullMove;
           }
       }
       

   //Movelist iteration.    
   for(i=0;i<_board->k;i++) {
       
       move_pc(_board, &_board->movelist[i]);  

       _board->movelist[i].score = thinkiterate(_board, 1-PL, DEEP-1, 0, 
               Alpha, Beta);
       
       //eval_info_move(&_board->movelist[i],DEEP, PL);
       //show_board(_board->squares);
       undo_move(_board, &_board->movelist[i]);
       
       //if (show_info) eval_info_move(&_board->movelist[i],DEEP, PL);  
       
       if (PL==machineplays) 
           if (_board->movelist[i].score > Alpha) {
               //printf("*.\n");
               Alpha = _board->movelist[i].score; r=i;
           
           if(Beta<=Alpha) ABcutoff=1;}


       if (PL!=machineplays)
            if (_board->movelist[i].score < Beta) {
               //printf("*.\n");
               Beta = _board->movelist[i].score; r=i;
            
            if(Beta<=Alpha) ABcutoff=1;
            }
       
       
              
        if (ABcutoff) {
            score = _board->movelist[i].score;
            free(_board);
            return score;
        }
     }
   }
     
     else {
     //score = evaluate(_board, PL);
     machine_score = evaluate(_board,  machineplays);
     enemy_score = evaluate(_board, 1-machineplays);
             
     score = machine_score - enemy_score * Brain.presumeOPPaggro;
     
     //printf(">>>>>>%i.\n", PL);
    //printf("score of %i //     machine = %i     enemy = %i;\n", score, 
   //machine_score, enemy_score);
     //if (show_info) eval_info_move(&_board->movelist[i],DEEP, PL);  

     free(_board);
     return score;
     }
    }

int evaluate(struct board *evalboard,int PL) {
    int score = 0;
    
    int i = 0;
    int j = 0;
    int L=0,Z=0,K=0;
    
    int chaos = rand() % (int)(Brain.randomness);   
  
    int deadpiece = 0;
    int parallelatks = 0;
    int paralleldefenders = 0;
    
    attackers_defenders(evalboard, PL);

    
    for (i=0;i<8;i++) for (j=0;j<8;j++) {
        L = getindex(evalboard->squares[i][j],pieces[PL],6);
        if (L<0) continue;
        K = Brain.pvalues[L];
        
        if (L==0) {
         if (PL) K= K + i * Brain.pawnrankMOD;
         else K = K + (7-i) * Brain.pawnrankMOD;
        }  
            
            
   
     score = score + K * Brain.seekpieces +
        ((-power(j,2)+7*j-5)+(-power(i,2)+7*i-5))*Brain.seekmiddle;
            

}
        
        
        
     for (Z=0;Z<evalboard->kad;Z++) {
        L= getindex(evalboard->defenders[Z][0],pieces[1-PL],6); 
        
        parallelatks = ifsquare_attacked
        (evalboard->squares,evalboard->defenders[Z][1],
                            evalboard->defenders[Z][2], 1-PL,0);
                
     if (L==5 && Brain.parallelcheck) {
          if (parallelatks>1) 
            score = score + (parallelatks) * Brain.parallelcheck;}
     else {
        paralleldefenders = ifsquare_attacked
        (evalboard->squares,evalboard->defenders[Z][1],
                            evalboard->defenders[Z][2], PL,0);
        
        score = score - paralleldefenders * Brain.MODbackup;
         
         
     }       
                
                
         score = score + Brain.pvalues[L]*Brain.seekatk;
         
         L = getindex(evalboard->attackers[Z][0],pieces[PL],6);
         score = score - Brain.pvalues[L] * Brain.balanceoffense;
         
         
         }
    score = score+chaos;       
    score = score + evalboard->mobility[PL] * Brain.MODmobility;
    
    
    //printf("score: %i\n", score);
    return score;
    
}

float scoremod (int DEEP, int method) {
    
    float modifier = 0;
    float helper = 0;
    
    if (method>3) method = 0;
    
    
    if (method == 0) modifier = 1;
    
    if (method == 1) modifier = 2*((DEEP+Brain.deviationcalc)/Brain.DEEP);
    
    if (method == 2) {
        modifier = -power(DEEP,2)+Brain.DEEP*DEEP+2*Brain.DEEP;
        

    helper = Brain.DEEP/2;
        
            helper = -power(helper,2)+Brain.DEEP*helper+2*Brain.DEEP;
            
            modifier = modifier/(helper/1.1);
    }
    
    
    if (method == 3) {

        modifier = Brain.TIMEweight[(int)(Brain.DEEP-DEEP)];
        
    }
    
    
    
    
    return modifier;
}