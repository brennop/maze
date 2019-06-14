#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define rows 15
#define cols 15

void shuffle(int *array, size_t n)
{
    if (n > 1) 
    {
        size_t i;
        for (i = 0; i < n - 1; i++) 
        {
          size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
          int t = array[j];
          array[j] = array[i];
          array[i] = t;
        }
    }
}

void recursion(int x, int y, int grid[rows][cols],int * n){
    if(*n <= rows/2 * cols/2){
        int dirs[4] = {0,1,2,3};
        shuffle(dirs, 4);
        grid[x][y] = 0;

        * n+=1;
        
        system("clear");
        for(int i = 0; i < rows; i++){
            for(int j = 0; j < cols; j++){
                if(grid[i][j] == 1){
                    printf("#");
                }else{
                    printf(" ");
                }
            }
            printf("\n");
        }   

        struct timespec ts;
        ts.tv_sec = 200/1000;
        ts.tv_nsec = (200%1000) * 1000000;
        nanosleep(&ts, NULL);


        for(int i = 0; i < 4; i++){
            switch (dirs[i])
            {
            case 0: // up
                if(y - 2 <= 0)
                    continue;
                if(grid[x][y-2] != 0){
                    grid[x][y-2] = 0;
                    grid[x][y-1] =0;
                    recursion(x, y-2, grid, n);
                }
                break;

            case 1: // Right
                if(x + 2 >= cols - 1)
                    continue;
                if(grid[x +2][y] != 0){
                    grid[x + 2][y] = 0;
                    grid[x + 1][y] = 0;
                    recursion(x + 2, y, grid, n);
                }
                break;

            case 2: // down
                if(y + 2 >= rows - 1)
                    continue;
                if(grid[x][y+2] != 0){
                    grid[x][y+2] = 0;
                    grid[x][y+1] =0;
                    recursion(x, y+2, grid, n);
                }
                break;

            
            case 3: // Left
                if(x - 2 <= 0)
                    continue;
                if(grid[x -2][y] != 0){
                    grid[x - 2][y] = 0;
                    grid[x - 1][y] = 0;
                    recursion(x - 2, y, grid, n);
                }
                break;
            }
        }
    }
}

void expand(int grid[rows][cols]){
    for(int i = 0; i < rows; i++){
        for(int j = 0; j < cols; j++){
            if(grid[i][j] == 1){
                
            }
        }
    }
}

int main(){
    int grid[rows][cols];
    int n =0;

    for(int i = 0; i < rows; i++){
        for(int j = 0; j < cols; j++){
            grid[i][j] = 1;
        }
    }

    int x = 1, y = 1;
    srand(time(NULL));

    recursion(x,y,grid, &n);

    return 0;

}