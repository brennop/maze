#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define rows 15
#define cols 15

void render(int grid[rows][cols]){
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
}

void recursion(int x, int y, int grid[rows][cols]){
    render(grid);
    grid[x][y] = 0; // mark current cell as visited

    for(int i = 0; i < 4; i++){
        int neighbours[4];
        int k = 0;

        if(y - 2 > 0 && grid[x][y - 2] != 0){ //up
            neighbours[k] = x + rows * (y - 2);
            k++;
        }
        if(x - 2 > 0 && grid[x - 2][y] != 0){ // left
            neighbours[k] = x - 2 + rows * y;
            k++;
        }
        if(y + 2 < rows -1 && grid[x][y + 2] != 0){ // down
            neighbours[k] = x + rows * (y + 2);
            k++;
        }
        if(x + 2 < cols -1 && grid[x + 2][y] != 0){ // right
            neighbours[k] = x + 2 + rows * y;
            k++;
        }
    
        if(k > 0){
            int r = rand() % k;
            int next_x = neighbours[r] % rows, next_y = neighbours[r] / rows;

            grid[(next_x + x)/2][(y + next_y) / 2] = 0; // remove wall;

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

    int x = rows/2, y = cols/2;
    srand(time(NULL));

    recursion(x,y,grid);

    return 0;

}