#include "ev_chess.h"
#include "ev_evolution.h"

struct board board;



char pieces[2][6] = {{'P','R','N','B','Q','K'},{'p','r','n','b','q','k'}};

bool computer_turn = false;

char *output = (char *)malloc(256 * sizeof(char));

int machineplays = 1;
bool loadedmachine = false;

char *infoAUX = (char *)malloc(256 * sizeof(char));

//variable params for intelligent evolution (standards initialized);
int pvalues[6] = {100,500,300,300,900,2000};
int eval_randomness = 285;
int param_aperture = 7;
float param_seekmiddle = -2.75;
int param_DEEP = 5;
int param_seekpieces = 12;
float param_deviationcalc = 1.6;
int param_evalmethod = 2;
int param_seekatk = 20;
float param_TIMEweight[10] = {1.08,0.918,0.84,0.629,0.398,0.413,0.501,0.557,0.602,1.02};
float param_presumeOPPaggro = -4.9;
float param_pawnrankMOD = 0;
/*//////variable params for intelligent evolution*/

int i;
char *infomoveTABLE[2048];

bool selectTOPmachines = false;

int  infomoveINDEX;

char *machinepath;

bool show_info = false;

bool againstHUMAN = false;
bool toloadmachine = false;

bool allow_castling = true;

int main(int argc, char** argv) {

    int i=0;
    signal(SIGINT, SIG_IGN);    
    signal(SIGTERM, SIG_IGN); 
    signal(SIGCHLD, SIG_IGN);
    
    printf("id name e-v dchess engine v0.3\n");
    printf("id author afrogabs\n");
    printf("uci ok\n");

    
    char *inp;

    if (argc > 1) 
        for (i=0;i<argc;i++) {
            
        if (strstr(argv[i], "-TOP") != NULL) {selectTOPmachines = true; printf("ack.\n");}    
            
        if (strstr(argv[i], "-MD") != NULL) {toloadmachine = true; machinepath = argv[i+1];}
        
        if (strstr(argv[i], "--showinfo") != NULL) show_info = true;
        
        if (strstr(argv[i], "--XHUMAN") != NULL) againstHUMAN = true;
  
        }
    
        
    
    fflush(stdout);
    
    
    
    if (toloadmachine) loadmachine(1, machinepath);
    
    
    char testfehn[128] = "rn1qkbnr/ppp1pppp/3p4/8/3P2b1/4PN2/PPP1BPPP/RNBQK2R w KQkq - 5 5";
    
    

    for (i=param_DEEP;i>=0;i--) printf("timeWEIGHT for DEEP=%i   %f\n",i,scoremod(i,param_evalmethod));
    
    
    inp =(char *)malloc(128*sizeof(char));

    for (i=0;i<2018;i++) infomoveTABLE[i] = (char*)malloc(16 * sizeof(char));
    
    
    
    
    
    for (;;) {
    
    fflush(stdout);
    
    read(0,inp, 128);
    for (i=0;i<128;i++) if (inp[i] == '\n') inp[i]= ' ';
      
    /*printf("line received ");    
    for (i=0;i<128;i++) printf("%c",inp[i]);
    for (i=0;i<128;i++) inp[i]='0';
    printf("\n");
      */  
        
    if (strstr(inp, "isready") != NULL) {printf("readyok\n"); fflush(stdout);}
   
    if (strstr(inp, "quit") != NULL) return 0;
    
    //if (strstr(inp, "go") != NULL) computer(1);
    
    if (strstr(inp, "position") !=NULL) {
        if (strstr(inp, "startpos" ) != NULL) setup_board(1);
        else {

            fehn2board(inp);
        }}
    //if (strstr(inp, "moves") !=NULL) read_movelines(inp);
    
    if (strstr(inp, "show") !=NULL) show_board(board.squares);
    
    if (strstr(inp, "quit") !=NULL) return 0;
    
    if (strstr(inp, "new") != NULL) {machineplays = 1; setup_board(1);} 
    
    if (strstr(inp, "test") != NULL) fehn2board(testfehn);
    
    if (strstr(inp, "black") != NULL && strstr(inp, "white") != NULL) {machineplays = 0; computer(0);}
    
    if (strstr(inp, "white") == NULL && strstr(inp, "black") != NULL) {machineplays = 1; printf("playing black. (%i)", machineplays);}
    
    if (strstr(inp, "remove") != NULL) history_rollback(2);
    
    if (strstr(inp, "history") != NULL) {
        printf("move history: %i moves.\n", board.hindex);
        for (i=0; i < board.hindex; i++) {
            print_movement(&board.movehistory[i]);
        }
    }
    
    /*if (strstr(inp, "usermove") !=NULL) { 
        if (read_movelines(inp) > 0) {
            computer(1);
        }
    }*/
        
    if (strstr(inp, "list") !=NULL)  {
        legal_moves(&board,0,0);
        printf("list [%i]:\n", board.k);
        for (i=0; i < board.k; i++) { print_movement(&board.movelist[i]);
        printf("attacker? %c.\n", board.movelist[i].casualty);}
        fflush(stdout);
    }   
             
    if (strstr(inp, "lis1") !=NULL)  {
        legal_moves(&board,1,0);
        printf("list [%i]:\n", board.k);
        for (i=0; i < board.k; i++) { print_movement(&board.movelist[i]);
        printf("attacker? %c.\n", &board.movelist[i].casualty);}
         legal_moves(&board,1,0);
    }   
           
       
    if(strstr(inp, "result") !=NULL) {
        if (strstr(inp, "1-0")!=NULL && machineplays == 0) applyresult(1);
        else if (strstr(inp, "0-1") !=NULL && machineplays == 1) applyresult(1); 
        else if (strstr(inp, "1/2-1/2") !=NULL) applyresult(0);
        else applyresult(-1);
        
    }
    
    if (read_movelines(inp,0)) {
       computer(0);
        

    }
    
    
    //signal(SIGINT, computer);
//printf("\n>>>\n");
    //fflush(stdin);
    
    for (i=0;i<128;i++) inp[i]=0;

   
}   
    return 0;
}

void computer(int verbose) {
    int P = machineplays;
    Vb printf("thinking for %i\n",P); fflush(stdout);
    

    struct move move;

    
    if (think(&move, P , param_DEEP, 0) < 0) return;
    sleep(1);
    move_pc(&board, &move);
    
 
    history_append(&move);    
    cord2pos(move.from);
    cord2pos(move.to);
    
    Vb show_board(board.squares);
    

    
 //   if (move[0][0]=='O') {
 //       if (move[1][1] == 0) snprintf(output, 32, "move O-O-O\n");
 //       if (move[1][1] == 1) snprintf(output, 32, "move O-O\n");}
    

    
    else {
        
     if (move.promoteto != 0)  snprintf(output, 32 ,"move %c%c%c%cq\n",move.from[0],move.from[1],move.to[0],move.to[1]);
     else snprintf(output, 32 ,"move %c%c%c%c \n", move.from[0],move.from[1],move.to[0],move.to[1]);
     
    
    write(1, output, strlen(output));fflush(stdout);
    }
    }


void SIGthink(int signum) {
    computer(1);
}