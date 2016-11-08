/* 
 * File:   board.h
 * Author: gabs
 *
 * Created on September 21, 2015, 12:44 AM
 */
//#define __CUDACC__

#ifndef EV_CHESS_H
#include <cstdlib>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>    
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <math.h>

#define DUMP(B) if(B!=NULL){free(B);B=NULL;}

#define onboard(i,j) i >=0 && i < 8 && j >= 0 && j <8
#define comp_arr(a,b) (a[0] == b[0] && a[1] == b[1])
#define print_play(p) printf("from %c%c to %c%c\n", p[0][0],p[0][1],p[1][0],p[1][1])
#define print_play_cord(p) printf("from %c%c to %c%c\n", p.from[0],p.from[1],p.to[0],p.to[1])
#define expand_play(p) p.from[0],p.from[1],p.to[0],p.to[1]
#define forsquares for(i=0;i<8;i++) for(j=0;j<8;j++)
#define flip(x) x = 1 - x
#define invert(x) x = -x;

#define F(v,x) for (v=0;v<x;v++)

#define INFINITE 9999000;

#define Vb if (verbose)

#ifdef __CUDACC__
#define Host __host__
#define Device __device__
#define Global __global__
#else
#define Host 
#define Device  
#define Global  
#endif

#ifdef __CUDA_ARCH__
#define IFnotGPU(p)  
#define IFGPU(p) p
#define Machineplays GPUmachineplays
#define Pieces GPUpieces
#define BRAIN GBrain
#else
#define IFGPU(p)   
#define IFnotGPU(p) p
#define Machineplays machineplays
#define Pieces pieces
#define BRAIN Brain
#endif

struct move;
struct move {
    char from[2];
    char to[2];
    
    int iscastle;
    int lostcastle;
    
    long score;
    
    char promoteto;
    
    char casualty;
    
    int passantJ[2];
    int passant;
};

struct movelist;
struct movelist {
    struct move movements[128];
    char attackers[64][3];
    char defenders[64][3];
    
    int k;
    int kad;
    
    int mobility;
};

struct board;
   struct board {
     char squares[8][8];
      
     int castle[2][3];
      
     int passantJ;
     int whoplays;
     
     long score;
     
     int MovementCount;
     int betaCut;
     struct move movements[64];
   }; 
   
  
   
struct param;
struct param {
  int DEEP;
  int xDEEP;
  int yDEEP;
  int pvalues[6];
  
  float randomness;
  float seekmiddle;
  float seekpieces;
  float deviationcalc;
  float evalmethod;
  float seekatk;
  float TIMEweight[10];
  float presumeOPPaggro;
  float pawnrankMOD;
  float parallelcheck;
  float balanceoffense;
  float cumulative;
  float MODbackup;
  float MODmobility;
  float moveFocus;
  float boardcontrol;
  float endgameWeight;
};

extern struct move movehistory[512];
extern char movehistoryboard[512][8][8];
extern int hindex;   

using namespace std;

//extern char squares[8][8];
extern Device char GPUpieces[2][6];
extern char pieces[2][6];

extern bool computer_turn;

extern struct board board;

extern struct param Brain;
IFGPU(extern __device__ struct param GBrain;)


extern bool loadedmachine;

extern bool selectTOPmachines;
extern const float BoardMiddleScoreWeight[8];


extern char *infoMOVE;

/*variable params for intelligent evolution*/

/*//////variable params for intelligent evolution*/


extern char *infomoveTABLE[2048];
extern int  infomoveINDEX;

extern char *output;

extern bool Show_Info;
extern bool show_info;

extern int machineplays;
extern Device int GPUmachineplays;

extern char *machinepath;

extern bool againstHUMAN;

extern bool toloadmachine;
extern char *specificMachine;

extern bool thinkVerbose;
extern bool fastmode;
extern bool loadDEEP;

extern int searchNODEcount;
IFnotGPU( extern bool allow_castling; )
IFGPU( extern __device__ bool allow_castling; )
//functions from main.cpp;#######################################################
void computer(int verbose);
void SIGthink(int signum);
Global void setBrainStandardValues(void);

