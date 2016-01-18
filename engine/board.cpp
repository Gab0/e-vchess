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
        
	/*board->ep_file = 0;
	
	board->white_oo = board->white_ooo = 1;
	board->black_oo = board->black_ooo = 1;
    */
   }
    
   else {
       int j;
       for (i=0;i>8;i++) {
           for (j=0;j>8;j++) {
               board.squares[i][j] = 'x';
           } 
       }
   }
    
    
    erase_moves(&board, 1);
}

void show_board (char squares[8][8]) {
    int i,j;

    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            printf("%c ", squares[i][j]);
        }
        printf("\n");
    }

}

int legal_moves (struct board *board, int PL, int verbose) {
    erase_moves(board,0);
    board->k=0;

    int EP = 1-PL;
       
  
        int i = 0;
        int j = 0;
        
        int m_i = 0;
        int m_j = 0;
        
        int zeta = 0;

        int pawn_vector = -1;
        pawn_vector += 2*PL;
        
 
        int promote=0;
        if (PL==1) promote=1;
        
        
        /*printf("starting move findings. PL=%i. pawn_vector=%i\n", PL,pawn_vector);*/
        for (i=0;i < 8; i++){
        for (j=0;j < 8; j++){
            /*printf(">%i %i\n",i,j);*/
            
            
            if (board->squares[i][j] == pieces[PL][0]) {

                if (i==6&&PL==1) {promote++; promote++;}
                if (i==1&&PL==0) {promote++; promote++;}
                
                
                
                if (is_in(board->squares[i+pawn_vector][j+1],pieces[EP],6) && onboard(i+pawn_vector,j+1)) 
              
                    append_move(board,i,j,pawn_vector,1,promote);
                    
                
                if (is_in(board->squares[i+pawn_vector][j-1],pieces[EP],6) && onboard(i+pawn_vector,j-1))
                    
                    append_move(board,i,j,pawn_vector,-1,promote);
                                       
                
                        
                if (board->squares[i+pawn_vector][j] == 'x' && onboard(i+pawn_vector,j)) 

                    append_move(board,i,j,pawn_vector,0,promote);
                                          
                
                if (promote > 1) { promote--;promote--;}
            
                
                
                
                

                if (board->squares[i+(2*pawn_vector)][j] == 'x' && i == (6-(5*PL)) && onboard(i+(2*pawn_vector),j)){
                    //*printf("double pawn movement added.\n");*/
                    if(board->squares[i+pawn_vector][j]=='x') append_move(board,i,j,-2+PL+PL+PL+PL,0,PL);
                          
               }
            
            } 
            
  /*tower movements.*/          
            if (board->squares[i][j] == pieces[PL][1]) {
               /* printf("scanning tower moves\n");*/

  /*movement in one direction, of four*/
                m_i = 1;
                m_j = 1;

while (mpc(board->squares, i+m_i,j,PL)) {
    if (mpc(board->squares, i+m_i,j,PL) == 1) {
        append_move(board,i,j,m_i,0,PL);
        
        break;}    
    append_move(board,i,j,m_i,0,PL);
    
    m_i++;


    }
                
  /*movement in one direction, of four*/
                m_i = 1;
                m_j = 1;

while (mpc(board->squares, i-m_i,j,PL)) {
    if (mpc(board->squares, i-m_i,j,PL) == 1) {
        append_move(board,i,j,-m_i,0,PL);
        
        break;}
    append_move(board,i,j,-m_i,0,PL);
    
    m_i++;


    }

  /*movement in one direction, of four*/
                m_i = 1;
                m_j = 1;

while (mpc(board->squares, i,j+m_j,PL)) {
    if (mpc(board->squares, i,j+m_j,PL) == 1) {
        append_move(board,i,j,0,m_j,PL);
        
        break;}
    append_move(board,i,j,0,m_j,PL);
    
    m_j++;


    }

  /*movement in one direction, of four*/
                m_i = 1;
                m_j = 1;

while (mpc(board->squares, i,j-m_j,PL)) {
    if (mpc(board->squares, i,j-m_j,PL) == 1) {
        append_move(board,i,j,0,-m_j,PL);
        
        break;}
    append_move(board,i,j,0,-m_j,PL);
    
    m_j++;


    }
            }
            
            
      /*"horse" movements. note -(k_atk-2) is a mathematical expression to equal 0 if k_atk=2, or 1 if k_atk=1 */      
     if(board->squares[i][j] == pieces[PL][2]){
        /* printf("scanning horse moves\n");*/
         zeta = 0;
         int HT[4][2] = {{1,2},{-1,2},{1,-2},{-1,-2}};
         int k_atk = 0;
         for (zeta=0; zeta < 4; zeta++){
             
     k_atk = mpc(board->squares, i+HT[zeta][0],j+HT[zeta][1],PL);       
     if (k_atk) append_move(board,i,j,HT[zeta][0],HT[zeta][1],PL);
        
    
     k_atk = mpc(board->squares, i+HT[zeta][1],j+HT[zeta][0],PL);
     if (k_atk) append_move(board,i,j,HT[zeta][1],HT[zeta][0],PL);
    
         }
         }
         
     /*bishop movements*/    
     if(board->squares[i][j] == pieces[PL][3]){
        /* printf("scanning bishop moves\n");*/
  /*movement in one direction, of four*/
                m_i = 1;
                m_j = 1;
while (mpc(board->squares, i+m_i,j+m_j,PL)) {
       if (mpc(board->squares, i+m_i,j+m_j,PL) == 1){
           append_move(board,i,j,m_i,m_j,PL);
           
           break;}   
    append_move(board,i,j,m_i,m_j,PL);

    
    m_i++;
    m_j++;
      /* printf("found!\n\n");
       print_play(mlist[k-1]);   
       printf("%i%i %i%i\n", i,j,i+m_i,j-m_j); 
       */

    }         

  /*movement in one direction, of four*/
                m_i = 1;
                m_j = 1;

while (mpc(board->squares, i+m_i,j-m_j,PL)) {
       if (mpc(board->squares, i+m_i,j-m_j,PL) == 1){
           append_move(board,i,j,m_i,-m_j,PL);
           
           break;}
    append_move(board,i,j,m_i,-m_j,PL);

    
    m_i++;
    m_j++;
      /* printf("found!\n\n");
       print_play(mlist[k-1]);   
       printf("%i%i %i%i\n", i,j,i+m_i,j-m_j); 
       */

    }                
         
  /*movement in one direction, of four*/
                m_i = 1;
                m_j = 1;

while (mpc(board->squares, i-m_i,j-m_j,PL)) {
       if (mpc(board->squares, i-m_i,j-m_j,PL) == 1){
           append_move(board,i,j,-m_i,-m_j,PL);
           
           break;} 
    append_move(board,i,j,-m_i,-m_j,PL);
    
    m_i++;
    m_j++;
    

    }         
         
  /*movement in one direction, of four*/
                m_i = 1;
                m_j = 1;

while (mpc(board->squares, i-m_i,j+m_j,PL)) {
       if (mpc(board->squares, i-m_i,j+m_j,PL) == 1){
           append_move(board,i,j,-m_i,m_j,PL);
           
           break;}
    append_move(board,i,j,-m_i,m_j,PL);
    
    m_i++;
    m_j++;
    

    }  
                
                
     }
 if (board->squares[i][j] == pieces[PL][4]) {
     /*printf("scanning queen moves.\n"); */          
  /*movement in one direction, of four*/
                m_i = 1;
                m_j = 1;

while (mpc(board->squares, i+m_i,j,PL)) {
    if (mpc(board->squares, i+m_i,j,PL) == 1) {
        append_move(board,i,j,m_i,0,PL);
        
        break;}    
    append_move(board,i,j,m_i,0,PL);
    
    m_i++;


    }
                
  /*movement in one direction, of four*/
                m_i = 1;
                m_j = 1;

while (mpc(board->squares, i-m_i,j,PL)) {
    if (mpc(board->squares, i-m_i,j,PL) == 1) {
        append_move(board,i,j,-m_i,0,PL);
        
        break;}
    append_move(board,i,j,-m_i,0,PL);
    
    m_i++;


    }

  /*movement in one direction, of four*/
                m_i = 1;
                m_j = 1;

while (mpc(board->squares, i,j+m_j,PL)) {
    if (mpc(board->squares, i,j+m_j,PL) == 1) {
        append_move(board,i,j,0,m_j,PL);
        
        break;}
    append_move(board,i,j,0,m_j,PL);
    
    m_j++;


    }

  /*movement in one direction, of four*/
                m_i = 1;
                m_j = 1;

while (mpc(board->squares, i,j-m_j,PL)) {
    if (mpc(board->squares, i,j-m_j,PL) == 1) {
        append_move(board,i,j,0,-m_j,PL);
        
        break;} 
    append_move(board,i,j,0,-m_j,PL);
    
    m_j++;

}

   /*movement in one direction, of four*/
                m_i = 1;
                m_j = 1;
while (mpc(board->squares, i+m_i,j+m_j,PL)) {
       if (mpc(board->squares, i+m_i,j+m_j,PL) == 1){
           append_move(board,i,j,m_i,m_j,PL);
           
           break;}   
    append_move(board,i,j,m_i,m_j,PL);

    
    m_i++;
    m_j++;
      /* printf("found!\n\n");
       print_play(mlist[k-1]);   
       printf("%i%i %i%i\n", i,j,i+m_i,j-m_j); 
       */

    }         

  /*movement in one direction, of four*/
                m_i = 1;
                m_j = 1;

while (mpc(board->squares, i+m_i,j-m_j,PL)) {
       if (mpc(board->squares, i+m_i,j-m_j,PL) == 1){
           append_move(board,i,j,m_i,-m_j,PL);
           
           break;}
    append_move(board,i,j,m_i,-m_j,PL);

    
    m_i++;
    m_j++;
      /* printf("found!\n\n");
       print_play(mlist[k-1]);   
       printf("%i%i %i%i\n", i,j,i+m_i,j-m_j); 
       */

    }                
         
  /*movement in one direction, of four*/
                m_i = 1;
                m_j = 1;

while (mpc(board->squares, i-m_i,j-m_j,PL)) {
    
       if (mpc(board->squares, i-m_i,j-m_j,PL) == 1){
           append_move(board,i,j,-m_i,-m_j,PL);
           
           break;} 
    append_move(board,i,j,-m_i,-m_j,PL);
    
    m_i++;
    m_j++;
    

    }         
         
  /*movement in one direction, of four*/
                m_i = 1;
                m_j = 1;

while (mpc(board->squares, i-m_i,j+m_j,PL)) {

          if (mpc(board->squares, i-m_i,j+m_j,PL) == 1){
           append_move(board,i,j,-m_i,m_j,PL);
           
           break;} 
    
    
    append_move(board,i,j,-m_i,m_j,PL);
    
    m_i++;
    m_j++;
    

    
    }  
           
            }
            
if (board->squares[i][j] == pieces[PL][5]){
    /*printf("scanning king moves.\n");*/
    m_i=-1;
    m_j=-1;
    
    for (m_i = -1; m_i < 2; m_i++){
        for (m_j = -1; m_j < 2; m_j++) {
            
            if (m_i||m_j) if (mpc(board->squares, i+m_i,j+m_j,PL)>0) append_move(board,i,j,m_i,m_j,PL);
            //if (mpc(board->squares, i+m_i,j+m_j,PL)==2) append_move(board,i,j,m_i,m_j,PL);   

            }
        }
    
    if (board->castle[PL][1]==1 && !ifsquare_attacked(board, i, j, PL, 0)) {

        if (cancastle(board, PL,-1)) append_move(board, 16, 2, 0, 0, PL);
        
        if (cancastle(board, PL, 1)) append_move(board, 16, 6, 0, 0, PL);
                
       
    }
    
    

    
    
      
    
    

        }
         
        }
        }

         

        attackers_defenders(board, PL);    
        return 0;
        }

