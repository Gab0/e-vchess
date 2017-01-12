#include "lampreia.h"

const char *Version = "v0.803";
struct board board;
struct param Brain;
struct movelist moves;

Device struct param GBrain;


const int MovementDiagonalLinear[2][4][2] = 
  { {	{1, -1}, {-1, 1}, {1, 1}, {-1, -1} },       
    {   {1, 0},  {-1, 0}, {0, 1}, {0, -1}  } };

char pieces[2][6] = {{'P','R','N','B','Q','K'},
                     {'p','r','n','b','q','k'}};
Device char GPUpieces[2][6] = {{'P','R','N','B','Q','K'},
                               {'p','r','n','b','q','k'}};
const float BoardMiddleScoreWeight[8] = {0, 0.33, 0.66, 1, 1, 0.66, 0.33, 0};
const float BoardInvaderScoreWeight[8] = {0, 0, 0, 0, 0.16, 0.27, 0.35, 0.43};
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
char movehistoryboard[512][64];
int hindex; 
//variable params for intelligent evolution (standards initialized);


/*//////variable params for intelligent evolution*/

int i;
char *infomoveTABLE[2048];

bool selectTOPmachines = false;

int  infomoveINDEX;
bool show_time_elapsed;
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
	if (strstr(argv[i], "--time") != NULL) show_time_elapsed = true;

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

    
    char testfehn[128] = "fen 5bkr/pp4p1/2p1Q3/3p3p/1P1P1P2/8/R1KBP1PP/5B1R w KQkq - 1 1";

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
        Machineplays = 0;
    
    if (strstr(inp, "white") == NULL && strstr(inp, "black") != NULL)
        Machineplays = 1; 
    
    if (strstr(inp, "remove") != NULL) history_rollback(2);
    
    if (strstr(inp, "dump") != NULL) dump_history();

    if (strstr(inp, "count") != NULL) printf("%i pieces.\n", countPieces(board.squares, 0));
    
    if (strstr(inp, "go") != NULL) {
      Machineplays = board.whoplays;
      computer(thinkVerbose);
    }

    if (strstr(inp, "isatk") != NULL) {
      int vI = inp[8] - '0';
      int vJ = inp[10] - '0';
      int eP = inp[6] - '0';
      if (-1 < eP < 2){
	int w = ifsquare_attacked(board.squares, vI, vJ, eP, 0 , 0);
	printf("%i %i is attacked by %i\n", vI, vJ, w);
      }
    }
    if (strstr(inp, "isxray") != NULL) {
      int vI = inp[7] - '0';
      int vJ = inp[9] - '0';
      int w = ifsquare_attacked(board.squares, vI, vJ, machineplays, 1 , 0);
      printf("%i %i is attacked by %i\n", vI, vJ, w);
    }

    if (strstr(inp, "eval") != NULL)
      {
	int AttackerDefenderMatrix[2][64];
	int BoardMaterialValue[64];

	int eP = inp[5] - '0';
	printf("evaluating position for player %i.\n", eP);
	GenerateAttackerDefenderMatrix(board.squares, AttackerDefenderMatrix);

	show_board_matrix(AttackerDefenderMatrix[0]);
	show_board_matrix(AttackerDefenderMatrix[1]);
	
	//int eP = board.whoplays;
	legal_moves(&board, &moves, eP, 0);
	printf("KAD = %i\n", moves.kad);
	
	int Ps = evaluateMaterial(&board,
				  BoardMaterialValue, AttackerDefenderMatrix,
				  eP, eP, 0);
	int Es = evaluateMaterial(&board,
				  BoardMaterialValue, AttackerDefenderMatrix,
				  1-eP, eP, 0);
	
	Ps += evaluateAttack(&moves,
			     BoardMaterialValue, AttackerDefenderMatrix,
			     eP, eP, 1);
	
	legal_moves(&board, &moves, 1-eP, 0);
	
	Es += evaluateAttack(&moves,
			     BoardMaterialValue, AttackerDefenderMatrix,
			     1-eP, eP, 1);
	printf("A=%i; s=%i;   Attacker score = %i    Defender score = %i.\n",
	       eP, 0, Ps,Es, board.score);
	show_board_matrix(BoardMaterialValue);
      }
    
    if (strstr(inp, "tmove") != NULL)
      {
	int IDX = inp[6] - '0';
	print_movement(&moves.movements[IDX], 1);
	move_piece(&board, &moves.movements[IDX], 1);
	move_piece(&board, &moves.movements[IDX], -1);
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
      int pList = inp[5] - '0';
      legal_moves(&board, &moves, pList, 0);
      show_movelist(&moves);
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
    int P = Machineplays;
    Vb printf("thinking for %i\n",P); fflush(stdout);
    

    struct move move;
 

    time_t ThinkingTime = time(NULL);

    if (fastmode) {
      if (think_fast(&move, P , Brain.DEEP, verbose) < 0) {
        printf("Checkmate.\n");return;}
    }
    else
      if (think(&move, P , Brain.DEEP, verbose) < 0) {
	printf("Checkmate.\n");return;}
    

    if(show_time_elapsed)
      printf("Thinking took %ld seconds.\n", time(NULL) - ThinkingTime);

    move_piece(&board, &move, 1);
    
 
    history_append(&move);    
    char moveFROM[2] = { SQR_I(move.from), SQR_J(move.from) };
    char moveTO[2] = { SQR_I(move.to), SQR_J(move.to) };

    cord2pos(moveFROM);		     
    cord2pos(moveTO);
    
    //Vb show_board(board.squares);
    
          
     if (move.promoteto != 0)  
         snprintf(output, 32 ,"move %c%c%c%cq\n",
                 moveFROM[0],moveFROM[1],moveTO[0],moveTO[1]);
     else snprintf(output, 32 ,"move %c%c%c%c \n", 
                 moveFROM[0],moveFROM[1],moveTO[0],moveTO[1]);
     
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
  Brain.seekInvasion = 0;

  Brain.scoreFlutuabilityContinuator = 0.7;

  Brain.freepiecevalue = 0;
  Brain.offensevalue = 0;
}

