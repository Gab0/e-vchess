/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include "lampreia.h"
int fehn2board (char str[]) {
    char *fstring;
    int z=0;
    int i=0;
    int j=0;
    int number=0;
    int PL=0;

    fstring = strtok(str, " ");
    
    fstring = strtok(NULL, " ");

    //read boardmap section.
    setup_board(0);
    printf("%s /%i\n",fstring, strlen(fstring));
    for (z=0;z<strlen(fstring);z++) {
    
     number = fstring[z]-'0';
        if (is_in(fstring[z],pieces[0],6)||is_in(fstring[z],pieces[1],6)) {
            board.squares[i][j] = fstring[z];
            j++;
        }
        if (fstring[z]=='/') {
            i++;
            j=0;        
        }
        if (0<number && number<9) j=j+number;

}
    
    fstring = strtok(NULL, " ");
    //read active player section.
    
    if (fstring[0] == 'b')
      board.whoplays=1;
    if (fstring[0] == 'w')
      board.whoplays = 0;
    
    fstring = strtok(NULL, " ");
    //read castling righst section.
    for (i=0;i<2;i++) for (j=0;j<3;j++) {
        board.castle[i][j]=0;
        board.castle[i][1]=1;
    }
    
    
    for (z=0;z<strlen(fstring);z++) {
        if (fstring[z] == 'Q') board.castle[0][0]=1;
        if (fstring[z] == 'K') board.castle[0][1]=1;
        if (fstring[z] == 'q') board.castle[1][0]=1;
        if (fstring[z] == 'k') board.castle[1][1]=1;
    }

    //read en-passant info;
    fstring = strtok(NULL, " ");

    

    //read half move info;
    fstring = strtok(NULL, " ");

    //read full move clock;
    fstring = strtok(NULL, " ");
    board.MovementCount =  atoi(fstring) -1;
    board.MovementCount *= 2;
    board.MovementCount += board.whoplays;
    
    hindex = 0;
    //board.MovementCount = 0;
    
    return PL;
}
int read_movelines (char txt[128], int verbose) {
    char *movement = strtok(txt, " ");
    struct move move;
    int x=0;

    //movement = strtok(NULL, " "); 

        while ( movement != NULL) {

            if (parse_move(&move, movement, 1-machineplays)) {
                Vb printf("moving from input: %i%i %i%i\n",move.from[0],move.from[1],move.to[0],move.to[1]);
                move.casualty = board.squares[move.to[0]][move.to[1]]; 
                
                move_pc(&board, &move);

                history_append(&move);
                x++;
            }
        movement = strtok(NULL, " ");        
                
        }
    Vb printf("%i moves read.\n",x);

    
    
    return x;
}
void print_movement (struct move *move, int full) {
    char play[2][2] = {{move->from[0], move->from[1]},
                        {move->to[0], move->to[1]}};
    
    cord2pos(play[0]);
    cord2pos(play[1]);
    
    print_play(play);
    
    if (full) {
        printf("iscastle = %i.\n", move->iscastle);
        printf("lostcastle = %i.\n", move->lostcastle);
        printf("passant = %i.\n", move->passant);
        printf("passantJ = %i %i.\n", move->passantJ[0], move->passantJ[1]);
        if (!move->promoteto) printf("promoteto = 0\n");
        else printf("promoteto = %c.\n", move->promoteto);
        printf("casualty = %c.\n", move->casualty);
    }
    
    
    
}
int parse_move (struct move *target, char *s, int P) {

    

    if (!isalpha(s[0]) || !isalpha(s[2])) return 0;
    if (!isdigit(s[1]) || !isdigit(s[3])) return 0;
    if (s[0] < 'a' || s[0] > 'h' || s[2] < 'a' || s[2] > 'h') return 0;
    if (s[1] < '1' || s[1] > '8' || s[3] < '1' || s[3] > '8') return 0;
    
    
        target->from[0] = s[0];
        target->from[1] = s[1];
        target->to[0] = s[2];
        target->to[1] = s[3];

        target->iscastle=0;
        target->lostcastle=0;
        //printf("%c%c %c%c\n", s[0],s[1],s[2],s[3]);
        pos2cord(target->from);
        pos2cord(target->to);           
        //printf("%i%i %i%i\n", target->from[0],target->from[1],target->to[0],target->to[1]);

        
        if(s[4]=='q') {
            if (s[1]<s[3])target->promoteto='Q';
            if (s[1]>s[3])target->promoteto='q';

        }
        
        else target->promoteto=0;
        
        //compute castling features of move.
        if (target->from[1] == 4){
            if (target->from[0] == 0 || target->from[0] == 7)
                if (target->to[1] == 6 || target->to[1] == 2)
                if (board.squares[target->from[0]][target->from[1]]==pieces[P][5]) {target->iscastle=1; //printf("castle.\n");
                
                }
        
        }
        
        //compute enpassant features of move.
        //initialize variables.
        target->passant=0;
        target->passantJ[0] = board.passantJ;
        target->passantJ[1] = -1;
        //set possibility, on double pawn movement.
        if (board.squares[target->from[0]][target->from[1]]==pieces[1-machineplays][0])
            if (target->from[0]==1||target->from[0]==6)
                if (target->to[0]==3||target->to[0]==4)
                    target->passantJ[1] = target->from[1];
        //read EP capture.
        if (target->to[1] == board.passantJ)
            if (board.squares[target->from[0]][target->to[1]]==pieces[machineplays][0])
                if (board.squares[target->from[0]][target->from[1]] == pieces[1-machineplays][0])
                    if ((target->from[0]==3&&machineplays)||(target->from[0]==4&&!machineplays))
                            target->passant=1;
                
        

        return 1;
        
    
}   
void eval_info_move(struct move *move, int DEEP, time_t startT, int P) {

  char buffer[4];
  time_t elapsedT = time(NULL) - startT;

  movement_to_string(move, buffer);
    
  asprintf(&output, "%i %ld %ld %i %s\n", DEEP, move->score, elapsedT, P, 
  	   buffer);
  write(1, output, strlen(output));   
}

