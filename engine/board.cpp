/* 
 * File:   main.cpp
 * Author: gabs
 *
 * Created on September 20, 2015, 11:29 PM
 */



#include "ev_chess.h"

using namespace std;


void setup_board (int setup) {
    int i;
    
   if (setup) { 
    	for(i = 0; i < 8; i++) {
            board.squares[2][i] = 'x';	/* empty squares */
            board.squares[3][i] = 'x';
            board.squares[4][i] = 'x';
            board.squares[5][i] = 'x';
	
	
	/* setup pawns */
            board.squares[1][i] = 'p';	/* white pawn */
            board.squares[6][i] = 'P';	/* black pawn */
	}
	
	board.squares[0][0] = board.squares[0][7] = 'r';	/* black rooks */
	board.squares[7][0] = board.squares[7][7] = 'R';	/* white rooks */
	board.squares[0][1] = board.squares[0][6] = 'n';	/* black knights */
	board.squares[7][1] = board.squares[7][6] = 'N';	/* white knights */
	board.squares[0][2] = board.squares[0][5] = 'b';	/* black bishops */
	board.squares[7][2] = board.squares[7][5] = 'B';	/* white bishops */
	board.squares[0][3] = 'q';								/* white queen */
	board.squares[7][3] = 'Q';								/* black queen */
	board.squares[0][4] = 'k';								/* white king */
	board.squares[7][4] = 'K';								/* black king */
	
        for(i=0;i<3;i++) {board.castle[0][i]=1; board.castle[1][i]=1;}
        board.whoplays = 0;
   }
    
   else {
       int j;
       for (i=0;i<8;i++) 
           for (j=0;j<8;j++) 
               board.squares[i][j] = 'x';
           
       
   }
    
    board.passantJ=-1;
    //erase_moves(&board, 1);
    }

void show_board (char squares[8][8]) {
    int i,j;

    for(i=0;i<8;i++){
        for(j=0;j<8;j++)
            printf("%c ", squares[i][j]);
      
        printf("\n");}
    //if (full){
   //     printf("")
   // }
    
    
    }

Host Device int legal_moves (struct board *board, struct movelist *moves, int PL, int verbose) {
    
    moves->k=0;
    moves->kad=0;

    //NullMove.
    //append_move(board, moves,9,6,6,6,PL);
    
    
    int EP = 1-PL;
       
  
        int i = 0;
        int j = 0;
        
        int zeta = 0;

        int pawn_vector = -1;
        pawn_vector += 2*PL;
        
 
        int promote=0;
        if (PL==1) promote=1;
        
        
        /*printf("starting move findings. PL=%i. pawn_vector=%i\n", PL,pawn_vector);*/
        
        
        forsquares{
            if (!is_in(board->squares[i][j], Pieces[PL],6)) continue;
            
            if (board->squares[i][j] == Pieces[PL][0]) {

                if (i==6&&PL==1) {promote++; promote++;}
                if (i==1&&PL==0) {promote++; promote++;}
                
                if (board->passantJ==j+1||board->passantJ==j-1)
                    if (board->passantJ>-1)
                        if ((i==3&&!PL)||(i==4&&PL)) 
                            if (board->squares[i][board->passantJ]==Pieces[1-PL][0]) 
                                append_move(board, moves, i*11,j,pawn_vector,board->passantJ-j,PL);
                        
                
                if ((is_in(board->squares[i+pawn_vector][j+1], Pieces[EP],6)) && onboard(i+pawn_vector,j+1)) 
              
                    append_move(board, moves,i,j,pawn_vector,1,promote);
                    
                
                if ((is_in(board->squares[i+pawn_vector][j-1], Pieces[EP],6)) && onboard(i+pawn_vector,j-1))
                    
                    append_move(board,moves,i,j,pawn_vector,-1,promote);
                                       
                
                        
                if (board->squares[i+pawn_vector][j] == 'x' && onboard(i+pawn_vector,j)) 

                    append_move(board,moves,i,j,pawn_vector,0,promote);
                                          
                
                if (promote > 1) { promote--;promote--;}

                if (board->squares[i+(2*pawn_vector)][j] == 'x' && i == 6-5*PL && onboard(i+(2*pawn_vector),j))
                        if(board->squares[i+pawn_vector][j]=='x') append_move(board,moves,i*11,j,4*PL-2,0,PL);
                          
               
            
            } 
            
  /*tower movements.*/          
            if (board->squares[i][j] == Pieces[PL][1]) {
               /* printf("scanning tower moves\n");*/

        movement_generator(board,moves,0, '+', i, j, PL);
            }
            
            
      /*"horse" movements. note -(k_atk-2) is a mathematical expression to equal 0 if k_atk=2, or 1 if k_atk=1 */      
     if(board->squares[i][j] == Pieces[PL][2]){
        /* printf("scanning horse moves\n");*/
         zeta = 0;
         int HT[4][2] = {{1,2},{-1,2},{1,-2},{-1,-2}};
         int k_atk = 0;
         for (zeta=0; zeta < 4; zeta++){
             
     k_atk = mpc(board->squares, i+HT[zeta][0],j+HT[zeta][1],PL);       
     if (k_atk) append_move(board,moves,i,j,HT[zeta][0],HT[zeta][1],PL);
        
    
     k_atk = mpc(board->squares, i+HT[zeta][1],j+HT[zeta][0],PL);
     if (k_atk) append_move(board,moves,i,j,HT[zeta][1],HT[zeta][0],PL);
    
         }
         }
         
     /*bishop movements*/    
     if(board->squares[i][j] == Pieces[PL][3]){
        /* printf("scanning bishop moves\n");*/

         movement_generator(board,moves,0, 'X', i, j, PL);
                
                
     }
 if (board->squares[i][j] == Pieces[PL][4]) {
     /*printf("scanning queen moves.\n"); */          

     movement_generator(board,moves,0, '+', i, j, PL);
     movement_generator(board,moves,0, 'X', i, j, PL);
           
            }
            
if (board->squares[i][j] == Pieces[PL][5]){
    /*printf("scanning king moves.\n");*/
    
     movement_generator(board,moves,1, '+', i, j, PL);
     movement_generator(board,moves,1, 'X', i, j, PL);
    
    if (board->castle[PL][1]==1 && !ifsquare_attacked(board->squares, i, j, PL, 0)) {

        if (cancastle(board, PL,-1)) append_move(board,moves, 16, 2, 0, 0, PL);
        
        if (cancastle(board, PL, 1)) append_move(board,moves, 16, 6, 0, 0, PL);
       
    }

        }
         
        }
        

         
        //board->mobility[PL] = board->k;

        return 0;
        }

