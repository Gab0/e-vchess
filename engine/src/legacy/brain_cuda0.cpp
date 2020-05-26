
    struct board *GPUboard;
    struct movelist *GPUmoves;
    
    long *_Alpha;
    long *_Beta;

    
    cudaMalloc((void**) &_Alpha, sizeof(long));
    cudaMalloc((void**) &_Beta, sizeof(long));
    
    cudaMemcpy(_Alpha, &Alpha, sizeof(long), cudaMemcpyHostToDevice);
    cudaMemcpy(_Beta, &Beta, sizeof(long), cudaMemcpyHostToDevice);
    
    cudaMalloc((void**) &GPUmoves, sizeof(struct movelist));
    cudaMemcpy(GPUmoves, moves, sizeof(struct movelist), cudaMemcpyHostToDevice);
    
    
    
    //array of boards GPU allocation.
    /*cudaMalloc((void**) &GPUboard, sizeof(struct board*) * moves->k);
    for (i=0;i<moves->k;i++) {
        cudaMalloc((void**) &GPUboard[i], sizeof(struct board));
        cudaMemcpy(GPUboard[i], _board, sizeof(struct board), cudaMemcpyHostToDevice); 
        printf("I=%p\n", GPUboard[i]);  
	}*/

    cudaMalloc((void**) &GPUboard, sizeof(struct board));
    cudaMemcpy(GPUboard, _board, sizeof(struct board), cudaMemcpyHostToDevice);

    
    int *CTest = (int*)calloc(moves->k, sizeof(int));
    int *Test;
    cudaMalloc((void**) &Test, moves->k * sizeof(int));
    
    cudaMemcpy(Test, CTest, sizeof(int) * moves->k, cudaMemcpyHostToDevice);
    dim3 threadsPerBlock(1, 1); 
    dim3 numBlocks(moves->k, 1);  
    kerneliterate <<<numBlocks, threadsPerBlock>>>(GPUboard, GPUmoves, PL, DEEP, Test, _Beta);


    

    
    //Testkernel <<<moves->k, 1>>> (Test);
    //cudaDeviceSynchronize();
    //cudaFree(GPUboard);
    
    //kernel error debug;
    cudaError_t err = cudaGetLastError();
    if (err != cudaSuccess) 
    printf("Error: %s\n", cudaGetErrorString(err));

    //cudaMemcpy(CTest, Test, moves->k * sizeof(int), cudaMemcpyDeviceToHost);
    
    
    int R[2];

    printf("GPUMOVES = %p\n", GPUmoves);
    cudaMemcpy(moves, GPUmoves, sizeof(struct movelist), cudaMemcpyDeviceToHost);
    
    
    
    
    select_top(moves->movements, moves->k, R, 1);
    r = R[0];
    
    
    for (i=0;i<moves->k;i++){
      printf("score for i=%i is %i\n", i , moves->movements[i].score);
      printf("TEST %i = %i\n", i, CTest[i]);
    }
    printf("but the GPU chose move n=%i\n", r);
    
    cudaFree(GPUmoves);
    cudaFree(GPUboard);
    
