#include "ev_chess.h"





unsigned long long rndseed(){
    unsigned int lo,hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((unsigned long long)hi << 32) | lo;
}

int loadmachine (int verbose, char *MachineDir) {
  
    
    
       FILE * fp;
       //FILE * flist;
       char * line = NULL;
       size_t len = 0;
       ssize_t read;
       char *reading = (char *) malloc(128 * sizeof(char));
       
       char *filename = (char *) malloc(128 * sizeof(char));
       
       int V = 1;//verbose;
       
       int Nmachines = 0;
       int Nchosenmachine = 0 ;
       char ch= '0';
       int i=0;
       
       
       if (selectTOPmachines)
	 sprintf(MachineDir, "%s/top_machines", MachineDir);


       if (strstr(specificMachine, ".mac") == NULL) {
       sprintf(filename, "%s/machines.list", MachineDir);
  
       
       printf("loaded machine list:  %s\n", filename);
       

       fp = fopen(filename, "r");

       while(!feof(fp)) {ch = fgetc(fp); if(ch == '\n') Nmachines++;}
       fclose(fp);

 
       
       fp = fopen(filename, "r");
        srand ( rndseed() );
       Nchosenmachine = rand() % Nmachines;

       Vb printf("number of machines on list: %i    (%ith was chosen)\n",
		 Nmachines,Nchosenmachine);
       
       
       
       for (i=0;i<=Nchosenmachine;i++) read = getline(&line, &len, fp);
       
       fclose(fp);
       
       printf("MACname > %s\n", line);
       strtok(line, "\n");
       sprintf(filename, "%s/%s", MachineDir, line);
       }

       // loading user-defined machine!

       else {
	 sprintf(filename, "%s/%s", MachineDir, specificMachine);
 	 printf("ok but plans changed as we loaded user-defined machine!  %s\n", filename);
	 }
       

	
       
       printf("opening machine: %s\n", filename);

       // printf("MACname > %s\n",line);
       fp = fopen(filename, "r");
       if (fp == NULL)
           exit(EXIT_FAILURE);
       
       while ((read = getline(&line, &len, fp)) != -1) {
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
           
           
        if(loadDEEP) if (strstr(line, "param_DEEP") != NULL)   
            Brain.DEEP = readparam(line, V);
           
        if (strstr(line, "param_deviationcalc") != NULL)
            Brain.deviationcalc = readparam(line, V);
        if (strstr(line, "eval_randomness") != NULL)
            Brain.randomness = readparam(line, V);
        if (strstr(line, "param_seekpieces") != NULL)
            Brain.seekpieces = readparam(line, V);
        if (strstr(line, "param_seekmiddle") != NULL)
            Brain.seekmiddle = readparam(line, V);
        if (strstr(line, "param_seekatk") != NULL)
            Brain.seekatk = readparam(line, V);
        if (strstr(line, "param_evalmethod") != NULL)
            Brain.evalmethod = readparam(line, V);
        if (strstr(line, "param_presumeOPPaggro") != NULL)
            Brain.presumeOPPaggro = readparam(line, V);
        if (strstr(line, "param_pawnrankMOD") != NULL)
            Brain.pawnrankMOD = readparam(line, V);
        if (strstr(line, "param_parallelcheck") != NULL)
            Brain.parallelcheck = readparam(line, V);
        if (strstr(line, "param_balanceoffense") != NULL)
            Brain.balanceoffense = readparam(line, V);
        if (strstr(line, "param_cumulative") != NULL)
            Brain.cumulative = readparam(line, V);          
        if (strstr(line, "param_MODbackup") != NULL)
            Brain.MODbackup = readparam(line, V);                
        if (strstr(line, "param_MODmobility") != NULL)
            Brain.MODmobility = readparam(line, V);                
           

           
           
           printf(".\n");
           
           
       }

       fclose(fp);
       if (line)
           free(line);
       
       
       machinepath = filename;
       applyresult(5);
       printf("machinepath>> %s\n", machinepath);
       
       loadedmachine = true;
       
       if (againstHUMAN) 
	 chesslog(MachineDir, machinepath);
       

       if (!Brain.randomness)
	 Brain.randomness = 10;
      
       return 0;
   
    
}

int applyresult (int result) {
    if (loadedmachine == false) return 0;
    snprintf(output, 32,"machinepath>> %s\n", machinepath);
    write(1,output,strlen(output));
    
    FILE *file = fopen(machinepath, "a");
    
    if (!againstHUMAN) {
    if (result==1) fprintf(file, "\nW\n");
    else if (result==-1) fprintf(file,"\nL\n");
    else if (result==0) fprintf(file, "\nx\n");
    else fprintf(file, "\nD\n");
 
    
    
    fprintf(file, "K = %i\n",countpieces());
    }
    
    else {
    if (result==1) fprintf(file, "\nHW\n");
    else if (result==-1) fprintf(file,"\nHL\n");
    else if (result==0) fprintf(file, "\nHx\n");
    else fprintf(file, "\nHD\n"); 
        
        
        
    }
    
    fclose(file);
    return 1;
}    

int countpieces(void) {
    int i=0;
    int j=0;
    int piece=0;
    
    int Tvalue=0;
    
    int Vpieces[6]={1,5,3,3,9,10};
    
    
    
    for (i=0;i<8;i++) for (j=0;j<8;j++) {
        
        piece = is_in(board.squares[i][j],pieces[machineplays],6);
        
        if (piece>-1) Tvalue=Tvalue+Vpieces[piece];
                    
        
        
        piece = is_in(board.squares[i][j],pieces[1-machineplays],6);
        
        if (piece>-1) Tvalue=Tvalue-Vpieces[piece];
                    
       
    }
    
    return Tvalue;
}

float readparam(char *line, int verbose) {
    
    //if (strstr(line, keyword) != NULL) {
    char *reading = (char *) malloc(64);
    
               
               reading = strtok(line, " ");
               reading = strtok(NULL, " ");
               reading = strtok(NULL, " ");
               float parameter = (float)(atof(reading));
               Vb printf("is %f\n",parameter);
    
   
    
    
    
    reading = NULL;
    return parameter;


}

void dump_history() {
    int i=0;
    
    printf("STARTING MOVE HISTORY DUMP.\b");
    for (i=0;i<hindex;i++) {

        printf("%i     [%i]\n", i, machineplays);
        print_movement(&movehistory[i],1);
        show_board(movehistoryboard[i]);
    }
    
    
}

void chesslog(char *location, const char content[]) {
    //printf("saving log. %s\n", content);
    
    sprintf(location, "%s/engine_log", location);
    FILE *logfile = fopen(location, "a");
    fprintf(logfile, ">> %s\n", content);
    fclose(logfile);
            
            
}
