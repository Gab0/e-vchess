/* 
 * File:   main.cpp
 * Author: gabs
 *
 * Created on September 20, 2015, 11:29 PM
 */

#include "lampreia.h"

//using namespace std;


void setup_board (int setup) {
  int i=0, j=0;
    char setup_position[128] = "fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
   if (setup)
     fehn2board(setup_position);

    
   else 
     forsquares 
       board.squares[ SQR(i, j) ] = 'x';
   
    
    board.passantJ=-1;
    board.passant_player=-1;

    //erase_moves(&board, 1);
    board.MovementCount=0;
    hindex=0;
}

void show_board (char squares[64])
{
  int i=0, j=0, X=0;

  output[X] = '\n';
  X++;
  for(i=0;i<8;i++){
    for(j=0;j<8;j++){
	  
      output[X] = squares[ SQR(i, j) ];
      X++;
      if (j!=7)
	{
	  output[X] = ' ';
	  X++;
	}

	  
    }
    output[X] = '\n';
    X++;
    //fprintf(stderr, "%s\n", BoardDraw);
    write(2, output, X);
    X=0;
    //BoardDraw[X] = '\n'; X++;
    //BoardDraw[X] = '\n'; X++;
    
  }
  //if (full){
  //     printf("")
  // }
    
  // BoardDraw[X] = '\n';
  // fprintf(stderr, "%s", BoardDraw);

    
}

Host Device int legal_moves (struct board *board, struct movelist *moves, int PL, int verbose) {
    
    moves->k=0;
    moves->kad=0;
    
    int EP = 1-PL;
    int i = 0;
    int j = 0;
    
    int zeta = 0;
    
    int pawn_vector = -1 + 2*PL;
    
    int promote = PL;

    forsquares
      {
	promote = 0;
	if (!is_in(board->squares[ SQR(i, j) ], Pieces[PL], 6)) continue;

	if (board->squares[SQR(i, j)] == Pieces[PL][0])
	  {
	  
	  if ( (i==6&&PL==1) || (i==1&&PL==0) )
	    promote = 4;
	  
          
	  if (board->passantJ==j+1||board->passantJ==j-1)
                    if (board->passantJ>-1)
		      if ( (i==3&&!PL) || (i==4&&PL) ) 
			if (board->squares[ SQR(i, board->passantJ) ] == Pieces[1-PL][0])
			  if (board->passant_player == flip(PL))
			  append_move(board, moves, SQR(i,j), SQR( (i + pawn_vector), board->passantJ ), 1, PL);
	  
          
	  if ((is_in(board->squares[ SQR((i+pawn_vector), (j+1)) ], Pieces[EP], 6)) && onboard((i+pawn_vector), (j+1)))
	    {
	      //printf(">%i>%i=%i    %c  %i= <%i<%i\n", i, j, SQR(i,j), board->squares[ SQR(i+pawn_vector, j+1)], SQR((i+pawn_vector), (j+1)), i+pawn_vector, j+1);
	      //show_board(board->squares);
	      append_move(board, moves, SQR(i, j), SQR( (i+pawn_vector), (j+1) ), promote, PL);
	    }
          
	  if ((is_in(board->squares[ SQR((i+pawn_vector), (j-1)) ], Pieces[EP], 6)) && onboard((i+pawn_vector), j-1))
	    append_move(board, moves, SQR(i, j), SQR( (i+pawn_vector), (j-1)), promote, PL);
                                       
	  
          
	  if (board->squares[ SQR((i+pawn_vector), j) ] == 'x' && onboard((i+pawn_vector),j) )
	    append_move(board, moves, SQR(i, j), SQR( (i+pawn_vector), j), promote, PL);
		



		if (i == 6-5*PL) // PL == 1 -> i == 1; PL == 0 -> i == 6
		  if (board->squares[ SQR((i+2*pawn_vector), j) ] == 'x')
		    if (board->squares[ SQR((i+pawn_vector), j) ] == 'x')
		      {
			append_move(board, moves, SQR(i, j), SQR( (i+4*PL-2), j), 2, PL);
		      }
	  }
	
	
	
	//tower movements.        
	if (board->squares[ SQR(i, j) ] == Pieces[PL][1])
	  {
	    //printf("%c\n",board->squares[ SQR(i, j) ]);
	    movement_generator(board, moves, 0, '+', i, j, PL);
	  }
        
      // HORSE movements. note -(k_atk-2) is a mathematical expression to equal 0 if k_atk=2, or 1 if k_atk=1 
     if (board->squares[SQR(i, j)] == Pieces[PL][2])
       {
       zeta = 0;
         int HT[4][2] = {{1,2},{-1,2},{1,-2},{-1,-2}};
         int k_atk = 0;
         for (zeta=0; zeta < 4; zeta++)
	   {
             
	     k_atk = mpc(board->squares, i+HT[zeta][0],j+HT[zeta][1], PL);       
	     if (k_atk) append_move(board,moves, SQR(i, j),SQR( (i+HT[zeta][0]),(j+HT[zeta][1]) ), 0, PL);
     
     
	     k_atk = mpc(board->squares, i+HT[zeta][1],j+HT[zeta][0], PL);
	     if (k_atk) append_move(board,moves, SQR(i, j) ,SQR( (i+HT[zeta][1]), (j+HT[zeta][0]) ), 0, PL);
     
         }
     }
     
     //bishop movements
     if(board->squares[SQR(i, j)] == Pieces[PL][3])
       {
	 movement_generator(board, moves, 0, 'X', i, j, PL);
       }
     
     //queen movements
     if (board->squares[SQR(i, j)] == Pieces[PL][4])
       {
       	 movement_generator(board,moves,0, '+', i, j, PL);
	 movement_generator(board,moves,0, 'X', i, j, PL);
       }
     
     //king movements
     if (board->squares[SQR(i, j)] == Pieces[PL][5])
       {
             
	 movement_generator(board,moves,1, '+', i, j, PL);
	 movement_generator(board,moves,1, 'X', i, j, PL);

	 if (j == 4)
	   if (i == (flip(PL)) *  7)
	       {
	     
		 if (cancastle(board, PL, -1))
		   append_move(board, moves, SQR(i, j), SQR(i, 2), 3, PL);

		 if (cancastle(board, PL, 1))
		   append_move(board, moves, SQR(i, j), SQR(i, 6), 3, PL);
	     
	       }
	 
       }	
      }
    

    
    attackers_defenders(moves);
    
    return 0;
}

