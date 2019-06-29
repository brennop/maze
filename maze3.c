#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>

#define rows 33
#define cols 33
#define size 4

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
                printf(" . ");
            }else{
            if(grid[i][j] == __INT_MAX__){
                printf(" # ");
            }else if(grid[i][j] == 0){
                printf("   ");
            }else{
                printf(" %02d", grid[i][j]);
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
    

    for(int i = y-(size/2)+1; i < y+(size/2); i++){
        for(int j = x-(size/2)+1; j < x+(size/2); j++){
            grid[j][i] = 0; // mark cells as visited
        }
    }

    render(grid, 50, x, y);

    for(int i = 0; i < 4; i++){
        int neighbours[4];
        int k = 0;

        if(y - size > 0 && grid[x][y - size] != 0){ //up
            neighbours[k] = x + rows * (y - size);
            k++;
        }
        if(x - size > 0 && grid[x - size][y] != 0){ // left
            neighbours[k] = x - size + rows * y;
            k++;
        }
        if(y + size < rows -1 && grid[x][y + size] != 0){ // down
            neighbours[k] = x + rows * (y + size);
            k++;
        }
        if(x + size < cols -1 && grid[x + size][y] != 0){ // right
            neighbours[k] = x + size + rows * y;
            k++;
        }
    
        if(k > 0){
            int r = rand() % k;
            int next_x = neighbours[r] % rows, next_y = neighbours[r] / rows;

            for(int i = -size/2 + 1; i < size/2; i++){
                for(int j = -size/2 + 1; j < size/2; j++){
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

        render2(distMap,20,x,y);

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

    

    recursion(size,size,grid);

    for(int i = 0; i < rows; i++){
        for(int j = 0; j < cols; j++){
            if(grid[i][j] == 1){
                dist[i][j] = __INT_MAX__;
            }else{
                dist[i][j] = 0;
            }
        }
    }
    
    path(rows/2,cols/2, dist,1);
    //render2(dist,10, 4,4);

    

    return 0;

}