Host Device int mpc(char squares[8][8], int i, int j, int player) {

    int enemy = 1 - player;

    
    if (onboard(i,j)) {
    if (squares[i][j] == 'x') {/*printf("mpc granted %i%i\n", i,j); */return 2;}
    
    if (is_in(squares[i][j], Pieces[enemy],6))
    {/*printf("mpc granted %i%i\n", i,j); */return 1;}
    
    else return 0;
   
    }
    else return 0;
}

Host Device void move_pc(struct board *tg_board, struct move *movement) {
    

    
    char from[2] = {movement->from[0], movement->from[1]};
    char to[2] = {movement->to[0], movement->to[1]};
    
    
    



    tg_board->squares[to[0]][to[1]] = tg_board->squares[from[0]][from[1]];
    tg_board->squares[from[0]][from[1]] = 'x';
    
    if(movement->promoteto!=0) tg_board->squares[to[0]][to[1]] = movement->promoteto;
    
    if(movement->iscastle){
        if (movement->to[1] < 4) {
            tg_board->squares[to[0]][to[1]+1] = tg_board->squares[to[0]][0];
            tg_board->squares[to[0]][0] = 'x';
        }
        
        if (movement->to[1] > 4) {
            tg_board->squares[to[0]][to[1]-1] = tg_board->squares[to[0]][7];
            tg_board->squares[to[0]][7] = 'x';
        }        
    }
  
    if(movement->passant) tg_board->squares[from[0]][to[1]] = 'x';
 
    
    int cP =-1;

    if(from[0]==7) cP = 0;
    if(from[0]==0) cP = 1;
    
    
    if (movement->lostcastle==1) tg_board->castle[cP][0]=0;
    if (movement->lostcastle==2) tg_board->castle[cP][1]=0;
    if (movement->lostcastle==3) tg_board->castle[cP][2]=0;
        

    tg_board->passantJ=movement->passantJ[1];

    tg_board->whoplays = 1 - tg_board->whoplays;
    
}

Host Device void undo_move(struct board *tg_board, struct move *movement) {
    
    
    char to[2] = {movement->from[0], movement->from[1]};
    char from[2] = {movement->to[0], movement->to[1]};
    

     
    if(movement->casualty==0) movement->casualty='x';
    
    
    tg_board->squares[to[0]][to[1]] = tg_board->squares[from[0]][from[1]];
    tg_board->squares[from[0]][from[1]] = movement->casualty;
 
    

    
    if(movement->promoteto!=0) {
        if (movement->from[0]==6) tg_board->squares[to[0]][to[1]] = 'p';
        if (movement->from[0]==1) tg_board->squares[to[0]][to[1]] = 'P';
           
    }
    
    if(movement->iscastle){
        if (movement->from[1] < 4) {
            tg_board->squares[from[0]][0] = tg_board->squares[to[0]][from[1]+1];
            tg_board->squares[from[0]][from[1]+1] = 'x';
        }
        
        if (movement->from[1] > 4) {
            tg_board->squares[from[0]][7] = tg_board->squares[to[0]][from[1]-1];
            tg_board->squares[from[0]][from[1]-1] = 'x';
        } 
}
    
    if(movement->passant) {
        tg_board->squares[to[0]][from[1]] = movement->casualty;
        tg_board->squares[from[0]][from[1]] = 'x';
    }
    
    int cP=-1;
    if(to[0]==7) cP = 0;
    if(to[0]==0) cP = 1;
    
    if (cP > -1){
        if(movement->lostcastle==1) tg_board->castle[cP][0] = 1;
        if(movement->lostcastle==2) tg_board->castle[cP][1] = 1;
        if(movement->lostcastle==3) tg_board->castle[cP][2] = 1;
       
        }
    
    tg_board->passantJ=movement->passantJ[0];
    
    tg_board->whoplays = 1 - tg_board->whoplays;    
    
}


