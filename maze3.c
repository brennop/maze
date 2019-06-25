#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define rows 21
#define cols 21

void render(int grid[rows][cols], int t,int x, int y){
    system("clear");
    for(int i = 0; i < rows; i++){
        for(int j = 0; j < cols; j++){
            if(grid[i][j] == 1){
                printf("#");
            }else if(grid[i][j] == 2){
                printf(".");
            }else if(grid[i][j] == 3){
                printf("x");
            }
            else{
                printf(" ");
            }
        }
        printf("\n");
    }   

    struct timespec ts;
    ts.tv_sec = t/1000;
    ts.tv_nsec = (t%1000) * 1000000;
    nanosleep(&ts, NULL);
}

void render2(int grid[rows][cols], int t, int x, int y){
    system("clear");
    for(int i = 0; i < rows; i++){
        for(int j = 0; j < cols; j++){
            if(x == j && y == i){
                printf(". ");
            }else{
            if(grid[i][j] == __INT_MAX__){
                printf("# ");
            }else{
                printf("%02d", grid[i][j]);
            }}
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

void path(int x, int y, int distMap[rows][cols], int dis){
    int queue[rows*cols];
    queue[0] = x + cols*y;
    int q = 1;
    int * current = queue;
    distMap[*current/rows][*current%cols] = 1;

    while(q > current - queue){
        x = * current/rows;
        y = * current%cols;
        dis = distMap[y][x] + 1;

        if(distMap[y - 1][x] == 0){ //up
            queue[q++] = x * rows + (y - 1);
            distMap[y-1][x] = dis;
        }
        if(distMap[y][x - 1] == 0){ // left
            queue[q++] = (x - 1) * rows + y;
            distMap[y][x-1] = dis;
        }
        if(distMap[y + 1][x] == 0){ // down
            queue[q++] = x * rows + (y + 1);
            distMap[y+1][x] = dis;
        }
        if(distMap[y][x + 1] == 0){ // right
            queue[q++] = (x + 1) * rows + y;
            distMap[y][x+1] = dis;
        }

        current++;
    }
}


int main(){
    int grid[rows][cols];
    int dist[rows][cols];

    for(int i = 0; i < rows; i++){
        for(int j = 0; j < cols; j++){
            if(i < 2 || i > rows-3 || j < 2 || j > rows-3){
                grid[i][j] = 0;
            }else{
                grid[i][j] = 1;
            }
        }
    }

    int x = rows/3, y = cols/3;
    srand(time(NULL));

    

    recursion(4,4,grid);

    for(int i = 0; i < rows; i++){
        for(int j = 0; j < cols; j++){
            if(grid[i][j] == 1){
                dist[i][j] = __INT_MAX__;
            }else{
                dist[i][j] = 0;
            }
        }
    }
    
    path(4,4, dist,1);
    //render2(dist,10, 4,4);

    

    return 0;

}