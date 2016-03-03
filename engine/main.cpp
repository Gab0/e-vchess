#include "ev_chess.h"


struct board board;
struct param Brain;

char pieces[2][6] = {{'P','R','N','B','Q','K'},{'p','r','n','b','q','k'}};

bool computer_turn = false;

char *output = (char *)malloc(256 * sizeof(char));

int machineplays = 1;
bool loadedmachine = false;

char *infoAUX = (char *)malloc(256 * sizeof(char));

char *infoMOVE = (char *) malloc(sizeof(char)*128);

struct move movehistory[512];
char movehistoryboard[512][8][8];
int hindex; 
//variable params for intelligent evolution (standards initialized);


/*//////variable params for intelligent evolution*/

int i;
char *infomoveTABLE[2048];

bool selectTOPmachines = false;

int  infomoveINDEX;

char *machinepath;

bool show_info = false;

bool againstHUMAN = false;
bool toloadmachine = false;
bool loadDEEP = true;

bool allow_castling = true;

int main(int argc, char** argv) {

    int i=0;
    signal(SIGINT, SIG_IGN);    
    signal(SIGTERM, SIG_IGN); 
    signal(SIGCHLD, SIG_IGN);
    
    printf("id name e-v dchess engine v0.3\n");
    printf("id author afrogabs\n");
    printf("uci ok\n");
    
    //iniatializing variables with standard values.
    
    //pvalues is the value of each piece in centipawns. 
    //order is pawn-rook-knight-bishop-queen-king.
    Brain.pvalues[0] = 100;
    Brain.pvalues[1] = 500;
    Brain.pvalues[2] = 300;
    Brain.pvalues[3] = 300;
    Brain.pvalues[4] = 900;
    Brain.pvalues[5] = 2000;
    
    //randomness is the limit to the randomic small variability on the score, 
    //in centipawns.
    Brain.randomness = 10;
    //seekmiddle augments the score for pieces in the center of the board.
    Brain.seekmiddle = 0;
    //DEEP is the number of future moves to be evaluated.
    //must be an even number, in order to always end in a engine move.
    Brain.DEEP = 2;
    //seekpieces augments the score for attacked enemy pieces.
    Brain.seekpieces = 1;
    
    Brain.deviationcalc = 0;
    Brain.evalmethod = 0;
    //seekatk augments the score for taken pieces.
    Brain.seekatk = 0;
    //brain.TIMEweight = {1.08,0.918,0.84,0.629,0.398,0.413,0.501,0.557,0.602,1.02};
    Brain.presumeOPPaggro = 1;
    //pawnrankMOD augments the score of the pawns, by the rank he occupies.
    Brain.pawnrankMOD = 0;
    Brain.parallelcheck = 0;
    Brain.balanceoffense = 0;    
    Brain.cumulative = 0;
    Brain.MODbackup = 0;
    Brain.MODmobility = 0;
    
    
    
    
    
    
    
    
    
    char *inp;

    if (argc > 1) 
     for (i=0;i<argc;i++) {
        
            
        if (strstr(argv[i], "-TOP") != NULL) {
            selectTOPmachines = true; printf("ack.\n");}    
            
        if (strstr(argv[i], "-MD") != NULL) {
            toloadmachine = true; machinepath = argv[i+1];}
        
        if (strstr(argv[i], "--showinfo") != NULL) show_info = true;
        
        if (strstr(argv[i], "--XHUMAN") != NULL) againstHUMAN = true;
        
        if (strstr(argv[i], "--deep") != NULL) {
            Brain.DEEP = (float)atof(argv[i+1]);
            loadDEEP = false;
        }
     }  
    
        
    
    fflush(stdout);
    
 
    
    if (toloadmachine) loadmachine(0, machinepath);
    

    
    char testfehn[128] = "r2qk2r/7n/3p3n/1p1pPBp1/PR4Pp/2p4P/2P1p3/2Q1K1NR b - - 5 65";
    
    

    for (i=Brain.DEEP;i>=0;i--) 
        printf("timeWEIGHT for DEEP=%i   %f\n",i,scoremod(i,Brain.evalmethod));
    
    
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
    
    if (strstr(inp, "go") != NULL) computer(1);
    
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
    
    if (strstr(inp, "black") == NULL && strstr(inp, "white") != NULL) {
        machineplays = 0; computer(0); printf("playing white.");}
    
    if (strstr(inp, "white") == NULL && strstr(inp, "black") != NULL) {
        machineplays = 1; printf("playing black. (%i)", machineplays);}
    
    if (strstr(inp, "remove") != NULL) history_rollback(2);
    
    if (strstr(inp, "dump") != NULL) dump_history();
    
    
    
    if (strstr(inp, "history") != NULL) {
        printf("move history: %i moves.\n", hindex);
        for (i=0; i < hindex; i++) {
            print_movement(&movehistory[i]);
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
    }   
             
    if (strstr(inp, "lis1") !=NULL)  {
        legal_moves(&board,1,0);
        printf("list [%i]:\n", board.k);
        for (i=0; i < board.k; i++) { print_movement(&board.movelist[i]);
        printf("attacker? %c.\n", board.movelist[i].casualty);}
    }   
           
       
    if(strstr(inp, "result") !=NULL) {
        if (strstr(inp, "1-0")!=NULL && machineplays == 0) applyresult(1);
        else if (strstr(inp, "0-1") !=NULL && machineplays == 1) applyresult(1); 
        else if (strstr(inp, "1/2-1/2") !=NULL) applyresult(0);
        else applyresult(-1);
        
    }
    if(strstr(inp, "echo") !=NULL) {
        write(1, output, strlen(output));fflush(stdout);
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

    
    if (think(&move, P , Brain.DEEP, 0) < 0) {
        printf("puta merda.\n");return;}
    //sleep(1);
    move_pc(&board, &move);
    
 
    history_append(&move);    
    cord2pos(move.from);
    cord2pos(move.to);
    
    //Vb show_board(board.squares);
    
          
     if (move.promoteto != 0)  
         snprintf(output, 32 ,"move %c%c%c%cq\n",
                 move.from[0],move.from[1],move.to[0],move.to[1]);
     else snprintf(output, 32 ,"move %c%c%c%c \n", 
                 move.from[0],move.from[1],move.to[0],move.to[1]);
     
    //sleep(1);
    write(1, output, strlen(output));fflush(stdout);
    
    }


void SIGthink(int signum) {
    computer(1);
}