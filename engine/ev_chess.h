/* 
 * File:   board.h
 * Author: gabs
 *
 * Created on September 21, 2015, 12:44 AM
 */


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
#define	EV_CHESS_H


#define onboard(i,j) i >=0 && i < 8 && j >= 0 && j <8
#define comp_arr(a,b) (a[0] == b[0] && a[1] == b[1])
#define print_play(p) printf("from %c%c to %c%c\n", p[0][0],p[0][1],p[1][0],p[1][1])
#define print_play_cord(p) printf("from %c%c to %c%c\n", p.from[0],p.from[1],p.to[0],p.to[1])
#define expand_play(p) p.from[0],p.from[1],p.to[0],p.to[1]


#define Vb if (verbose)





struct move;
struct move {
    char from[2];
    char to[2];
    
    int iscastle;
    int lostcastle;
    
    long score;
    
    char promoteto;
    
    char casualty;
    

    
};


struct board;
   struct board {
      char squares[8][8];
      struct move movelist[128];
      int k;
      
      char attackers[64][3];
      char defenders[64][3];
      int kad;
      
      int mobility[2];
      
      int castle[2][3];

   }; 
   
  
   
struct param;
   struct param {
    int pvalues[6];
    float randomness;
    float seekmiddle;
    float DEEP;
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
   };

extern struct move movehistory[512];
extern char movehistoryboard[512][8][8];
extern int hindex;   

using namespace std;

//extern char squares[8][8];
extern char pieces[2][6];

extern bool computer_turn;

extern struct board board;

extern struct param Brain;

extern int machineplays;
extern bool loadedmachine;

extern bool selectTOPmachines;


extern char *infoMOVE;

/*variable params for intelligent evolution*/

/*//////variable params for intelligent evolution*/


extern char *infomoveTABLE[2048];
extern int  infomoveINDEX;

extern char *output;

extern bool show_info;

extern int machineplays;

extern char *machinepath;

extern bool againstHUMAN;
extern bool toloadmachine;


extern bool allow_castling;
/*functions from main.cpp*/

void computer(int verbose);
void SIGthink(int signum);
    
/*functions from board.cpp*/
void setup_board(int setup);
void show_board(char squares[8][8]);
int legal_moves (struct board *board, int PL, int verbose);
int mpc (char squares[8][8], int i, int j, int player);
void move_pc(struct board *tg_board, struct move *movement);
void undo_move(struct board *tg_board, struct move *movement);
void attackers_defenders (struct board *board,int P);
void h_move_pc (struct board *board,char movement[][2]);
int history_append(struct move *move);
int history_rollback(int times);
//void castle (struct board *board, int doundo, int PL, int side);
int findking (char board[8][8], char YorX, int player);
int cancastle (struct board *board, int P, int direction);
void movement_generator(struct board *board, int limit, char direction, int i, int j, int P);

/*functions from operation.cpp*/

void cord2pos (char out[]); 
void pos2cord (char out[]);
int parse_move (struct move *target, char *s, int P);
bool is_in(char val, char arr[], int size);
bool is_legal(struct move *play, int P);
int append_move(struct board *board, int i,int j, int mod_i, int mod_j, int P);
//void erase_moves(struct board *tgt, int eraseall);
void print_movement (struct move *move);

int ifsquare_attacked (char squares[8][8], int TGi, int TGj, int P, int verbose) ; 
int check_move_check (struct board *tg_board, struct move *move, int P);

int fehn2board (char str[]);

int read_movelines (char txt[128], int verbose);
int getindex (char x, char array[],int size);
struct board *makeparallelboard (struct board *board);
void select_top (struct move *array, int size, int target[], int quant);
void replicate_move(struct move *target, struct move *source) ;
//void freeboard (struct board *target);
int power(int base, unsigned int exp);

void reorder_movelist(struct board *board); 

void eval_info_move(struct move *move, int DEEP, int P);

/*functions from brain.cpp*/

int think (struct move *out, int PL, int DEEP, int verbose);
int evaluate(struct board *evalboard, int PL);
long thinkiterate(struct board *feed, int PL, int DEEP, 
        long chainscore, long Alpha, long Beta);
float scoremod (int DEEP, int method);




/*functions from evolution.cpp*/

int loadmachine (int verbose, char *dir);
int applyresult (int result);
int countpieces (void);
float readparam(char *line, int verbose);
void dump_history();

#endif	/* BOARD_H */