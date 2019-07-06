#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>

#define rows 33
#define cols 33

int dirs[4] = {1,0,-1,0};

void render(int grid[rows][cols], int t){
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
    for(int i = y-1; i < y+2; i++){
        for(int j = x-1; j < x+2; j++){
            grid[j][i] = 0; // mark cells as visited
        }
    }

    int k;
    do{
        int neighbours[3];
        k = 0;

        for(int j = 0; j < 4; j++){
            if(grid[x + dirs[j]*4][y + dirs[(j+1)%4]*4])
                neighbours[k++] = x + dirs[j]*4 + rows * (y + dirs[(j+1)%4]*4);
        }
    
        if(k){
            int r = rand() % k;
            for(int i = -1; i < 2; i++){
                for(int j = -1; j < 2; j++){
                    grid[(neighbours[r] % rows + x)/2 + j][( neighbours[r] / rows + y)/2 + i] =0;
                }
            }
            recursion(neighbours[r] % rows,  neighbours[r] / rows, grid);
        }
    }while(k != 0);
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

    clock_t start, stop;
    double perf = 0.0;
    int times = 100000;
    srand(time(NULL));

    for(int t = 0; t < times; t++){
        for(int i = 0; i < rows; i++){
            for(int j = 0; j < cols; j++){
                if(i < 2 || i > rows-3 || j < 2 || j > rows-3){
                    grid[i][j] = 0;
                }else{
                    grid[i][j] = 1;
                }
            }
        }

        start = clock();
        recursion(4,4,grid);
        stop = clock();
        perf += (double) (stop-start);
    }
    render(grid, 50);
    printf("%lf\n", perf/times);
    return 0;

}