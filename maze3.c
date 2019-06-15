#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define rows 17
#define cols 17

void render(int grid[rows][cols], int t,int x, int y){
    system("clear");
    for(int i = 0; i < rows; i++){
        for(int j = 0; j < cols; j++){
            if(i == x && j == y){
                printf("x");
            }else{
                if(grid[i][j] == 1){
                    printf("#");
                }else{
                    printf(" ");
                }
            }
        }
        printf("\n");
    }   

    struct timespec ts;
    ts.tv_sec = t/1000;
    ts.tv_nsec = (t%1000) * 1000000;
    nanosleep(&ts, NULL);
}

void recursion(int x, int y, int grid[rows][cols]){
    

    for(int i = y-1; i < y+2; i++){
        for(int j = x-1; j < x+2; j++){
            grid[j][i] = 0; // mark cells as visited
        }
    }
    render(grid,x,y, 250);

    for(int i = 0; i < 4; i++){
        int neighbours[4];
        int k = 0;

        if(y - 4 > 0 && grid[x][y - 4] != 0){ //up
            neighbours[k] = x + rows * (y - 4);
            k++;
        }
        if(x - 4 > 0 && grid[x - 4][y] != 0){ // left
            neighbours[k] = x - 4 + rows * y;
            k++;
        }
        if(y + 4 < rows -1 && grid[x][y + 4] != 0){ // down
            neighbours[k] = x + rows * (y + 4);
            k++;
        }
        if(x + 4 < cols -1 && grid[x + 4][y] != 0){ // right
            neighbours[k] = x + 4 + rows * y;
            k++;
        }
    
        if(k > 0){
            int r = rand() % k;
            int next_x = neighbours[r] % rows, next_y = neighbours[r] / rows;

            for(int i = -1; i < 2; i++){
                for(int j = -1; j < 2; j++){
                    grid[(next_x + x)/2 + j][(y + next_y)/2 + i] =0;
                }
            }
            //grid[(next_x + x)/2][(y + next_y) / 2] = 0; // remove wall;

            recursion(next_x, next_y, grid);
        }
    }
}


int main(){
    int grid[rows][cols];

    for(int i = 0; i < rows; i++){
        for(int j = 0; j < cols; j++){
            grid[i][j] = 1;
        }
    }

    int x = rows/3, y = cols/3;
    srand(time(NULL));

    recursion(2,2,grid);

    return 0;

}