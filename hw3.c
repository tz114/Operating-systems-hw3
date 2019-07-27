#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
typedef struct{
    int x;
    int y;
    int m;
    int n;
    int numMoves;
    char **squares;
}board;

int max_squares=1;
int numD=0;
board* dead_end_boards;
pthread_mutex_t lock;
int p=0;

int numValid(board b, bool moves[]){
	int res = 0;
    if(b.x-2>=0 && b.y-1>=0 && b.squares[b.y-1][b.x-2]!='S'){
		res++;
		moves[0] = true;
	}
	else
		moves[0] = false;
	if(b.x-1>=0 && b.y-2>=0 && b.squares[b.y-2][b.x-1]!='S'){
		res++;
		moves[1] = true;
	}
	else
		moves[1] = false;
    if(b.x+1<b.n && b.y-2>=0 && b.squares[b.y-2][b.x+1]!='S'){
		res++;
		moves[2] = true;
	}
	else
		moves[2] = false;
	if(b.x+2<b.n && b.y-1>=0 && b.squares[b.y-1][b.x+2]!='S'){
		res++;
		moves[3] = true;
	}
	else
		moves[3] = false;
	if(b.x+2<b.n && b.y+1<b.m && b.squares[b.y+1][b.x+2]!='S'){
		res++;
		moves[4] = true;
	}
	else
		moves[4] = false;
	if(b.x+1<b.n && b.y+2<b.m && b.squares[b.y+2][b.x+1]!='S'){
		res++;
		moves[5] = true;
	}
	else
		moves[5] = false;
	
	if(b.x-1>=0 && b.y+2<b.m && b.squares[b.y+2][b.x-1]!='S'){
		res++;
		moves[6] = true;
	}
	else
		moves[6] = false;
	if(b.x-2>=0 && b.y+1<b.m && b.squares[b.y+1][b.x-2]!='S'){
        
		res++;
		moves[7] = true;
	}
	else
		moves[7] = false;
	
	return res;
}

