#include "ev_chess.h"


struct board board;
struct param Brain;
struct movelist moves;

Device struct param GBrain;
        
char pieces[2][6] = {{'P','R','N','B','Q','K'},
                     {'p','r','n','b','q','k'}};
Device char GPUpieces[2][6] = {{'P','R','N','B','Q','K'},
                               {'p','r','n','b','q','k'}};

bool computer_turn = false;

char *output = (char *)malloc(256 * sizeof(char));

int machineplays = 1;
Device int GPUmachineplays = 1;


bool loadedmachine = false;
char *specificMachine = (char *)malloc(64 * sizeof(char));


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

char *machinepath = (char *)malloc(sizeof(char)*128);

bool Show_Info = false;
Device bool show_info = false;

bool againstHUMAN = false;
bool toloadmachine = false;

bool loadDEEP = false;
bool loadxDEEP = false;


int NODEcount;

IFnotGPU( bool allow_castling = true; )
IFGPU( __device__ bool allow_castling = true; )
        
        
            //iniatializing variables with standard values.
    
    //pvalues is the value of each piece in centipawns. 
    //order is pawn-rook-knight-bishop-queen-king.

    
    
    
    
int main(int argc, char** argv) {
    Brain.pvalues[0] = 100;
    Brain.pvalues[1] = 500;
    Brain.pvalues[2] = 300;
    Brain.pvalues[3] = 300;
    Brain.pvalues[4] = 900;
    Brain.pvalues[5] = 2000;
    
    //randomness is the limit to the randomic small variability on the score, 
    //in centipawns.
    Brain.randomness = 50;
    //seekmiddle augments the score for pieces in the center of the board.
    Brain.seekmiddle = 0;
    //DEEP is the number of future moves to be evaluated.
    //must be an even number, in order to always end in a engine move.
    Brain.DEEP = 4;
    //xDEEP is the number of evaluations on top of the original one will be made,
    //'artificially' increasing total ply deepnes by xDEEP * DEEP;
    Brain.xDEEP = 0;
    //seekpieces augments the score for attacked enemy pieces.
    Brain.seekpieces = 1;
    
    Brain.deviationcalc = 0;
    Brain.evalmethod = 0;
    //seekatk augments the score for taken pieces.
    Brain.seekatk = 0;
    //brain.TIMEweight = {1.08,0.918,0.84,0.629,0.398,0.413,0.501,0.557,0.602,1.02};
    Brain.presumeOPPaggro = 0;
    //pawnrankMOD augments the score of the pawns, by the rank he occupies.
    Brain.pawnrankMOD = 0;
    Brain.parallelcheck = 0;
    Brain.balanceoffense = 0;    
    Brain.cumulative = 0;
    Brain.MODbackup = 0;
    Brain.MODmobility = 0;

    int i=0;
    signal(SIGINT, SIG_IGN);    
    signal(SIGTERM, SIG_IGN); 
    signal(SIGCHLD, SIG_IGN);
    
    printf("id name e-v dchess engine v0.7\n");
    printf("id author afrogabs\n");
    printf("uci ok\n");
    
    
    
    
    
    char *inp;

    if (argc > 1) 
     for (i=0;i<argc;i++) {
        
            
        if (strstr(argv[i], "-TOP") != NULL)
	  selectTOPmachines = true;
	
	if (strstr(argv[i], "--specific") != NULL)
	  sprintf(specificMachine, "%s", argv[i+1]);

            
        if (strstr(argv[i], "-MD") != NULL) {
	  toloadmachine = true;
	  machinepath = argv[i+1];
	}
        
        if (strstr(argv[i], "--showinfo") != NULL) Show_Info = true;
        
        if (strstr(argv[i], "--XHUMAN") != NULL) againstHUMAN = true;
        
        if (strstr(argv[i], "--deep") != NULL) {
            Brain.DEEP = (float)atof(argv[i+1]);
            loadDEEP = false;
        }
	if (strstr(argv[i], "--xdeep") != NULL)
	  {
	  Brain.xDEEP = (float)atof(argv[i+1]);
	  loadxDEEP = false;
	}
	
        }  
    
        
    
    fflush(stdout);
    
 
    
    if (toloadmachine) loadmachine(0, machinepath);
    

    #ifdef __CUDACC__
    cudaMemcpyToSymbol(GBrain, &Brain, sizeof(struct param),0, cudaMemcpyHostToDevice);
    cudaMemcpyToSymbol(show_info, &Show_Info, sizeof(bool),0, cudaMemcpyHostToDevice);
    cudaMemcpyToSymbol(GPUmachineplays, &machineplays, sizeof(int),0, cudaMemcpyHostToDevice);
    #else
    show_info = Show_Info;
    #endif

    
    char testfehn[128] = "fen r3kbr1/3bp2p/1pN3p1/1B1pQ3/PP2p3/7P/5qPR/RNK5 b - - 2 26";
        

    for (i=Brain.DEEP;i>=0;i--) 
        printf("timeWEIGHT for DEEP=%i   %f\n",i,scoremod(i,Brain.evalmethod));
    
    
    inp =(char *)malloc(128*sizeof(char));

    //for (i=0;i<2018;i++) infomoveTABLE[i] = (char*)malloc(16 * sizeof(char));
    
    //printf("brain.deep = %f", Brain.DEEP);
    //read inputs loop.
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
    
    if (strstr(inp, "position") !=NULL) {
        if (strstr(inp, "startpos" ) != NULL) setup_board(1);
        else fehn2board(inp);
        }
    //if (strstr(inp, "moves") !=NULL) read_movelines(inp);
    
    if (strstr(inp, "show") !=NULL) show_board(board.squares);
    
    if (strstr(inp, "quit") !=NULL) break;
    
    if (strstr(inp, "new") != NULL) setup_board(1);
    
    if (strstr(inp, "test") != NULL) fehn2board(testfehn);
    
    if (/*strstr(inp, "black") != NULL*/strstr(inp, "white") != NULL) 
        machineplays = 0;
    
    if (strstr(inp, "white") == NULL && strstr(inp, "black") != NULL)
        machineplays = 1; 
    
    if (strstr(inp, "remove") != NULL) history_rollback(2);
    
    if (strstr(inp, "dump") != NULL) dump_history();
    
    if (strstr(inp, "go") != NULL) computer(0);
    
    if (strstr(inp, "history") != NULL) {
        printf("move history: %i moves.\n", hindex);
        for (i=0; i < hindex; i++) {
            print_movement(&movehistory[i],0);
        }
    }
    
    if (strstr(inp, "testsend") != NULL) {
        applyresult(1);
    }
    
    /*if (strstr(inp, "usermove") !=NULL) { 
        if (read_movelines(inp) > 0) {
            computer(1);
        }
    }*/
#ifndef __CUDACC__
    if (strstr(inp, "list") !=NULL)  {
        legal_moves(&board, &moves,0,0);
        printf("list [%i]:\n", moves.k);
        for (i=0; i < moves.k; i++) { print_movement(&moves.movements[i],0);
        printf("attacker? %c.\n", moves.movements[i].casualty);}
    }   
             
    if (strstr(inp, "lis1") !=NULL)  {
        legal_moves(&board,&moves,1,0);
        printf("list [%i]:\n", moves.k);
        for (i=0; i < moves.k; i++) { print_movement(&moves.movements[i],0);
        printf("attacker? %c.\n", moves.movements[i].casualty);}
    }   
#endif
       
       if(strstr(inp, "result") !=NULL) {
        if (strstr(inp, "1-0") !=NULL && machineplays == 0) applyresult(1);
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

    
    for (i=0;i<128;i++) inp[i]=0;

   
}   
    return 0;
}