Device void attackers_defenders (char squares[8][8], struct movelist moves, int P) {
    int i = 0;
    moves.kad = 0;
    char Attacker = 0;
    
    
    for (i=0;i<moves.k;i++) {

        if (moves.movements[i].casualty != 'x') {

        /*print_movement(k);*/
        Attacker = 
        squares[moves.movements[i].from[0]][moves.movements[i].from[1]];
        moves.attackers[moves.k][0] = Attacker;
        moves.attackers[moves.k][1] = moves.movements[i].from[0];
        moves.attackers[moves.k][2] = moves.movements[i].from[1];
    
        moves.defenders[moves.k][0] = moves.movements[i].casualty;
        moves.defenders[moves.k][1] = moves.movements[i].to[0];
        moves.defenders[moves.k][2] = moves.movements[i].to[1];
        
        //char buf[2] = {defenders[kad][1], defenders[kad][2]};
        //cord2pos(buf);
        /*printf("defender: %c at %c%c.\n",defenders[kad][0],buf[0],buf[1]);*/
        moves.kad++;
        }
    } 
    
}

int history_append(struct move *move) {
    int i=0,j=0;
    
    replicate_move(&movehistory[hindex], move);
    
    for (i=0;i<8;i++) 
        for (j=0;j<8;j++) 
            movehistoryboard[hindex][i][j] = board.squares[i][j];
                    
        hindex++;
        return 0;
    }
    



int history_rollback(int times) {
    int i=0;
    
    for (i=0;i<times;i++) {
    hindex--;
    print_play_cord(movehistory[hindex]);
    undo_move(&board, &movehistory[hindex]);
    
    }
   return 0; 
}



Host Device int findking (char board[8][8], char YorX, int player) {
    char KING = Pieces[player][5];
 
    int i=0;
    int j=0;
 

    
    for (i=0;i<8;i++) for (j=0;j<8;j++) {
        if (board[i][j] == KING) {
            if (YorX == 'Y') return i;
            if (YorX == 'X') return j;

            
            
        }
    }
    return -1;
}

Host Device int cancastle (struct board *board, int P, int direction) {
    if (!board->castle[P][1]) return 0;
    if (!allow_castling) return 0;
    int ROW = 7*(1-P);
    

    
    if (board->castle[P][0] && direction==-1) {
    if (board->squares[ROW][0] == Pieces[P][1]  &&
        board->squares[ROW][1]=='x' && !ifsquare_attacked(board->squares,ROW,1,P,0) &&
        board->squares[ROW][2]=='x' && !ifsquare_attacked(board->squares,ROW,2,P,0) &&
        board->squares[ROW][3]=='x' && !ifsquare_attacked(board->squares,ROW,3,P,0)) {
        
        
        return 1;
        
        
        
    }}
           
            
            
    
    
    
    if (board->castle[P][2] && direction==1) {
    if (board->squares[ROW][7] == Pieces[P][1] &&
        board->squares[ROW][5]=='x' && !ifsquare_attacked(board->squares,ROW,5,P,0) &&
        board->squares[ROW][6]=='x' && !ifsquare_attacked(board->squares,ROW,6,P,0)){
        
        return 1;
    }}
        
    
   return 0;     
 
}

Host Device void movement_generator(struct board *board, struct movelist *moves, int limit, 
                        char direction, int i, int j, int P) {
    int X=0, q=0;
    int Ti=0,Tj=0;
    
    int matrix[4][2];
    
    if (direction=='X'){
        matrix[0][0] = 1;
        matrix[0][1] = -1;
        matrix[1][0] = -1;
        matrix[1][1] = 1;
        matrix[2][0] = 1;
        matrix[2][1] = 1;
        matrix[3][0] = -1;
        matrix[3][1] = -1;
    }
       
    if (direction=='+') {
        matrix[0][0] = 1;
        matrix[0][1] = 0;
        matrix[1][0] = -1;
        matrix[1][1] = 0;
        matrix[2][0] = 0;
        matrix[2][1] = 1;
        matrix[3][0] = 0;
        matrix[3][1] = -1;
    }       
    
    
    
       for (X=0;X<4;X++){
           q=1;
           while (q>0) {
           Ti = i+matrix[X][0]*q;
           Tj = j+matrix[X][1]*q;
           
           
           
           if (onboard(Ti,Tj)) {
               if (board->squares[Ti][Tj]=='x') {
                   append_move(board, moves, i, j, Ti-i, Tj-j, P);
                   q++;
                   if (limit) q=0;

               }
               if (is_in(board->squares[Ti][Tj], Pieces[1-P],6)) {
                   append_move(board, moves, i, j, Ti-i, Tj-j, P);
                   q=0;
               }
               
               if (is_in(board->squares[Ti][Tj], Pieces[P],6)) q=0;
               
               
           }
           else q=0;
           } 
      }
    
    
}