Host Device int mpc(char squares[64], int i, int j, int player) {
  
  int enemy = flip(player);
  
  
  if (onboard(i, j))
    {
      if (squares[ SQR(i, j) ] == 'x') return 2;
      
      if (is_in(squares[ SQR(i, j) ], Pieces[enemy], 6))
	return 1;
      
      else return 0;
      
    }
  else return 0;
}

Host Device void move_piece(struct board *tg_board, struct move *movement, int MoveUnmove)
{
  
  if (movement->casualty==0)
    movement->casualty='x';
  int cP;
  
  int from;
  int to;
  if (MoveUnmove > 0)
    {
      from = movement->from;
      to = movement->to;

      tg_board->squares[ from ] = 'x';
    }
  else
    {
      to = movement->from;
      from = movement->to;

      tg_board->squares[ from ] = movement->casualty;
    }

    tg_board->squares[ to ] = movement->piece;

    if (movement->promoteto!=0 && MoveUnmove > 0)
      tg_board->squares[ to ] = movement->promoteto;
    
    if(movement->iscastle)
      {
	if (MoveUnmove > 0)
	  {
	    if (SQR_J(to) < 4)
	      {
		tg_board->squares[ to + 1 ] =
		  tg_board->squares[ SQR( SQR_I(to), 0) ];
		tg_board->squares[ SQR( SQR_I(to), 0) ] = 'x';
	      }
        
	    if (SQR_J(to) > 4)
	      {
		tg_board->squares[ to - 1 ] =
		  tg_board->squares[ SQR( SQR_I(to), 7) ];
		tg_board->squares[ SQR( SQR_I(to), 7) ] = 'x';
	      }

	  }
	else
	  {// ATTENTION; (solved?)
	    if (SQR_J(from) < 4)
	      {
		tg_board->squares[ SQR(SQR_I(from), 0) ] = 
		  tg_board->squares[ from + 1 ];
		tg_board->squares[ from + 1 ] = 'x';
	      }
	    
	    if (SQR_J(from) > 4)
	      {
		tg_board->squares[ SQR(SQR_I(from), 7) ] =
		  tg_board->squares[ from -1 ];
		tg_board->squares[ from - 1 ] = 'x';
	      } 

	  }

      }
    
    if (movement->passant)
      {
	if (MoveUnmove > 0)
	  tg_board->squares[ SQR(SQR_I(from), SQR_J(to)) ] = 'x';
	else
	  {
	    tg_board->squares[ SQR( SQR_I(to), SQR_J(from) ) ] = movement->casualty;
	    tg_board->squares[ from ] = 'x';
	  }
      }

    int AffectCastling = 0;
    
    if (MoveUnmove == -1)
      AffectCastling = 1;

    F(cP,2)
    if (movement->lostcastle[cP])
      {
	//int P=0;
	// SWAP THIS
	//if (SQR_I(from) == 0) P=1;
	//if (is_in(movement->piece, Pieces[1], 6)) P=1;

	//	if (movement->lostcastle == 2)
	  
	FLIP(tg_board->castle[cP][ movement->lostcastle[cP]-1 ]);

      }
    
    tg_board->passantJ = movement->passantJ[1-AffectCastling];
    tg_board->passant_player = movement->passant_player[1-AffectCastling];

   FLIP(tg_board->whoplays);
    
    if (MoveUnmove > 0)
      replicate_move(&tg_board->movements[tg_board->MovementCount], movement);
    tg_board->MovementCount += MoveUnmove;
    
    
}
    // LEGACY FUNCTION;
    /*Host Device void undo_move(struct board *tg_board, struct move *movement) {
    
    
    char to[2] = {movement->from[0], movement->from[1]};
    char from[2] = {movement->to[0], movement->to[1]};
    

     
    if(movement->casualty==0) movement->casualty='x';
    
    
    tg_board->squares[ SQR(to[0], to[1]) ] = tg_board->squares[ SQR(from[0], from[1]) ];
    tg_board->squares[ SQR(from[0], from[1]) ] = movement->casualty;
 
    

    
    if(movement->promoteto!=0)
      {
      if (movement->from[0]==6) tg_board->squares[ SQR(to[0], to[1]) ] = 'p';
      if (movement->from[0]==1) tg_board->squares[ SQR(to[0], to[1]) ] = 'P';
      }
    
    if(movement->iscastle){
        if (movement->from[1] < 4)
	  {
	    tg_board->squares[ SQR(from[0], 0) ] = tg_board->squares[ SQR(to[0], (from[1]+1)) ];
	    tg_board->squares[ SQR(from[0], (from[1]+1)) ] = 'x';
	  }
        
        if (movement->from[1] > 4)
	  {
	    tg_board->squares[ SQR(from[0], 7) ] = tg_board->squares[ SQR(to[0], (from[1]-1)) ];
	    tg_board->squares[ SQR(from[0], (from[1]-1)) ] = 'x';
	  } 
    }
    
    if(movement->passant)
      {
        tg_board->squares[ SQR(to[0], from[1]) ] = movement->casualty;
	tg_board->squares[ SQR(from[0], from[1]) ] = 'x';
      }
    
    int cP=-1;
    if(to[0]==7) cP = 0;
    if(to[0]==0) cP = 1;
    
    if (cP > -1)
      {
        if(movement->lostcastle==1) tg_board->castle[cP][0] = 1;
        if(movement->lostcastle==2) tg_board->castle[cP][1] = 1;
        if(movement->lostcastle==3) tg_board->castle[cP][2] = 1;
      }
    
    tg_board->passantJ=movement->passantJ[0];
    
    flip(tg_board->whoplays);
    tg_board->MovementCount--;

    
    }*/

