#include "ev_chess.h"


int think (struct move *out, int PL, int DEEP, int verbose) {
    int i=0; int r=0;
    int last[2]={0,-32760};

    Vb printf("thinking r:%i  k:%i DEEP:%i.\n",r,board.k,DEEP);
    

    struct board *_board = makeparallelboard(&board);  
    
    
    long Alpha = -1690000;
    long Beta = +1690000;
    
    
    legal_moves(_board, PL, 0); 
    if (_board->k == 0) return -1;
    Vb printf("value of k is %i.\n",_board->k);
    
   
    for (i=0;i<_board->k;i++) {
    //infoMOVE = (char *)malloc(sizeof(char)*128);
        
        //show_board(board.squares);
     Vb printf("new tree branching. i=%i\n",i);
     move_pc(_board, &_board->movelist[i]);    
     _board->movelist[i].score = thinkiterate(_board, PL, DEEP, 0,
             Alpha, Beta);
     
     //if (_board->movelist[i].score > Alpha) Alpha
     
     //show_board(_board->squares);
     undo_move(_board, &_board->movelist[i]); 
             
     if (_board->movelist[i].score>last[1]) {
         last[1] = _board->movelist[i].score;
         last[0] = i;}
     
   
     Vb printf("analyzed i=%i; score is %i.\n",i, _board->movelist[i].score);

     if (show_info) eval_info_move(&_board->movelist[i],DEEP, PL);              
             
             

     }

    

    r = last[0];

    replicate_move(out, &_board->movelist[r]);

    
   Vb printf("r = %i\n", r);

   
   return r;
}

int evaluate(struct board *evalboard,int PL) {
    int score = 0;
    
    int i = 0;
    int j = 0;
    int L=0,Z=0,K=0;
    
    int chaos = rand() % (int)(Brain.randomness);
  
    int deadpiece = 0;
    int parallelatks = 0;
    
    

    
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
            

     
     if (L==5) if (Brain.parallelcheck) {
         parallelatks = ifsquare_attacked(evalboard->squares,i,j,PL,0);
         if (parallelatks>-1) 
            score = score + (parallelatks) * Brain.parallelcheck;}
     
     
     
     
        }
        
        
        
         for (Z=0;Z<evalboard->kad;Z++) score = score +
         Brain.pvalues[getindex(evalboard->defenders[Z][0],pieces[1-PL],6)]
                 *Brain.seekatk;
 

    score = score+chaos;       
    
    
    
    //printf("score: %i\n", score);
    return score;
    
}

long thinkiterate(struct board *feed, int PL, int DEEP, long chainscore,
        long Alpha, long Beta) {


    int i=0;int t=0;int W=0;int top[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    
    int enemy_score=0;int machine_score=0;
    

    struct board *_board = makeparallelboard(feed);
    
   
    int ABcutoff = 0;

    //struct move result;
   
    long score=0;
   
     
    legal_moves(_board, PL, 0);
    reorder_movelist(_board);
    
    


     
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
    
       


    
   if (DEEP>0) {
   for(i=0;i<_board->k;i++) {
       
       move_pc(_board, &_board->movelist[i]);  
       
       _board->movelist[i].score = thinkiterate(_board, 1-PL, DEEP-1, 0, 
               Alpha, Beta);
       
       //eval_info_move(&_board->movelist[i],DEEP, PL);
       //show_board(_board->squares);
       undo_move(_board, &_board->movelist[i]);
       
       if (PL==machineplays) {
           if (_board->movelist[i].score > Alpha) 
               Alpha = _board->movelist[i].score;
           if(Beta<=Alpha) ABcutoff=1;
       }
               
       
       else {
            if (_board->movelist[i].score < Beta) 
               Beta = _board->movelist[i].score;
            if(Alpha<=Beta) ABcutoff=1;
       }
       
        if (ABcutoff) 
        {free(_board);return _board->movelist[i].score;}
     }}
     
     else {
     machine_score = evaluate(_board,  machineplays);
     enemy_score = evaluate(_board, 1-machineplays);
             
     score = machine_score - enemy_score *Brain.presumeOPPaggro;
     
     //printf(">>>>>>%i.\n", PL);
    //printf("score of %i //     machine = %i     enemy = %i;\n", score, 
            //machine_score, enemy_score);
    
     
     score = score;// * scoremod(DEEP, Brain.evalmethod);
     free(_board);
     return score;
     }
    
   
   if(PL==machineplays) W=1; else W=-1;     
   select_top(_board->movelist, _board->k, top, W);
   
    
   //if(show_info) eval_info_move(&_board->movelist[top[0]], DEEP, PL); 

   
   score = _board->movelist[top[0]].score;//printf("score is %i\n", score);

   //if (PL!=machineplays) score = -score;
 
    //printf("found i=%i and i=%i.\n", top[0][0], top[1][0]);
   
   free(_board);
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