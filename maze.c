#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define rows 33
#define cols 33

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

void recursion(int x, int y, int grid[rows][cols]){
    int dirs[4] = {0,1,2,3};
    shuffle(dirs, 4);
    
    for(int i =y-1; i < y+2;i++){
        for(int j = x-1; j < x+2; j++){
            grid[j][i] = 0;
        }
    }

    system("clear");
    for(int i = 0; i < rows; i++){
        for(int j = 0; j < cols; j++){
            if(i == x && y == j){
                printf(".");
            }else{
            if(grid[i][j] == 1){
                printf("=");
            }else{
                printf(" ");
            }
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
            if(y - 4 <= 0)
                continue;
            if(grid[x][y-4] != 0){
                for(int i = x-1; i < x+2; i++){
                    grid[i][y-2] =0;
                }
                recursion(x, y-4, grid);
            }
            break;

        case 1: // Right
            if(x + 4 >= cols - 1)
                continue;
            if(grid[x +4][y] != 0){
                for(int i = y-1; i < y+2; i++){
                    grid[x+2][i] =0;
                }
                recursion(x + 4, y, grid);
            }
            break;

        case 2: // down
            if(y + 4 >= rows - 1)
                continue;
            if(grid[x][y+4] != 0){
                for(int i = x-1; i < x+2; i++){
                    grid[i][y+2] =0;
                }
                recursion(x, y+4, grid);
            }
            break;

        
        case 3: // Left
            if(x - 4 <= 0)
                continue;
            if(grid[x -4][y] != 0){
                for(int i = y-1; i < y+2; i++){
                    grid[x-2][i] =0;
                }
                recursion(x - 4, y, grid);
            }
            break;
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

    int x = rows/3, y = cols/3;
    srand(time(NULL));

    recursion(x-1,y-1,grid);

    return 0;

}