void eval_info_group_move(struct move *primary, struct move *secondary, int DEEP, time_t startT, int P) {

  int m=0;
  time_t elapsedT = time(NULL) - startT;

  char bufferA[4];
  char bufferB[4];

  movement_to_string(primary, bufferA);
  movement_to_string(secondary, bufferB);
  
  asprintf(&output, "%i %ld %ld %i %s ", DEEP, secondary->score, elapsedT, P, bufferA);

  for(m=0;m<Brain.DEEP-1;m++)
    asprintf(&output, "%s???? ", output);

  asprintf(&output, "%s %s ", output, bufferB);
  
  for(m=0;m<Brain.DEEP-1;m++)
    asprintf(&output, "%s???? ", output);

  asprintf(&output, "%s\n", output);
  write(1, output, strlen(output));

}


void show_moveline(struct board *finalboard, int bottom_span, time_t startT) {
  char buffer[4];
  int I=0;
  int DEEP = finalboard->MovementCount-bottom_span;
  time_t elapsedT = time(NULL) - startT;
  
  //show_board(finalboard->squares);
  asprintf(&output, "%i %ld %ld %i", DEEP, finalboard->score, elapsedT, searchNODEcount);
  
  for (I=bottom_span; I < finalboard->MovementCount; I++) {
    movement_to_string(&finalboard->movements[I], buffer);

    asprintf(&output, "%s %c%c%c%c", output, buffer[0],buffer[1],buffer[2],buffer[3]);

  }
    asprintf(&output, "%s\n", output);
    write(1, output, strlen(output));
}


void stdoutWrite(const char * text) {
  asprintf(&output, text);
  write(1, output, strlen(output));
}



void show_movelist(struct movelist *moves) {
  int i=0;
        printf("list [%i]:\n", moves->k);
        for (i=0; i < moves->k; i++) { print_movement(&moves->movements[i], 0);
        printf("attacker? %c.\n", moves->movements[i].casualty);}

}

void show_board_matrix (int Matrix[8][8])
{
  int i=0, j=0, X=0;

  char *MatrixDrawLine = (char *) calloc(1, sizeof(char) * 26);      
    fprintf(stderr, "\n");
    for(i=0;i<8;i++)
      {
	sprintf(MatrixDrawLine, "");
	for(j=0;j<8;j++)
	  {
	    //printf("%i\n", Matrix[i][j]);
	    sprintf(MatrixDrawLine, "%s %i", MatrixDrawLine, Matrix[i][j]);
	    
	    if (j!=7)
	      {
		sprintf(MatrixDrawLine, "%s ", MatrixDrawLine);
	      }
	  }
	
	fprintf(stderr, "%s\n", MatrixDrawLine);
	X=0;
	
	
      }
    
    
    // DUMP(MatrixDrawLine);
}
