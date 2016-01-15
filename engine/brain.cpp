#include "ev_chess.h"
#include "ev_evolution.h"

int think (struct move *out, int PL, int DEEP, int verbose) {
    int i=0;
    int last[2]={0,-32760};
    char movebuff[3][2] = {};
    legal_moves(&board, PL, 0);
    int r = 0;
    Vb printf("thinking r:%i  k:%i DEEP:%i.\n",r,board.k,DEEP);
    
    struct move showmovebuff;

    int score=0;
    
    char *INDEX=" ";
    
    
    legal_moves(&board, PL, 0); 
    if (board.k == 0) return -1;
    //printf("value of k is %i.\n",board.k);
    for (i=0;i<board.k;i++) {

        //printf("new tree branching. i=%i\n",i);
        //show_board(board.squares);
        
             
     score = thinkiterate(&board, PL, DEEP, 0, &board.movelist[i],0);

     Vb printf("analyzed i=%i; score is %i.\n",i,score);
     
     if (show_info) {
     /*schemes to show current move evaluation to Xboard*/
     replicate_move(&showmovebuff, &board.movelist[i]);
     cord2pos(showmovebuff.from);
     cord2pos(showmovebuff.to);
     

     /* printing info to xboard; format is: ply score time nodes pv*/
     snprintf(output, strlen(infoAUX)+32, "%i %i 0 0 %c%c%c%c %s\n", DEEP, score, 
     showmovebuff.from[0], showmovebuff.from[1], showmovebuff.to[0],showmovebuff.to[1], infoAUX);
        write(1, output, strlen(output));      
     
        
        
        infoAUX = (char *)malloc(256 * sizeof(char));
        infomoveINDEX=0;
       
     }
             
             
             
     if (score>last[1]) {
         last[1] = score;
         last[0] = i;
     }
        //legal_moves(&board, PL);  
    }
    

    r = last[0];

    

    
    replicate_move(out,&board.movelist[r]);

    
    
   Vb printf("r = %i\n", r);
   if (show_info) free(infoAUX);
   return r;
}

int evaluate(struct board *evalboard, struct move *move, int PL) {
    int score = 0;
    
    int i = 0;
    int j = 0;
    int sausage = rand() % eval_randomness;
    sausage = sausage-eval_randomness/2;
    int deadpiece = 0;
    
    if (PL!=machineplays) {
        int param_seekpieces = param_seekpieces * param_presumeOPPaggro;
        int param_seekatk = param_seekatk * param_presumeOPPaggro;
    }

    
    if (move->casualty != 'x'){
        deadpiece=getindex(move->casualty,pieces[1-PL],6);
        score = score + (pvalues[deadpiece]*param_seekpieces);
        
        
    }
    attackers_defenders(evalboard, PL);

    
    
    for (i=0;i<evalboard->kad;i++) {
           
         j = getindex(evalboard->defenders[i][0],pieces[1-PL],6);
         score = score + (pvalues[j])+param_seekatk;
    }
    
    score = score+sausage;

    
    
    for (i=0;i<8;i++) for (j=0;j<8;j++) 
        if (is_in(evalboard->squares[i][j],pieces[PL],6)) {

            score = score + (-power(j,2)+7*j-5)*param_seekmiddle;
            score = score + (-power(i,2)+7*i-5)*param_seekmiddle;
            
            if (is_in(evalboard->squares[i][j], pieces[PL],1)){
                if (PL) score = score + i * param_pawnrankMOD;
                else score = score + (7-i) * param_pawnrankMOD;        
            }
        }
        
        
    
    
    
    
    
    
    
    
    
    //printf("score: %i\n", score);
    return score;
    
}

