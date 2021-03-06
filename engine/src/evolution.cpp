#include "lampreia.h"

unsigned long long rndseed() {
    unsigned int lo,hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((unsigned long long)hi << 32) | lo;
}

int loadmachine (int verbose, char *MachineDir) {
      
       FILE *MachineListFile;
       FILE *MachineFile;
       //FILE * flist;
       char * line = NULL;
       size_t len = 0;
       ssize_t read;
       
       char *reading = (char *) malloc(128 * sizeof(char));
       char *filename = (char *) malloc(128 * sizeof(char));
       
       int V = 1;//verbose;
       
       int Nmachines = 0;
       int selected_machine_index = 0;
       char ch= '0';
       int i=0;


       
       if (HallOfFameMode)
	 sprintf(MachineDir, "%s/halloffame", MachineDir);

       // to read entire machine dir, and randomize one to load;
       if (strstr(specificMachine, ".mac") == NULL) {
	 /*
       sprintf(filename, "%s/machines.list", MachineDir);
  
       
       printf("loaded machine list:  %s\n", filename);
       

       MachineListFile = fopen(filename, "r");
       if (!MachineListFile) {
	 printf("Failed to reach [machines.list].\n");
	 return 1;
       }
       while(!feof(MachineListFile)) {
	 ch = fgetc(MachineListFile);
	 if(ch == '\n') Nmachines++;}
       
       rewind(MachineListFile);
       
       srand ( rndseed() );
       Nchosenmachine = rand() % Nmachines;

       Vb printf("number of machines on list: %i    (%ith was chosen)\n",
		 Nmachines,Nchosenmachine);
       
       
       
       for (i=0;i<=Nchosenmachine;i++) read = getline(&line, &len, MachineListFile);
       
       fclose(MachineListFile);
       
       printf("MACname > %s\n", line);
       strtok(line, "\n");
       sprintf(filename, "%s/%s", MachineDir, line);*/
	 
	 int MACINDEX=0;
	 char **machinelist;
	 machinelist = (char **)malloc(10000 * sizeof(char *));
	 F (i, 128)
	   machinelist[i] = (char *)malloc(32 * sizeof(char));
	 struct dirent *dp;
	 DIR *openDIR = opendir(MachineDir);
	 if(openDIR != NULL) {
	   while((dp = readdir(openDIR)) != NULL)
	     if (strstr(dp->d_name, ".mac") != NULL)
	       {
		 machinelist[MACINDEX] = dp->d_name;
		 MACINDEX++;
		 //printf("%s\n", dp->d_name);
	       }
	   closedir(openDIR);
	 }
	 
	 // EMPTY DIR!
	 if (!MACINDEX)
	   {
	     free(reading);
	     free(filename);
	     return 1;
	   }
	 
	 srand ( rndseed() );
	 selected_machine_index = rand() % MACINDEX;

	 Vb printf("number of machines on list: %i    (%ith was chosen)\n",
		   MACINDEX, selected_machine_index);
       
	 sprintf(filename, "%s/%s", MachineDir, machinelist[selected_machine_index]);
	 MachineListFile = fopen(filename, "r");
       
       

	 //       F(i, 128)
	 // free(machinelist[i]);
	 free(machinelist);

	 
       }

       // to load specific user-defined machine;
       else {
	 sprintf(filename, "%s/%s", MachineDir, specificMachine);
 	 printf("Loading user defined machine.\n");
	 }
       

	
       
       printf("opening machine: %s\n", filename);

       // printf("MACname > %s\n",line);
       MachineFile = fopen(filename, "r");
       if (MachineFile == NULL) {
	 printf("ERROR: failed to load machine [null file].\n");
	 return 0;
	 //exit(EXIT_FAILURE);
	 }
	 printf("\n");
	 while ((read = getline(&line, &len, MachineFile)) != -1) {
           i=0;
           
           Vb printf("Retrieved line of length %zu :\n", read);
           Vb printf("%s", line);
           
           
           if (strstr(line, "pvalues")!=NULL) {
	    reading = strtok(line, " ");   
            reading = strtok(NULL, " ");
            reading = strtok(NULL, " ");
           
            Vb printf("piecevalue is ");
            
            
            for (i=0;i<6;i++) {if (!atoi(reading)) exit(0); 
            Brain.pvalues[i] = atoi(reading); reading = strtok(NULL, " "); 
            Vb printf("P%i worth %i /n",i,Brain.pvalues[i]);}

                      
           }

	   /*readparam(line, V, "eval_randomness", &Brain.randomness);
	   readparam(line, V, "param_seekpieces", &Brain.seekpieces);
	   readparam(line, V, "param_seekmiddle", &Brain.seekmiddle);
	   readparam(line, V, "param_seekatk", &Brain.seekatk);
	   readparam(line, V, "param_evalmethod", &Brain.evalmethod);
	   readparam(line, V, "param_presumeOPPaggro", &Brain.presumeOPPaggro);
	   readparam(line, V, "param_pawnrankMOD", &Brain.pawnrankMOD);
	   readparam(line, V, "param_parallelAttacks", &Brain.parallelAttacks);
	   readparam(line, V, "param_balanceoffense", &Brain.balanceoffense);
	   readparam(line, V, "param_cumulative", &Brain.cumulative);
	   readparam(line, V, "param_MODbackup", &Brain.MODbackup);
	   readparam(line, V, "param_MODmobility", &Brain.MODmobility);
	   readparam(line, V, "param_moveFocus", &Brain.moveFocus);
	   readparam(line, V, "param_boardcontrol", &BRAIN.boardcontrol);
	   readparam(line, V, "param_endgameWeight", &BRAIN.endgameWeight);
	   readparam(line, V, "param_opponentAddMaterialValue", &BRAIN.opponentAddMaterialValue);
	   readparam(line, V, "param_kingPanic", &BRAIN.kingPanic);

	   readparam(line, V, "param_pawnIssue", &BRAIN.pawnIssue);
	   readparam(line, V, "param_seekInvasion", &BRAIN.seekInvasion);
	   readparam(line, V, "param_offensevalue", &BRAIN.offensevalue);
	   readparam(line, V, "param_freepiecevalue", &BRAIN.freepiecevalue);
	   readparam(line, V, "param_limitDefender", &BRAIN.limitDefender);
	   readparam(line, V, "param_parallelAttacker", &BRAIN.parallelAttacker);
	   readparam(line, V, "param_castlebonus", &BRAIN.castlebonus);
	   
	   readparam(line, V, "param_kingAreaPanic", &BRAIN.kingAreaPanic);
	   readparam(line, V, "param_kingAreaTower", &BRAIN.kingAreaTower);
	   readparam(line, V, "param_kingAreaSecure", &BRAIN.kingAreaSecure);
	   readparam(line, V, "param_pawnSafeMarch", &BRAIN.pawnSafeMarch);*/
	   
#include "include_parameterReader"

	 }
	 printf("\n");
	 fclose(MachineFile);
      
       
	 //machinepath = filename;
	 
	 printf("machinepath>> %s\n", filename);
	 
	 loadedmachine = true;
	 
	 if (againstHUMAN)
	   chesslog(MachineDir, machinepath);

	 free(reading);
	 free(filename);
	 return 0;
	 
    
}


