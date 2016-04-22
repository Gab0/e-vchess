#include "ev_chess.h"

void cord2pos (char out[]) {
    char keymap[8] = {'a','b','c','d','e','f','g','h'};
    int i = out[0];
    int j = out[1];
    
    out[0] = keymap[j];
    out[1] = 8 - i + '0';
}
void pos2cord (char out[]) {
    char keymap[8] = {'a','b','c','d','e','f','g','h'};
   
    char let = out[0];
    char num = out[1];
    
    int i = 0;
    while ( i < 8 && keymap[i] != let ) i++;
    
    int j = num -'0';
    
    out[0] = 8 - j;
    out[1] = i;
}



Host Device bool is_in(char val, char arr[], int size){
    int i = 0;
    for (i=0; i < size; i++) {
        if (arr[i] == val)
            return true;
    }
    return false;
}


Device int append_move(struct board *board, struct movelist *moves, int i,int j, int mod_i, int mod_j, int P) {
    
    //if (board->squares[i+mod_i][j+mod_j] == pieces[1-P][5]) return 0;
     
    moves->movements[moves->k].passant=0;
    moves->movements[moves->k].passantJ[0]=board->passantJ;
    moves->movements[moves->k].passantJ[1]=-1;
    
    moves->movements[moves->k].iscastle = 0;
    
    
        
    if (i>7) {
    //i=16 denotes a castling movement.
    if (i==16) {
        
        int I = 7 * (1-P);
        
        moves->movements[moves->k].from[0] = I;
        moves->movements[moves->k].from[1] = 4;
        
        moves->movements[moves->k].to[0] = I;
        moves->movements[moves->k].to[1] = j;
        
        moves->movements[moves->k].promoteto = 0;
        moves->movements[moves->k].casualty = 'x';
        
        moves->movements[moves->k].iscastle = 1;
        moves->movements[moves->k].lostcastle = 2;
        
        }

        
        //i=33 or 44 denotes an en passant capture.
     if(i==33||i==44){
          moves->movements[moves->k].passant = 1;
            i = i/11;
        }
        
        //i=11 or 66 denotes a double-step pawn movement.
     if(i==11||i==66) { 
            moves->movements[moves->k].passantJ[1]=j;
                i=i/11;
        }
    }
        
        
    
    
    if (i<8) { 
    
    moves->movements[moves->k].from[0] = i;
    moves->movements[moves->k].from[1] = j;
    
    moves->movements[moves->k].to[0]=i+mod_i;
    moves->movements[moves->k].to[1]=j+mod_j;
    
    moves->movements[moves->k].casualty = board->squares[i+mod_i][j+mod_j];
    moves->movements[moves->k].promoteto = 0;
    moves->movements[moves->k].iscastle = 0;
    moves->movements[moves->k].lostcastle = 0;
    
    if (moves->movements[moves->k].passant) 
        moves->movements[moves->k].casualty = Pieces[1-P][0];
    
    
    if ((i==0 && P==1)||(i==7 && P==0)){
        if(j==0 && board->castle[P][0]==1)
            moves->movements[moves->k].lostcastle = 1;
        
        if(j==4 && board->castle[P][1]==1)
            moves->movements[moves->k].lostcastle = 2;
        
        if(j==7 && board->castle[P][2]==1)
            moves->movements[moves->k].lostcastle = 3;
            
    }
        
        
    //printf("testing check.\n");
    //print_play_cord(move);
    
    if (P>3) {printf("fodeumlk\n");}
    
    if (P==3) {moves->movements[moves->k].promoteto = 'q'; P=1;}
    if (P==2) {moves->movements[moves->k].promoteto = 'Q'; P=0;}
    
    
    
    if (check_move_check(board, &moves->movements[moves->k], P)) return 0;      
        
       
  
    }
     
  

    /*printf("appending[k=%i]: ", k);*/
    /*print_movement(k,P);*/
                    moves->k++;
                    return 1;
}





Host Device int ifsquare_attacked (char squares[8][8], int TGi, int TGj, int P, int verbose) {
    //show_board(squares);
    
    int i = 0;
    int j =0;
    int offender = 0;
    int target[2] = {TGi, TGj};
    
    int z=0;
    int n=0;
    
    long result = 0;
    
    int aim_x = 0;
    int aim_y =0;    
    
    int matrix[10][2]= {{0,0},{-1,-1},{-1,0},{-1,1},{0,-1},{0,0},{0,1},{1,-1},{1,0},{1,1}};
    int horse_matrix[2][2] = {{-1,1},{-2,2}};    
    
    //printf("g.\n");
    
    for(z=0;z<10;z++) {
        if (z==0||z==5) continue;
        i=1;
        j=1;
        
        aim_y=target[0]+i*matrix[z][0];
        aim_x=target[1]+j*matrix[z][1];
        
        
        
        while (onboard(aim_y,aim_x) && squares[aim_y][aim_x] == 'x')
     {i++; aim_y=target[0]+i*matrix[z][0]; j++; aim_x=target[1]+j*matrix[z][1];}
        
        Vb printf("checking %i%i\n", aim_y,aim_x);

        
        if (onboard(aim_y,aim_x) && is_in(squares[aim_y][aim_x], Pieces[1-P],6)) {
            offender = getindex(squares[aim_y][aim_x], Pieces[1-P],6);
            Vb printf("y");
            //printf("kpos %i%i x=%i\n",aim_y,aim_x,x);
            
            
            if (offender==1||offender==4) if (z==2||z==4||z==6||z==8) result++; 
                    
            if (offender==3||offender==4) if (z==1||z==3|z==7||z==9) result++;
            
            if (offender==5) if (i==1) result++;
            
            if (offender==0) if (i==1) if (P==0) if (z==1||z==3)  result++;
            
            if (offender==0) if (i==1) if (P==1) if (z==7||z==9) result++;
              

            

        } }
    
    
    //printf("b.\n");
    
    
    for(z=0;z<2;z++) for(n=0;n<2;n++) {
    

        
        aim_y=target[0]+horse_matrix[1][n];
        aim_x=target[1]+horse_matrix[0][z];
        
        if ((onboard(aim_y,aim_x)) && (squares[aim_y][aim_x] == Pieces[1-P][2])) result++;
             
  
        aim_y=target[0]+horse_matrix[0][n];
        aim_x=target[1]+horse_matrix[1][z];
                
        if ((onboard(aim_y,aim_x)) && (squares[aim_y][aim_x] == Pieces[1-P][2])) result++;
        


  
    }         
            
            
            
            
          
    
    //printf("%i.\n",result);
    
    return result;
}