Host Device void undo_lastMove(struct board *board, int Number) {
  int N=0;
  for (N=0; N<Number;N++)
    move_piece(board,
	       &board->movements[board->MovementCount-1], -1);  

}
    
Device void attackers_defenders (struct movelist *moves)
{
  int M = 0;
  moves->kad = 0;



  F(M, moves->k)
    {
      if (moves->movements[M].casualty != 'x')
	{
	  
	  //print_movement(&moves->movements[M], 1);
	  moves->attackers[moves->kad][0] = moves->movements[M].piece;
	  moves->attackers[moves->kad][1] = moves->movements[M].from;
	  
	  moves->defenders[moves->kad][0] = moves->movements[M].casualty;
	  moves->defenders[moves->kad][1] = moves->movements[M].to;

	  moves->kad++;
        }
    } 
}


void history_append(struct move *move)
{
    int i=0,j=0;
    
    replicate_move(&movehistory[hindex], move);

    cloneboard(&board, &boardhistory[hindex]);
                    
    hindex++;
    
}
    



int history_rollback(int times) {
    int i=0;
    
    for (i=0;i<times;i++) {
    hindex--;
    print_play_cord(movehistory[hindex]);
    move_piece(&board, &movehistory[hindex], -1);
    
    }
   return 0; 
}