int thinkiterate (struct board *feed, int PL,int DEEP, int chainscore, struct move *move, char *INDEX) {

    int i=0;
    
    int t=0;
    //legal_moves(board, PL);
    
    struct move movebuff; 
    
    //printf("cloning board.\n");
    struct board *_board = makeparallelboard(feed);
    
    int top[16]={0};
    
    char *nextINDEX = (char *)malloc(8*sizeof(char));;
    
    char bestmoves[2][3][2]={0};
    int branch_score[16]={0};
    
   
    int score=0;
    
    //show_board(squares);

    
    //printf("before:\n");
    //show_board(_board->squares);
    
    move_pc(_board, move);
    
    //printf("during:\n");
    //show_board(_board->squares);
    
    
   
    

    
    //printf("iterating --k=%i --DEEP=%i --P=%i \n\n",_board->k,DEEP,PL);
    //printf("%p\n", _board->squares);  
    //printf("it's here.\n");
 
    legal_moves(_board, PL, 0);
    
    
    
    if (chainscore<-5000) {free(_board);free(nextINDEX);return chainscore;}

     
     if (_board->k == 0) {
         if (ifsquare_attacked(_board, findking(_board->squares, 'Y', PL), 
                 findking(_board->squares, 'X', PL), PL, 0)) {
            score = 1300 - 50*(param_DEEP-DEEP); 
            if (PL == machineplays) score = -score;
            
         }
             
         else score = 0;
      
         
         
  
         free(nextINDEX);
         free(_board);

   return score;
         
     }

     score = evaluate(_board, move, PL);
     
     score = score*scoremod(DEEP, param_evalmethod);
     
     if (PL!=machineplays) score = -score;
     
     
     score = chainscore + score;
     
     
     
     
     //printf("score is %i\n",score);

     
     

        //now searching and evaluations are for his enemy. 
     
    if (DEEP>0) {  
     
    
    legal_moves(_board,1-PL, 0);
    

    
    
    for (i=0;i< _board->k;i++) {   
    //printf("going. D=%i k=%i\n\n", DEEP,i);    
   
    replicate_move(&movebuff, &_board->movelist[i]);    
    //printf("got, i=%i \n",i);
    move_pc(_board, &movebuff);  

    
    _board->evaltable[i] = evaluate(_board, &movebuff, 1-PL);
 
    //printf("printing to evaltable k=%i, i=%i.   score = %lli\n", _board->k,i,_board->evaltable[i]);
    undo_move(_board, &movebuff);
    //printf("must be equal 'during'.\n");
    //show_board(_board->squares);
      } 
    

    
    select_top(_board->evaltable, _board->k, top, param_aperture);


  
    //printf("found i=%i and i=%i.\n", top[0][0], top[1][0]);
      
    
        //printf("selection done.\n");
    
    
 

    
    
         //printf("proceeding to iteration.\n");
    for (i=0;i<param_aperture;i++) {
        
         replicate_move(&movebuff, &_board->movelist[top[i]]);
         
         asprintf(&nextINDEX, "%s%i", INDEX, i);
         
         
         branch_score[i] =  thinkiterate (_board,1-PL, DEEP-1, score, &movebuff, nextINDEX);
            //if(PL==1)
         //if (i==0) score = score + t;
         
                
        //printf("sent to thinkiterate\n");    
        //rintf("after:\n");
        //show_board(_board->squares);
        //printf("from thinkinterate score: %i\n", t);
        }
    
    

   score = 16900;
   if (PL!=machineplays) score = -16900;
   
   for (i=0;i<param_aperture;i++) {
       
       
    if (PL!=machineplays) if (branch_score[i] > score) {score = branch_score[i];t=i;}
       
   
    
    if (PL==machineplays) if (branch_score[i] < score) {score = branch_score[i];t=i;}
    
    
    }
    
   
   
   
   
   if(show_info) {
       struct move info_move;
       replicate_move(&info_move, &_board->movelist[top[t]]);
       cord2pos(info_move.from);
       cord2pos(info_move.to);

       
       asprintf(&infomoveTABLE[infomoveINDEX], "%s %c%c%c%c", INDEX, 
       info_move.from[0],info_move.from[1],info_move.to[0],info_move.to[1]);
       infomoveINDEX++;
   }
   
   
   
   
     }
    //show_board(_board->squares);

    freeboard(_board);
    free(nextINDEX);
    
 return score;
}

float scoremod (int DEEP, int method) {
    
    float modifier = 0;
    float helper = 0;
    
    if (method>3) method = 0;
    
    
    if (method == 0) modifier = 1;
    
    if (method == 1) modifier = 2*((DEEP+param_deviationcalc)/param_DEEP);
    
    if (method == 2) {
        modifier = -power(DEEP,2)+param_DEEP*DEEP+2*param_DEEP;
        

    helper = param_DEEP/2;
        
            helper = -power(helper,2)+param_DEEP*helper+2*param_DEEP;
            
            modifier = modifier/(helper/1.1);
    }
    
    
    if (method == 3) {

        modifier = param_TIMEweight[param_DEEP-DEEP];
        
    }
    
    
    
    
    return modifier;
}