#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>
#include <stdbool.h>

#ifdef _WIN32
#define CLEAR "cls"
#else
#define CLEAR "clear"
#endif

// terminal coloridinho
#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define YEL "\x1B[33m"
#define BLU "\x1B[34m"
#define MAG "\x1B[35m"
#define CYN "\x1B[36m"
#define WHT "\x1B[37m"
#define RST "\x1B[0m"

// outros efeitos top
#define BLD "\x1B[1m"
#define ITL "\x1B[3m"
#define UND "\x1B[4m"

bool DEBUG = false;

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
    int race, aling, class, size, strength, constitution, hp, dexterity, intelligence, x, y;
};

struct Trap{
    int x, y;
};

// Buffer de mensagens no estilo Rogue/Nethack
char message[50];

int input();
int game();
int ** genMap();
void spawnTraps();
void checkTraps(struct Player * player, int ** grid, bool isAuto);
void enter();
void move();
void walk();
void render();
struct Player create();

int main(int argc, char *argv[]){
    srand(time(NULL));

    if(argc >= 2){
        if(strcmp(argv[1], "-d") == 0){
            DEBUG = true;
        }
    }

    struct Player player = create();
    //struct Player player; // empty player for debugging
    player.hp = 20;
    player.constitution = 20;

    game(player, 33);
}

int game(struct Player player, int size){
    int **map, opt;

    map = genMap(size);
    spawnTraps(map, size);
    player.x = size/2;
    player.y = size/2;

    map[player.y][player.x] = -1;

    while(1 == 1){

        render(player, map, size, 2);
        
        switch (getch()){
            case -1: //EOF
                exit(0);
                break;

            case 'A': // UP
                move(&player, map, 0, -1);
                break;
            case 'B': // Down
                move(&player, map, 0, 1);
                break;
            case 'C': // rigth
                move(&player, map, 1, 0);
                break;
            case 'D': //left
                move(&player, map, -1, 0);
                break;
            case 'c':
                strcpy(message, "ITL procurando armadilhas..." RST);
                checkTraps(&player, map, false);
                break;
            default:
                // limpa o buffer a cada ação
                strcpy(message, "\0");
                break;
        }

    }
}

// Função para esperar um input qualquer do usuário
void enter(){
    scanf("%*c%*[^\n]s"); 
}


void move(struct Player * player, int ** grid, int dir_x, int dir_y){
    // verificação automatica de armadilhas
    checkTraps(player, grid, true);

    // Verifica se o movimento é válido
    if(grid[player->y + dir_y][player->x + dir_x] != 1){

        // Verifica se é armadilha:
        if(grid[player->y + dir_y][player->x + dir_x] == -2 || grid[player->y + dir_y][player->x + dir_x] == 2 || grid[player->y + dir_y][player->x + dir_x] == 3){
            // reduz a vida do player
            player->hp -= 1;
            strcpy(message, ITL "você pisou em uma armadilha" RST);
            // armadilha é destruída após
        }

        grid[player->y + dir_y][player->x + dir_x] = -1;
        grid[player->y][player->x] = 0;

        player->x += dir_x;
        player->y += dir_y;
    }
}


// TODO: REFACTOR THIS!!
void checkTraps(struct Player * player, int ** grid, bool isAuto){
    
    for(int i = player->y - 2; i <= player->y + 2; i++){
        for(int j = player->x - 2; j <= player-> x +2; j++){
            if(grid[i][j] == 2 || grid[i][j] == 3 - isAuto){
                int r = rand() % 100;
                int classModifier;

                if(player->class == 3){
                    classModifier = 12;
                }else{
                    classModifier = 20;
                }

                if( (player->dexterity + player->intelligence) * 100 / classModifier >= r){
                    grid[i][j] = -2;
                }else{
                    grid[i][j] += isAuto;
                }
            }
        }
    }
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
        p.class = input("( 1 - Gurreiro | 2 - Mago )", 2);
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

    p.hp = p.constitution;

    p.x = 0;
    p.y = 0;

    return p;
    
}

void render(struct Player player, int ** map, int size, int fov){
    system(CLEAR);
    printf("\n");
    // 
    for(int i = player.y - fov; i <= player.y + fov; i++){
        printf("    ");
        for(int j = player.x - fov; j <= player.x + fov; j++){
            
            if(i >= 0 && i < size && j >= 0 && j < size){
                switch (map[i][j])
                    {
                    case 1:
                        printf("= ");
                        break;
                    case -1:
                        printf(YEL "@ " RST);
                        break;
                    case -2:
                        printf(RED "x " RST);
                        break;
                    default:
                        printf(". ");
                        break;
                    }
            }
            else{
                printf("  ");
            }
        }
        printf("\n");
        
    }
    printf("\n");
    printf("HP: %d/%d\n", player.hp, player.constitution);
    printf("%s\n", message);

    if(DEBUG){
        for(int i = 0; i < size; i++){
            for(int j = 0; j < size; j++){
                switch (map[i][j])
                    {
                    case 1:
                        printf("= ");
                        break;
                    case -1:
                        printf(YEL "@ " RST);
                        break;
                    case -2:
                        printf(RED "x " RST);
                        break;
                    case 2:
                        printf(YEL "x " RST);
                        break;
                    case 3:
                        printf(BLU "x " RST);
                        break;
                    default:
                        printf(". ");
                        break;
                    }
            }
            printf("\n");
        }
        printf("%d %d\n", player.x, player.y);
    }


}

void spawnTraps(int ** grid, int size){
    int n = size/4; // Quantidade de traps
    int x, y; // posição das traps

    while(n > 0){
        x = rand() % size;
        y = rand() % size;

        if(grid[y][x] == 0){
            grid[y][x] = 2; // Coloca armadilha;
            n--;
        }
    }
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