Host Device int check_move_check (struct board *tg_board, struct move *move, int P) {
    

    int kpos[2];

    int check=-1;
    
    int i=0;
    int j=0;
    
    int verbose = 0;
    //if (movement[1][0] == 5 && movement[1][1]==7) verbose = 1;
    
    
    
    move_pc(tg_board, move);
    
    for (i=0;i<8;i++) for (j=0;j<8;j++){
         
            if (tg_board->squares[i][j] == Pieces[P][5]) {kpos[0]=i; kpos[1]=j;check++;}
            
        }
    
    
    //printf("checking check kpos= %i%i\n", kpos[0],kpos[1]);
    
    
    if (check==-1) {
        //printf("er-r [king not found].\n"); show_board(tg_board->squares);
        undo_move(tg_board, move);return 0;}
    
    if (ifsquare_attacked(tg_board->squares, kpos[0],kpos[1], P, verbose)>0)check=1;
    
    
        


    //printf("check done.\n");
    undo_move(tg_board, move);
    return check;
}


Host Device int getindex (char x, char array[],int size) {
    int i=0;
    for (i=0;i<size;i++) {
        if (array[i] == x) return i;
        
    }
    return -1;
}

Host Device struct board *makeparallelboard (struct board *model) {
    int i=0, j=0;

    struct board *_board = (struct board *)malloc(sizeof (struct board));
    
    forsquares
            _board->squares[i][j] = model->squares[i][j];
            
            
    _board->passantJ = model->passantJ;
    
    
    for(i=0;i<2;i++) for(j=0;j<3;j++) _board->castle[i][j] = model->castle[i][j];
    
    return _board;
}

//LEGACY FUNCTION
void freeboard (struct board *target) {
    
    if (target != NULL) {

    free (target);
    
    }
}

Host Device void select_top (struct move *array, int size, int target[], int quant) {
    int i = 0;
    int qu=0;
    int win[16][2]={0};
    char forbid[16]={};
    int f_index=0;
    
    
    if (quant < 0) {
        quant = -quant;
    
    for (qu=0;qu<quant;qu++) {
        win[qu][1] = 16700;
        
    for (i=0;i<size;i++) {
        if (!is_in(i,forbid,f_index+1)) {
        

        if (array[i].score < win[qu][1]){ 
            win[qu][1] = array[i].score;
            win[qu][0] = i;
            forbid[f_index] = i;  
            
        }       
        
        }
        
        
    }
        f_index++;
    }
    

        for (i=0;i<quant;i++) {
        target[i] = win[i][0];
        //target[i][1] = win[i][1];
                

    }
                
        
        
    }
    
    else {
    for (qu=0;qu<quant;qu++) {
        win[qu][1] = -16700;
        
    for (i=0;i<size;i++) {
        if (!is_in(i,forbid,f_index+1)) {
            
        

        if (array[i].score > win[qu][1]){ 
            win[qu][1] = array[i].score;
            win[qu][0] = i;
            forbid[f_index] = i;  
            
        }       
        
        }
        
        
    }
        f_index++;
    }
    

        for (i=0;i<quant;i++) {
        target[i] = win[i][0];
        //target[i][1] = win[i][1];
                

    }
        
    }
               
    
}

Host Device void replicate_move(struct move *target, struct move *source) {
    target->from[0] = source->from[0];
    target->from[1] = source->from[1];

    target->to[0] = source->to[0];
    target->to[1] = source->to[1];

        target->casualty = source->casualty;
        target->promoteto = source->promoteto;
        
        target->iscastle = source->iscastle;
        target->lostcastle = source->lostcastle;
        
        target->passant = source->passant;
        target->passantJ[0] = source->passantJ[0];
        target->passantJ[1] = source->passantJ[1];
        
        target->score = 0;
}

Host Device int power(int base, unsigned int exp) {
    int i, result = 1;
    for (i = 0; i < exp; i++)
        result *= base;
    return result;
 }

Host Device void reorder_movelist(struct movelist *movelist) {
    int i=0;
    struct move temp;
    
    int Freeindex[128];
    int bFK=0, tFK=0;
    

    
    for (i=1;i<movelist->k;i++) {
        if (movelist->movements[i].casualty == 'x') {
            Freeindex[tFK] = i;
            tFK++;
            }
        
        else if (tFK>bFK) {
            replicate_move(&temp, &movelist->movements[Freeindex[bFK]]);
            //temp = movelist->movements[Freeindex[bFK]];
            //movelist->movements[Freeindex[bFK]] = movelist->movements[i];
            replicate_move(&movelist->movements[Freeindex[bFK]], &movelist->movements[i]);;
            
            replicate_move(&movelist->movements[i],&temp);
            //movelist->movements[i] = temp;
            bFK++;
        }

        
        
    }
}