int mpc(char squares[8][8], int i, int j, int player) {

    int enemy = 1 - player;

    
    if (onboard(i,j)) {
    if (squares[i][j] == 'x') {/*printf("mpc granted %i%i\n", i,j); */return 2;}
    
    if (is_in(squares[i][j],pieces[enemy],6)) {/*printf("mpc granted %i%i\n", i,j); */return 1;}
    
    
    
    
    
    else return 0;
   
    }
    else return 0;
}

void move_pc(struct board *tg_board, struct move *movement) {

    char from[2] = {movement->from[0], movement->from[1]};
    char to[2] = {movement->to[0], movement->to[1]};
    

    
    int i=0;
    
    
    
    /*pos2cord(from, movement[0][0],movement[0][1]);
    pos2cord(to, movement[1][0], movement[1][1]);
    */
    
    /*print_play(movement);*/
    
    /*printf("%i %i to %i %i\n", from[0],from[1],to[0],to[1]);*/
    //printf("moving %s.\n", tg_board->squares[from[0]][from[1]]);
    
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
        
    int cP =-1;

    if(from[0]==7) cP = 0;
    if(from[0]==0) cP = 1;
    
    if (cP > -1){
        if (tg_board->squares[to[0]][to[1]] == pieces[cP][5]) {
        if(from[1]==0 && tg_board->castle[cP][0] == 1) {tg_board->castle[cP][0] = 0; movement->lostcastle=1;}
        if(from[1]==4 && tg_board->castle[cP][1] == 1) {tg_board->castle[cP][1] = 0; movement->lostcastle=2;}
        if(from[1]==7 && tg_board->castle[cP][2] == 1) {tg_board->castle[cP][2] = 0; movement->lostcastle=3;}
        }
        }
    
}

