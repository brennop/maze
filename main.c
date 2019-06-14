#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>

#ifdef _WIN32
#define CLEAR "cls"
#else
#define CLEAR "clear"
#endif

/*Função de captura de entrada do teclado */
int getch() {
    int c=0;
    struct termios org_opts, new_opts;
    int res=0;
    //—– guarda configurações antigas ———//
    res=tcgetattr(STDIN_FILENO, &org_opts);
    assert(res==0);
    //—- configura novos parametros de entrada ——//
    memcpy(&new_opts, &org_opts, sizeof(new_opts));
    new_opts.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ECHOPRT |ECHOKE | ICRNL);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_opts);
    c=getchar();
    //—— restore old settings ———
    res=tcsetattr(STDIN_FILENO, TCSANOW, &org_opts);
    assert(res==0);
    return(c);
}

struct Player{
    int race, aling, class, size, strength, constitution, dexterity, intelligence, x, y;
};


int input();
int game();
int ** genMap();
void enter();
void walk();
void render();
struct Player create();

int main(){
    srand(time(NULL));

    //struct Player player = create();
    struct Player player; // empty player for debugging

    game(player, 33);
}

int game(struct Player player, int size){
    int **map, opt;

    map = genMap(size);
    player.x = size/2;
    player.y = size/2;

    map[player.y][player.x] = -1;

    while(1 == 1){

        render(player.x, player.y, map, size, 5);
        
        switch (getch()){
            case -1: //EOF
                exit(0);
                break;

            case 'A': // UP
                if(map[player.y-1][player.x] == 0){
                    map[player.y-1][player.x] = -1;
                    map[player.y][player.x] = 0;
                    player.y--;
                }
                break;

            case 'B': // Down
                if(map[player.y+1][player.x] == 0){
                    map[player.y+1][player.x] = -1;
                    map[player.y][player.x] = 0;
                    player.y++;
                }
                break;
            case 'C': // rigth
                if(map[player.y][player.x+1] == 0){
                    map[player.y][player.x+1] = -1;
                    map[player.y][player.x] = 0;
                    player.x++;
                }
                break;
            case 'D': //left
                if(map[player.y][player.x-1] == 0){
                    map[player.y][player.x-1] = -1;
                    map[player.y][player.x] = 0;
                    player.x--;
                }
                break;
            default:
                break;
        }
    }
}

// Função para esperar um input qualquer do usuário
void enter(){
    scanf("%*c%*[^\n]s"); 
}

// Função para pegar um input de opções
int input(char txt[], int opts){
    int chosen = 0;
    while(chosen > opts || chosen <= 0){ // enquanto o escolha não estiver no intervalo válido
        printf("%s ", txt);
        scanf("%d", &chosen);
        printf("\n");
    }
    return chosen;
}


struct Player create(){
    struct Player p;
    int sum = 11;

    printf("Raça\n");
    p.race = input("( 1 - Humano | 2 - Anão | 3 - Elfo )", 3);

    printf("Alinhamento\n");
    p.aling = input("( 1 - Mal | 2 - Neutro | 3 - Bom )", 3);

    printf("Profissão\n");
    if(p.aling == 3){
        p.class = input("( 1 - Gurreiro | 2 - Mago )", 3);
    }else{
        p.class = input("( 1 - Gurreiro | 2 - Mago | 3 - Ladino )", 3);
    }

    printf("História Prévia\n");
    enter();

    printf("\nPorte\n");
    if(p.race == 2){
        p.size = input("( 1 - Médio | 2 - Pequeno )", 2) + 1;
    }else{
        p.size = input("( 1 - Grande | 2 - Médio | 3 - Pequeno )", 3);
    }

    while (sum > 10){
        sum = 0;

        printf("Força ");
        p.strength = input("( 1 - 5 )", 5);
        sum += p.strength;

        printf("Constituição ");
        p.constitution = input("( 1 - 5 )", 5);
        sum += p.constitution;

        printf("Destreza ");
        p.dexterity = input("( 1 - 5 )", 5);
        sum += p.dexterity;

        printf("Inteligência ");
        p.intelligence = input("( 1 - 5 )", 5);
        sum += p.intelligence;

        // TODO: Facilitar distribuição dos pontos
    }

    if(p.race == 2){
        p.strength += 1;
        p.dexterity -= 1;
    }else if(p.race == 3){
        p.dexterity += 1;
        p.constitution -= 1;
    }

    p.x = 0;
    p.y = 0;

    return p;
    
}

void render(int x, int y, int ** map, int size, int fov){
    system(CLEAR);

    // 
    for(int i = y-fov/2; i <= y+fov/2; i++){
        for(int j = x-fov/2; j < x+fov/2; j++){
            if(i >= 0 && i < size && j >= 0 && j < size){
                switch (map[i][j])
                    {
                    case 1:
                        printf("#");
                        break;
                    case -1:
                        printf("@");
                        break;
                    default:
                        printf(" ");
                        break;
                    }
            }
            else{
                printf(" ");
            }
        }
        printf("\n");
        
    }
    printf("%d %d\n", x-2, y-2);
}


/* 

    Geração do Labirinto 
    Implementação de Recursive Backtracker

*/

int ** genMap(int size){
    int ** grid;

    // Aloca e cria uma matriz de paredes ("1")
    grid = (int **) malloc(size * sizeof(int*));
    for(int i = 0; i < size; i++){
        grid[i] = (int *) malloc(size * sizeof(int));
        for(int j = 0; j < size; j++){
            grid[i][j] = 1;
        }
    }

    // começa com a célula do meio do labirinto
    walk(2, 2, grid, size);

    return grid;

}

void walk(int x, int y, int ** grid, int size){
    // Marca a região da célula atual como visitada
    for(int i = y-1; i < y+2; i++){
        for(int j = x-1; j < x+2; j++){
            grid[j][i] = 0; 
        }
    }
    
    for(int i = 0; i < 4; i++){ // para cada direção ( não vai na direção que veio )
        int neighbours[4]; // Vetor de vizinhos possíveis
        int k = 0; // Tamanho do vetor de vizinhos

        // Testa em cada direção se é um vizinho possível
        // se for um candidato, guarda a posição "contígua" na matriz

        if(y - 4 > 0 && grid[x][y - 4] != 0){ //up
            neighbours[k] = x + size * (y - 4);
            k++;
        }
        if(x - 4 > 0 && grid[x - 4][y] != 0){ // left
            neighbours[k] = x - 4 + size * y;
            k++;
        }
        if(y + 4 < size -1 && grid[x][y + 4] != 0){ // down
            neighbours[k] = x + size * (y + 4);
            k++;
        }
        if(x + 4 < size -1 && grid[x + 4][y] != 0){ // right
            neighbours[k] = x + 4 + size * y;
            k++;
        }

        if(k > 0){ // se há vizinhos disponíveis
            // escolhe um vizinho aleatóriamente
            int r = rand() % k;

            // traduz as posições contíguas em coordenadas
            int next_x = neighbours[r] % size, next_y = neighbours[r] / size; 

            // remove a parede
            //grid[(next_x + x)/2][(y + next_y) / 2] = 0; 

            // remove as paredes
            for(int i = -1; i < 2; i++){
                for(int j = -1; j < 2; j++){
                    grid[(next_x + x)/2 + j][(y + next_y)/2 + i] =0;
                }
            }

            // anda para a próxima célula
            walk(next_x, next_y, grid, size); 
        }
    }
}
