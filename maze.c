#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

void render(int ** grid, int size){
    for(int i = 0; i < size; i++){
        for(int j = 0; j < size; j++){
            printf("%02d ", grid[i][j]);
        }
        printf("\n");
    }
}

int main(){
    int ** map;
    int size = 8;

    map = (int **) malloc(sizeof(int*)*size);
    for(int i = 0; i < size; i++){
        map[i] = (int *) malloc(sizeof(int)*size);
        for(int j = 0; j < size; j++){
            map[i][j] = i + j * size;
        }
    }
    
    render(&map[2], size - 2);

    return 0;

}