void computer(int verbose) {
    int P = machineplays;
    Vb printf("thinking for %i\n",P); fflush(stdout);
    

    struct move move;

    
    if (think(&move, P , Brain.DEEP, verbose) < 0) {
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

/*#ifdef __CUDACC__
Global void UpdateGPUBrain() {
       
    //iniatializing variables with standard values.
    
    //pvalues is the value of each piece in centipawns. 
    //order is pawn-rook-knight-bishop-queen-king.
    GBrain.pvalues[0] = Brain.pvalues[0];
    GBrain.pvalues[1] = Brain.pvalues[1];
    GBrain.pvalues[2] = Brain.pvalues[2];
    GBrain.pvalues[3] = Brain.pvalues[3];
    GBrain.pvalues[4] = Brain.pvalues[4]; 
    GBrain.pvalues[5] = Brain.pvalues[5];
    
    //randomness is the limit to the randomic small variability on the score, 
    //in centipawns.
    GBrain.randomness = Brain.randomness;
    //seekmiddle augments the score for pieces in the center of the board.
    GBrain.seekmiddle = Brain.seekmiddle;
    //DEEP is the number of future moves to be evaluated.
    //must be an even number, in order to always end in a engine move.
    GBrain.DEEP = Brain.DEEP;
    //seekpieces augments the score for attacked enemy pieces.
    GBrain.seekpieces = Brain.seekpieces;
    
    GBrain.deviationcalc = Brain.deviationcalc;
    GBrain.evalmethod = Brain.evalmethod;
    //seekatk augments the score for taken pieces.
    GBrain.seekatk = Brain.seekatk;
    //brain.TIMEweight = {1.08,0.918,0.84,0.629,0.398,0.413,0.501,0.557,0.602,1.02};
    GBrain.presumeOPPaggro = Brain.presumeOPPaggro;
    //pawnrankMOD augments the score of the pawns, by the rank he occupies.
    GBrain.pawnrankMOD = Brain.pawnrankMOD;
    GBrain.parallelcheck = Brain.parallelcheck;
    GBrain.balanceoffense = Brain.balanceoffense;
    GBrain.cumulative = Brain.cumulative;
    GBrain.MODbackup = Brain.MODbackup;
    GBrain.MODmobility = Brain.MODmobility;
    
}
#endif*/