void undo_move(struct board *tg_board, struct move *movement) {
    
    char to[2] = {movement->from[0], movement->from[1]};
    char from[2] = {movement->to[0], movement->to[1]};
    
    
      if (to[0] == 'O') {
          castle(tg_board, 0, from[0], from[1]); 
          tg_board->castle[from[0]][1]=1;
          return;}  
  
    tg_board->squares[to[0]][to[1]] = tg_board->squares[from[0]][from[1]];
    tg_board->squares[from[0]][from[1]] = movement->casualty;
 
    if(movement->casualty==0) printf("busted!.\n");

    
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
    
    int cP=-1;
    if(to[0]==7) cP = 0;
    if(to[0]==0) cP = 1;
    
    if (cP > -1){
        if(to[1]==0 && movement->lostcastle==1) tg_board->castle[cP][0] = 1;
        if(to[1]==4 && movement->lostcastle==2) tg_board->castle[cP][1] = 1;
        if(to[1]==7 && movement->lostcastle==3) tg_board->castle[cP][2] = 1;
       
        }
    
    
    
    
}


void attackers_defenders (struct board *board, int P) {
    int i = 0;
    board->kad = 0;
    
    
    
    for (i=0;i<board->k;i++) {

        if (board->movelist[i].casualty != 'x') {

        /*print_movement(k);*/
        board->attackers[board->kad][0] = board->movelist[i].from[0];
        board->attackers[board->kad][1] = board->movelist[i].from[1];
    
        board->defenders[board->kad][0] = board->movelist[i].casualty;
        board->defenders[board->kad][1] = board->movelist[i].to[0];
        board->defenders[board->kad][2] = board->movelist[i].to[1];
        
        //char buf[2] = {defenders[kad][1], defenders[kad][2]};
        //cord2pos(buf);
        /*printf("defender: %c at %c%c.\n",defenders[kad][0],buf[0],buf[1]);*/
        board->kad++;
        }
    } 
    
}