//functions from board.cpp;######################################################
void setup_board(int setup);
void show_board(char squares[8][8]);
Host Device int legal_moves 
(struct board *board, struct movelist *moves, int PL, int verbose);
Host Device int mpc (char squares[8][8], int i, int j, int player);
Host Device void move_pc(struct board *tg_board, struct move *movement);
Host Device void undo_move(struct board *tg_board, struct move *movement);
Device void attackers_defenders (char squares[8][8], struct movelist moves, int P);
void h_move_pc (struct board *board,char movement[][2]);
int history_append(struct move *move);
int history_rollback(int times);
//void castle (struct board *board, int doundo, int PL, int side);
Host Device int findking (char board[8][8], char YorX, int player);
Host Device int cancastle 
    (struct board *board, int P, int direction);
Host Device void movement_generator
    (struct board *board, struct movelist *moves, 
            int limit, char direction, int i, int j, int P);
Host Device void undo_lastMove(struct board *board, int Number);

//functions from operation.cpp;##################################################
void cord2pos (char out[]); 
void pos2cord (char out[]);

Host Device bool is_in(char val, char arr[], int size);
bool is_legal(struct move *play, int P);
Host Device int append_move
    (struct board *board, struct movelist *moves, 
        int i,int j, int mod_i, int mod_j, int P);
//void erase_moves(struct board *tgt, int eraseall);
Host Device int ifsquare_attacked (char squares[8][8],
				   int TGi, int TGj, int AttackingPlayer, int verbose); 
Host Device int check_move_check (struct board *tg_board, struct move *move, int P);
Host Device int getindex (char x, char array[],int size);

Host Device struct board *makeparallelboard (struct board *board);
Host Device void cloneboard (struct board *model, struct board *target);

Host Device void selectBestMoves (struct move *array, int size, int target[], int quant);
Host Device void replicate_move(struct move *target, struct move *source);

//void freeboard (struct board *target);
Host Device int power(int base, unsigned int exp);
Host Device void reorder_movelist(struct movelist *movelist); 

Host Device void movement_to_string(struct move *move, char *target);

Host Device int variableComparation(long A, long B, int startPlayer, int endPlayer);

//functions from interface.cpp;##################################################
int parse_move (struct move *target, char *s, int P);
void print_movement (struct move *move, int full);
int read_movelines (char txt[128], int verbose);
int fehn2board (char str[]);
void eval_info_move(struct move *move, int DEEP, time_t startT, int P);
void eval_info_group_move(struct move *primary, struct move *secondary,
			  int DEEP, time_t startT, int P);
void stdoutWrite(const char * text);
void show_moveline(struct board *finalboard, int bottom_span, time_t startT);

void show_movelist(struct movelist *moves);

//functions from brain.cpp;######################################################
int think (struct move *out, int PL, int DEEP, int verbose);
Device int evaluate(struct board *evalboard, struct movelist *moves, int P, int Attacker);
Device struct board *thinkiterate(struct board *feed, int DEEP, int verbose,
				  long Alpha, long Beta, int AllowCutoff);
Host Device float scoremod (int DEEP, int method);
Device int canNullMove (int DEEP, struct board *board, int K, int P);

Global void kerneliterate(struct board *workingboard,
			  struct movelist *mainmove,
			  int PL, int DEEP, int *Test, long *_Beta);

Global void Testkernel(int *Test);
Device void Testdevice(int *Test);
Global void evalkernel(long *VALUE, struct board *board, struct movelist *moves);
Device int satellite_evaluation (struct move *movement);

Device int compare_movements (struct move *move_A, struct move *move_B);
Device int check_fivemove_repetition (void);
//functions from brain_fast.cpp;#################################################
int think_fast(struct move *out, int PL, int DEEP, int verbose);
Device long thinkiterate_fast(struct board *_board, int DEEP, int verbose,
			      long Alpha, long Beta, int AllowCutoff);

//functions from evolution.cpp;
unsigned long long rndseed();
int loadmachine (int verbose, char *dir);
//int applyresult (int result);
//int countpieces (void);
void readparam(char *line, int verbose, const char *keyword, float *Parameter);
void dump_history();
void chesslog(char *location, const char content[]);

#define	EV_CHESS_H
#endif	/* BOARD_H */