void *go(void *bo){
	
    bool moves[8];
    board b = *(board*)bo;
	
    int valid=numValid(b,moves);
    pthread_t tid[valid];
    int tidcount=0;
    int * deadmoves=calloc(1,sizeof(int));

    if(valid>=1){
        if(valid!=1){

            printf("THREAD %u: %d moves possible after move #%d; creating threads...\n", (unsigned int) pthread_self(), valid, b.numMoves);
            
        }  
        
        for(int i=0;i<8;i++){
            
            if(moves[i]){
                
                board next;

                next.x=b.x;
                next.y=b.y;
                next.m=b.m;
                next.n=b.n;
                next.numMoves=b.numMoves+1;
                next.squares=calloc(b.m,sizeof(char *));
                for (int t=0;t<b.m;t++){
                    next.squares[t]=calloc(b.n,sizeof(char));
                }
                for(int t=0;t<b.n;t++){
                    for(int j=0;j<b.m;j++){
                        next.squares[j][t]=b.squares[j][t];
                    }
                }
                if(i==0){
                    next.x-=2;
                    next.y-=1;
                }
                else if(i==1){
                    next.x-=1;
                    next.y-=2;
                }
                else if(i==2){
                    next.x+=1;
                    next.y-=2;
                }
                else if(i==3){
                    next.x+=2;
                    next.y-=1;
                }
                else if(i==4){
                    next.x+=2;
                    next.y+=1;
                }
                else if(i==5){
                    next.x+=1;
                    next.y+=2;
                }
                else if(i==6){
                    next.x-=1;
                    next.y+=2;
                }
                else if(i==7){
                    next.x-=2;
                    next.y+=1;
                }
                next.squares[next.y][next.x]='S';
                

					board *ne=malloc(sizeof(board));
					*ne=next;
					if(valid!=1){
					   
						pthread_create(&tid[tidcount],NULL,go,ne);
                        
                        #ifdef NO_PARALLEL
                            int * num;
                            pthread_join(tid[tidcount],(void **)&num);
                            *deadmoves=MAX(*deadmoves,*num);
                            
                            printf("THREAD %u: joined (returned %d)\n", (unsigned int)pthread_self(), *num);
                            free(num);
                        #endif
						tidcount++;
					
					   
					}
					else{
						int *ret=go(ne);
                        deadmoves=ret;
                        
                        
                       
					}
				
            }
        }
        if(valid!=1 && p==0){
            for(int t=0;t<valid;t++){
                int * num;
                pthread_join(tid[t],(void **)&num);
                *deadmoves=MAX(*deadmoves,max_squares);
                printf("THREAD %u: joined (returned %d)\n", (unsigned int)pthread_self(), *num);
                free(num);
            }
        }
    
    }
    else{
            if(b.numMoves==b.m*b.n){
                printf("THREAD %u: Sonny found a full knight's tour!\n", (unsigned int)pthread_self());
            }
            else{
                printf("THREAD %u: Dead end after move #%d\n", (unsigned int)pthread_self(), b.numMoves);
            }
            pthread_mutex_lock(&lock);
            
        
            if(b.numMoves!=b.m*b.n){
                numD++;
                dead_end_boards=realloc(dead_end_boards,numD*sizeof(board));
                dead_end_boards[numD-1]=b;
            }
            
            if(max_squares<=b.numMoves){
                max_squares=b.numMoves;
            }
            pthread_mutex_unlock(&lock);
            
            
            *deadmoves=b.numMoves;
            
            pthread_exit(deadmoves);

    }
    return deadmoves;
}
int main(int argc, char *argv[]){
	if(argc<3){
		fprintf(stderr, "ERROR: Invalid argument(s)\n");
		fprintf(stderr, "USAGE: a.out <m> <n> [<x>]\n");
		return EXIT_FAILURE;
	}
    int m = atoi(argv[1]);
    int n = atoi(argv[2]);
    pthread_mutex_init(&lock, NULL);
    if(m<=2 || n<=2){
        fprintf(stderr, "ERROR: Invalid argument(s)\n");
		fprintf(stderr, "USAGE: a.out <m> <n> [<x>]\n");
		return EXIT_FAILURE;
	}
    int x = 0;
    if(argc == 4){
        x=atoi(argv[3]);
		if(x<=0 || x>m*n){
			fprintf(stderr, "ERROR: Invalid argument(s)\n");
			fprintf(stderr, "USAGE: a.out <m> <n> [<x>]\n");
			return EXIT_FAILURE;
		}
	}
    board b;
    #ifdef NO_PARALLEL
        p=1;
    #endif
    b.x=0;
    b.y=0;
    b.m=m;
    b.n=n;
    b.numMoves=1;
    b.squares=calloc(m,sizeof(char *));
    for (int i=0;i<m;i++){
        b.squares[i]=calloc(n,sizeof(char));
    }
    for(int i=0;i<m;i++){
        for(int j=0;j<n;j++){
            b.squares[i][j]='.';
        }
    }
    b.squares[0][0]='S';
    int curr = pthread_self();
    board *bo = malloc(sizeof(board));
	*bo = b;
    printf("THREAD %u: Solving Sonny's knight's tour problem for a %dx%d board\n", curr,m,n);
    go(bo);
    printf("THREAD %u: Best solution(s) found visit %d squares (out of %d)\n", curr, max_squares, m * n);
	printf("THREAD %u: Dead end boards:\n", curr);
    free(bo);

    for(int i=0;i<numD;i++){
        if(dead_end_boards[i].numMoves>=x){
            printf("THREAD %u: > %s\n", (unsigned int)pthread_self(), dead_end_boards[i].squares[0]);
            for(int j=1;j<m;j++){
                printf("THREAD %u:   %s\n", (unsigned int)pthread_self(), dead_end_boards[i].squares[j]); 
            }
        }
    }
    for(int i=0;i<numD;i++){
        for(int j=0;j<m;j++){
            free(dead_end_boards[i].squares[j]);
        }
        free(dead_end_boards[i].squares);
    }
    for(int i=0;i<m;i++){
        free(b.squares[i]);
    }
    free(b.squares);
}
    
   