int history_append(struct move *move) {
    
    
    replicate_move(&board.movehistory[board.hindex], move);

        board.hindex++;
        return 0;
    }
    



int history_rollback(int times) {
    int i=0;
    
    for (i=0;i<times;i++) {
    board.hindex--;
    print_play_cord(board.movehistory[board.hindex]);
    undo_move(&board, &board.movehistory[board.hindex]);
    
    }
   return 0; 
}

//LEGACY FUNCTION.
void castle (struct board *board, int doundo, int PL, int side) {
    int i = 0;
    int j = 0;
    
    if (PL == 0) i=7;
   
   
    if (doundo) {
        board->squares[i][4]='x';
        board->squares[i][j]='x';
        if(side) {
            board->squares[i][6]=pieces[PL][5];
            board->squares[i][5]=pieces[PL][1];}
        else {
            board->squares[i][2]=pieces[PL][5];
            board->squares[i][3]=pieces[PL][1];}
    }
    
    else {
        board->squares[i][4]=pieces[PL][5];
        board->squares[i][j]=pieces[PL][1];;
        if(side) {
            board->squares[i][6]='x';
            board->squares[i][5]='x';}
        else {
            board->squares[i][2]='x';
            board->squares[i][3]='x';}        
    }
}


int findking (char board[8][8], char YorX, int player) {
    char KING = pieces[player][5];
 
    int i=0;
    int j=0;
 

    
    for (i=0;i<8;i++) for (j=0;j<8;j++) {
        if (board[i][j] == KING) {
            if (YorX == 'Y') return i;
            if (YorX == 'X') return j;
            else printf("ERROR invalid YorX for findking");
            
            
        }
    }
    return -1;
}


int cancastle (struct board *board, int P, int direction) {
    if (!board->castle[P][1]) return 0;
    if (!allow_castling) return 0;
    int ROW = 7*(1-P);
    

    
    if (board->castle[P][0] && direction==-1) {
    if (board->squares[ROW][0] == pieces[P][1]  &&
        board->squares[ROW][1]=='x' && !ifsquare_attacked(board,ROW,1,P,0) &&
        board->squares[ROW][2]=='x' && !ifsquare_attacked(board,ROW,2,P,0) &&
        board->squares[ROW][3]=='x' && !ifsquare_attacked(board,ROW,3,P,0)) {
        
        
        return 1;
        
        
        
    }}
           
            
            
    
    
    
    if (board->castle[P][2] && direction==1) {
    if (board->squares[ROW][7] == pieces[P][1] &&
        board->squares[ROW][5]=='x' && !ifsquare_attacked(board,ROW,5,P,0) &&
        board->squares[ROW][6]=='x' && !ifsquare_attacked(board,ROW,6,P,0)){
        
        return 1;
    }}
        
    
   return 0;     
 
}

