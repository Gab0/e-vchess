#include "lampreia.h"

const char *Version = "v0.803";
struct board board;
struct param Brain;
struct movelist moves;

Device struct param GBrain;
        
char pieces[2][6] = {{'P','R','N','B','Q','K'},
                     {'p','r','n','b','q','k'}};
Device char GPUpieces[2][6] = {{'P','R','N','B','Q','K'},
                               {'p','r','n','b','q','k'}};
const float BoardMiddleScoreWeight[8] = {0, 0.33, 0.66, 1, 1, 0.66, 0.33, 0};
bool computer_turn = false;

char *output = (char *)malloc(364 * sizeof(char));

int machineplays = 1;
Device int GPUmachineplays = 1;

bool fastmode = false;

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

bool thinkVerbose = false;

int searchNODEcount;

IFnotGPU( bool allow_castling = true; )
IFGPU( __device__ bool allow_castling = true; )
        
  
    
int main(int argc, char** argv) {

  //DEEP is the number of future moves to be evaluated.
  //must be an even number, in order to always end in a engine move.
  Brain.DEEP = 4;

    
  //xDEEP is the number of evaluations on top of the original one will be made,
  //'artificially' increasing total ply deepness by xDEEP * DEEP;
  Brain.xDEEP = 0;


  //yDEEP is the number of movements from the first section of evaluating
  //to be considered for the second, yDEEP top movements.
  Brain.yDEEP = 8;

    
  setBrainStandardValues();
    int i=0;
    signal(SIGINT, SIG_IGN);    
    signal(SIGTERM, SIG_IGN); 
    signal(SIGCHLD, SIG_IGN);
    
   
    asprintf(&machinepath, "../machines");
    
    printf("lampreia chess engine %s\n", Version);
    printf("author afrogabs\n\n");
    
    char *inp;

    if (argc > 1) 
     for (i=0;i<argc;i++) {
        
            
       if (strstr(argv[i], "-TOP") != NULL || strstr(argv[i], "-t") != NULL)
	  selectTOPmachines = true;
	
	if (strstr(argv[i], "--specific") != NULL)
	  sprintf(specificMachine, "%s", argv[i+1]);
	
	if (strstr(argv[i], "-l") != NULL)
	  toloadmachine = true;
            
        if (strstr(argv[i], "-MD") != NULL) {
	  toloadmachine = true;
	  
	  machinepath = argv[i+1];
	}
        
        if (strstr(argv[i], "--showinfo") != NULL) Show_Info = true;
        
        if (strstr(argv[i], "--XHUMAN") != NULL) againstHUMAN = true;
        
        if (strstr(argv[i], "--deep") != NULL) 
	  BRAIN.DEEP = (int)atoi(argv[i+1]);

	if (strstr(argv[i], "--xdeep") != NULL)
	  BRAIN.xDEEP = (int)atoi(argv[i+1]);

	if (strstr(argv[i], "--ydeep") != NULL)
	  Brain.yDEEP = (int) atoi(argv[i+1]);
	
	if (strstr(argv[i], "--tverbose") != NULL) thinkVerbose = true;

	if (strstr(argv[i], "--fast") != NULL) fastmode = true;


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

    
    char testfehn[128] = "fen r1bqkb1r/pppp1ppp/2n2n2/4p2Q/2B1P3/8/PPPP1PPP/RNB1K1NR w KQkq - 4 4";

    printf("DEEP=%i   xDEEP=%i   yDEEP=%i\n\n", Brain.DEEP, Brain.xDEEP, BRAIN.yDEEP);
 
    if (fastmode)
      printf("fastmode on\n");
    
    inp =(char *)malloc(128*sizeof(char));

    //for (i=0;i<2018;i++) infomoveTABLE[i] = (char*)malloc(16 * sizeof(char));
    
    //printf("brain.deep = %f", Brain.DEEP);
    //read inputs loop.
    for (;;) {
    
    fflush(stdout);
    
    read(0, inp, 128);
    for (i=0;i<128;i++) if (inp[i] == '\n') inp[i]= ' ';
      
    /*printf("line received ");    
    for (i=0;i<128;i++) printf("%c",inp[i]);
    for (i=0;i<128;i++) inp[i]='0';
    printf("\n");
     */  
    //board.MovementCount=0;
    if (strstr(inp, "isready") != NULL) {printf("readyok\n"); fflush(stdout);}
   
    if (strstr(inp, "quit") != NULL) return 0;
    
    if (strstr(inp, "position") !=NULL) {
        if (strstr(inp, "startpos" ) != NULL) setup_board(1);
        else fehn2board(inp);
        }
    //if (strstr(inp, "moves") !=NULL) read_movelines(inp);
    
    if (strstr(inp, "show") !=NULL) show_board(board.squares);
    
    if (strstr(inp, "quit") !=NULL) {//sleep(1);
      break;}
    
    if (strstr(inp, "new") != NULL) setup_board(1);

    if (strstr(inp, "reorder") != NULL) reorder_movelist(&moves);
    
    if (strstr(inp, "test") != NULL) fehn2board(testfehn);
    
    if (/*strstr(inp, "black") != NULL*/strstr(inp, "white") != NULL) 
        machineplays = 0;
    
    if (strstr(inp, "white") == NULL && strstr(inp, "black") != NULL)
        machineplays = 1; 
    
    if (strstr(inp, "remove") != NULL) history_rollback(2);
    
    if (strstr(inp, "dump") != NULL) dump_history();

    if (strstr(inp, "count") != NULL) printf("%i pieces.\n", countPieces(board.squares, 0));
    
    if (strstr(inp, "go") != NULL) {
      machineplays = board.whoplays;
      computer(thinkVerbose);
    }

    if (strstr(inp, "isatk") != NULL) {
      int vI = inp[6] - '0';
      int vJ = inp[8] - '0';
      int w = ifsquare_attacked(board.squares, vI, vJ, machineplays, 0 , 0);
      printf("%i %i is attacked by %i\n", vI, vJ, w);

    }
    if (strstr(inp, "isxray") != NULL) {
      int vI = inp[7] - '0';
      int vJ = inp[9] - '0';
      int w = ifsquare_attacked(board.squares, vI, vJ, machineplays, 1 , 0);
      printf("%i %i is attacked by %i\n", vI, vJ, w);
    }

    if (strstr(inp, "eval") != NULL)
      {

	int defenderMatrix[2][8][8];
	GenerateDefenderMatrix(board.squares, defenderMatrix);

	int eP = board.whoplays;
	legal_moves(&board, &moves, eP, 0);
	printf("KAD = %i\n", moves.kad);
	int Ps = evaluate(&board, &moves, defenderMatrix, eP, eP, 1);
	legal_moves(&board, &moves, 1-eP, 0);
	int Es = evaluate(&board, &moves, defenderMatrix, 1-eP, eP, 1);	
	printf("A=%i; s=%i;   Attacker score = %i    Defender score = %i.\n",
	       eP, 0, Ps,Es, board.score);
	       
      }

    

    
    if (strstr(inp, "history") != NULL) {
        printf("move history: %i moves.\n", hindex);
        for (i=0; i < hindex; i++) {
            print_movement(&movehistory[i],0);
        }
    }

    if (strstr(inp, "load") != NULL) {
      char *reading = strtok(inp, " ");
      reading = strtok(NULL, " ");

      if (reading == '\0') continue;
      if (strstr(reading, ".mac") != NULL)
	sprintf(specificMachine, "%s", reading);
      else
	sprintf(specificMachine, "any");
      setBrainStandardValues();
      loadmachine(0, machinepath);
      printf("loading  machine %s on runtime\n", reading);

    }
    

    
    /*if (strstr(inp, "usermove") !=NULL) { 
        if (read_movelines(inp) > 0) {
            computer(1);
        }
    }*/

    if (strstr(inp, "list") !=NULL)  {
        legal_moves(&board, &moves, 0, 0);

    }   
             
    if (strstr(inp, "lis1") !=NULL)  {
        legal_moves(&board,&moves, 1, 0);

    }   

    if (strstr(inp, "showlist") !=NULL) {
      show_movelist(&moves);

    }
 
    if(strstr(inp, "echo") !=NULL) {
        write(1, output, strlen(output));fflush(stdout);
    }
    
    if (read_movelines(inp, 0)) {
       computer(thinkVerbose);
        

    }

    
    for (i=0;i<128;i++) inp[i]=0;

   
}   
    return 0;
}

void computer(int verbose) {
    int P = machineplays;
    Vb printf("thinking for %i\n",P); fflush(stdout);
    

    struct move move;

    if (fastmode) {
      if (think_fast(&move, P , Brain.DEEP, verbose) < 0) {
        printf("Checkmate.\n");return;}
    }
    else
      if (think(&move, P , Brain.DEEP, verbose) < 0) {
	printf("Checkmate.\n");return;}
    
   

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


Global void setBrainStandardValues(void) {
               
  //iniatializing variables with standard values.
    
  //pvalues is the value of each piece in centipawns. 
  //order is pawn-rook-knight-bishop-queen-king.
  Brain.pvalues[0] = 100;
  Brain.pvalues[1] = 500;
  Brain.pvalues[2] = 320;
  Brain.pvalues[3] = 340;
  Brain.pvalues[4] = 900;
  Brain.pvalues[5] = 2000;
    
  //randomness is the limit to the randomic small variability on the score, 
  //in centipawns.
  Brain.randomness = 50;
  
  //seekmiddle augments the score for pieces in the center of the board.
  Brain.seekmiddle = 0;
    

  //seekpieces modifies pieces' raw material value.
  Brain.seekpieces = 1;
    
  //seekatk augments the score for each piece that is under emminent attack.
  Brain.seekatk = 0;

  Brain.presumeOPPaggro = 0;
  
  //pawnrankMOD augments the score of the pawns, by the rank he occupies.
  Brain.pawnrankMOD = 0;

  // parallelAttacks adds score when a enemy occupied square
  // is being attacked by multiple pieces.
  Brain.parallelAttacks = 0;

  
  Brain.balanceoffense = 0;    
  Brain.cumulative = 0;
  Brain.MODbackup = 0;
  Brain.MODmobility = 0;

  Brain.moveFocus = 0;

  // 
  Brain.boardcontrol = 0;
  
  // modifies score for various situations that happen only in endgame time.
  Brain.endgameWeight = 0;

  // this add bonus material value for pieces under opponent control,
  // the idea is to make the engine avoid trading pieces.
  Brain.opponentAddMaterialValue = 0;

  Brain.kingPanic = 0;
  Brain.pawnIssue = 0;
}