void readparam(char *line, int verbose, const char *keyword, float *Parameter) {
    
  if (strstr(line, keyword) == NULL) return;
    char *reading = (char *) malloc(64);
    
               
               reading = strtok(line, " ");
               reading = strtok(NULL, " ");
               reading = strtok(NULL, " ");
               float parameter = (float)(atof(reading));
               Vb printf("%s is %g\n",keyword,parameter);
    
    reading = NULL;
    *Parameter = parameter;
    

}

void dump_history() {
    int i=0;
    
    printf("STARTING MOVE HISTORY DUMP.\b");
    for (i=0;i<hindex;i++) {

        printf("%i     [%i]\n", i, machineplays);
        print_movement(&movehistory[i],1);
        show_board(boardhistory[i].squares);
	show_castling_status(&boardhistory[i]);

    }
    
    
}

void chesslog(char *location, const char content[]) {
    //printf("saving log. %s\n", content);
    
    sprintf(location, "%s/engine_log", location);
    FILE *logfile = fopen(location, "a");
    fprintf(logfile, ">> %s\n", content);
    fclose(logfile);
            
            
}

void show_castling_status(struct board *board)
{

  asprintf(&output, "WHITE R%i  K%i  R%i        BLACK r%i    k%i    r%i\n",
	   board->castle[0][0],
	   board->castle[0][1],
	   board->castle[0][2],
	   board->castle[1][0],
	   board->castle[1][1],
	   board->castle[1][2]);
	write(2, output, strlen(output));



}
