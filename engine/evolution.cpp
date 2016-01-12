


#include "ev_chess.h"


int loadmachine (int verbose, char *dir) {
  
    
    
       FILE * fp;
       //FILE * flist;
       char * line = NULL;
       size_t len = 0;
       ssize_t read;
       char *reading = (char *) malloc(64);
       
       char *filename = (char *) malloc(64);
       
       
       
       int Nmachines = 0;
       int Nchosenmachine = 0 ;
       char ch= '0';
       int i=0;
       
       
       if (selectTOPmachines) sprintf(dir, "%s/top_machines", dir);
       
       sprintf(filename, "%s/machines.list", dir);

       
       
       printf("ok  %s\n,", filename);
       
       
       fp = fopen(filename, "r");

       while(!feof(fp)) {ch = fgetc(fp); if(ch == '\n') Nmachines++;}
       fclose(fp);

       
       
       fp = fopen(filename, "r");
        srand ( time(NULL) );
       Nchosenmachine = rand() % Nmachines;

       Vb printf("number of machines on list: %i    (%ith was chosen)\n", Nmachines,Nchosenmachine);
       
       
       
       for (i=0;i<=Nchosenmachine;i++) read = getline(&line, &len, fp);
       
       fclose(fp);
       
       Vb printf("line > %s   /len > %i\n",line,len);
       strtok(line, "\n");
       sprintf(filename, "%s/%s", dir, line);
       
       
       Vb printf("opening machine: %s\n",filename);
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
            pvalues[i] = atoi(reading); reading = strtok(NULL, " "); 
            Vb printf("P%i worth %i /n",i,pvalues[i]);}

                      
           }
           
           if (strstr(line, "param_DEEP") != NULL) {
               reading = strtok(line, " ");
               reading = strtok(NULL, " ");
               reading = strtok(NULL, " ");
               param_DEEP = atoi(reading);
               Vb printf("param_DEEP is %i.\n", param_DEEP);}
           
           if (strstr(line, "param_deviationcalc") != NULL) {
               reading = strtok(line, " ");
               reading = strtok(NULL, " ");
               reading = strtok(NULL, " ");
               param_deviationcalc = atof(reading);
               Vb printf("param_deviationcalc is %f.\n", param_deviationcalc);}
           
           if (strstr(line, "eval_randomness") != NULL) {
               reading = strtok(line, " ");
               reading = strtok(NULL, " ");
               reading = strtok(NULL, " ");
               eval_randomness = atoi(reading);           
               Vb printf("eval_randomness is %i.\n", eval_randomness);}
           
           if (strstr(line, "param_aperture") != NULL) {
               reading = strtok(line, " ");
               reading = strtok(NULL, " ");
               reading = strtok(NULL, " ");
               param_aperture = atoi(reading);           
               Vb printf("eparam_aperture is %i.\n", param_aperture);}
           
           if (strstr(line, "param_seekpieces") != NULL) {
               reading = strtok(line, " ");
               reading = strtok(NULL, " ");
               reading = strtok(NULL, " ");
               param_seekpieces = atoi(reading);
               Vb printf("param_seekpieces is %i.\n", param_seekpieces);}
           
           
           if (strstr(line, "param_seekmiddle") != NULL) {
               reading = strtok(line, " ");
               reading = strtok(NULL, " ");
               reading = strtok(NULL, " ");
               param_seekmiddle = atof(reading);
               Vb printf("param_seekmiddle is %f\n", param_seekmiddle);}
           
           if (strstr(line, "param_evalmethod") != NULL) {
               reading = strtok(line, " ");
               reading = strtok(NULL, " ");
               reading = strtok(NULL, " ");
               param_evalmethod = atoi(reading);
               Vb printf("param_evalmethod is %i\n", param_evalmethod);}           
           
           if (strstr(line, "param_seekatk") != NULL) {
               reading = strtok(line, " ");
               reading = strtok(NULL, " ");
               reading = strtok(NULL, " ");
               param_seekatk = atoi(reading);
               Vb printf("param_seekatk is %i\n", param_seekatk);}
           
           if (strstr(line, "param_presumeOPPaggro") != NULL) {
               reading = strtok(line, " ");
               reading = strtok(NULL, " ");
               reading = strtok(NULL, " ");
               param_presumeOPPaggro = atof(reading);
               Vb printf("param_presumeOPPaggro is %f\n", param_presumeOPPaggro);}   

           if (strstr(line, "param_pawnrankMOD") != NULL) {
               reading = strtok(line, " ");
               reading = strtok(NULL, " ");
               reading = strtok(NULL, " ");
               param_pawnrankMOD = atof(reading);
               Vb printf("param_pawnrankMOD is %f\n", param_pawnrankMOD);}  
           
           printf(".\n");
           
           
       }

       fclose(fp);
       if (line)
           free(line);
       
       
       machinepath = filename;
       applyresult(5);
       printf("machinepath>> %s\n", machinepath);
       
       loadedmachine = true;
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
    else if (result==5) fprintf(file, "\nx\n");
    else fprintf(file, "\nD\n");
 
    
    
    fprintf(file, "K = %i\n",countpieces());
    }
    
    else {
    if (result==1) fprintf(file, "\nHW\n");
    else if (result==-1) fprintf(file,"\nHL\n");
    else if (result==5) fprintf(file, "\nHx\n");
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
        
        if (is_in(board.squares[i][j],pieces[machineplays],6)) {
            piece=getindex(board.squares[i][j],pieces[machineplays],6);
            Tvalue=Tvalue+Vpieces[piece];
                    
        }
        
        if (is_in(board.squares[i][j],pieces[1-machineplays],6)) {
            piece=getindex(board.squares[i][j],pieces[1-machineplays],6);
            Tvalue=Tvalue-Vpieces[piece];
                    
        }
    }
    
    return Tvalue;
}