Host Device int findking (char board[64], char YorX, int player) {
    char KING = Pieces[player][5];
 
    int i=0;
    int j=0;
 
    for (i=0;i<8;i++) for (j=0;j<8;j++) {
        if (board[SQR(i, j)] == KING)
	  {
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
    if (ifsquare_attacked(board->squares, ROW, 4, 1-P,0,0)) return 0;    

    
    if (board->castle[P][0] && direction==-1)
      {
	if (board->squares[ SQR(ROW, 0) ] == Pieces[P][1])
	  {
	  if(board->squares[ SQR(ROW, 1) ] == 'x' && !ifsquare_attacked(board->squares,ROW,1,1-P,0,0) &&
	     board->squares[ SQR(ROW, 2) ] == 'x' && !ifsquare_attacked(board->squares,ROW,2,1-P,0,0) &&
	     board->squares[ SQR(ROW, 3) ] == 'x' && !ifsquare_attacked(board->squares,ROW,3,1-P,0,0))
	    return 1;
	  }

      }
    
            
    
    if (board->castle[P][2] && direction==1)
      {
	if (board->squares[ SQR(ROW, 7) ] == Pieces[P][1])
	  if (board->squares[ SQR(ROW, 5) ] == 'x' && !ifsquare_attacked(board->squares,ROW,5,1-P,0,0) &&
	    board->squares[ SQR(ROW, 6) ] == 'x' && !ifsquare_attacked(board->squares,ROW,6,1-P,0,0))

	  {
	    return 1;
	}
      }
    
    
    return 0;     
    
}

Host Device void movement_generator(struct board *board, struct movelist *moves, int limit, 
                        char direction, int i, int j, int P) {
    int X=0, q=0;
    int Ti=0, Tj=0;
    
    int M=0;


    if (direction=='+')
      M = 1;

    for (X=0;X<4;X++)
      {
	q=1;
	while (q>0) {
	  Ti = i + MovementDiagonalLinear[M][X][0] *q;
	  Tj = j + MovementDiagonalLinear[M][X][1] *q;
	  
	  
	  
	  if (onboard(Ti,Tj))
	       {
		 
		 if (board->squares[ SQR(Ti, Tj) ] == 'x')
		   {
		     append_move(board, moves, SQR(i, j), SQR(Ti, Tj), 0, P);
		     q++;
		     if (limit)
		       {
			 q=0;
			 continue;
		       }
		     
		   }
		 
		 
		 if (is_in(board->squares[ SQR(Ti, Tj) ], Pieces[1-P], 6))
		   {
		     append_move(board, moves, SQR(i, j), SQR(Ti, Tj), 0, P);
		     q=0;		   
		   }
		 if (is_in(board->squares[ SQR(Ti, Tj) ], Pieces[P], 6)) q=0;
	       }
	  else
	    q=0;
	}
      }
}

int countPieces (char squares[64], int CountPawns)
{
  int PieceCount=0, i=0;
  if (CountPawns)
    F(i, 64)  
      if (squares[i] != 'x')
	PieceCount += 1;
  
  else
    F(i,64)
      if (squares[i] != 'x' && squares[i] != 'p' && squares[i] != 'P') PieceCount += 1;
  
  return PieceCount;
}
