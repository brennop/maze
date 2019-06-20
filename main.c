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

typedef struct{
    int race, aling, class, size, strength, constitution, dexterity, intelligence, x, y;
    int hp, maxhp;
    int inventory[9], items;
    int stronger;
}Player;

typedef struct{
    int class;
    int x, y;
}Enemy;

// Use it later, if has rendering/raytracing
struct Trap{
    int x, y;
};

char message[100]; // Buffer de mensagens no estilo Rogue/Nethack
bool isPlaying = false; // Variável para indicar se o jogo está rodando ou pausado

int input(); // talvez vai morrer
void enter(); // tbm

int ** genMap(int size);
void walk(int x, int y, int ** grid, int size);

int game(Player player, char i[7], int size);

void spawn(int ** grid, int size, int n, int type);
void checkTraps(Player * player, int ** grid, bool isAuto);

void move(Player * player, int ** grid, int dir_x, int dir_y);

void render(Player player, int ** map, int size, int fov, int ttl);
void inventory(Player * player);

Player create();
Enemy spawnEnemy(int ** grid, int size, Player player);
char * config();

int main(int argc, char *argv[]){
    srand(time(NULL));

    if(argc >= 2){
        if(strcmp(argv[1], "-d") == 0){
            DEBUG = true;
        }
    }

    Player player = create();
    char i[7] = {'A', 'B', 'C', 'D', 'c', 'i', '\0'};
    //Player player; // empty player for debugging
    //player.hp = 20;
    //player.constitution = 20;

    game(player, i, 33);
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

int min(int a, int b){
    return((a+b - abs(a-b))/2);
}

int max(int a, int b){
    return((a+b + abs(a-b))/2);
}

Player create(){
    Player p;
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

    // HP é 3 * Constituição
    // Se constiuição for zero, HP = 1
    p.hp = p.constitution * 3 + !p.constitution;
    p.maxhp = p.hp;

    // Deixa o inventário vazio
    for(int i = 0; i < 10; i++){
        p.inventory[i] = 0;
    }
    p.items = 0;
    p.stronger = 0;

    return p;
    
}

int game(Player player, char i[7], int size){
    int **map;
    char opt;

    int timeToNextEnemy = 10, entityCount = 0;
    Enemy *enemies;
    enemies = (Enemy*) malloc(10*sizeof(Enemy));

    map = genMap(size);

    spawn(map, size, size/4, 2);  // Spawn de size/4 armadilhas (representação 2)
    spawn(map, size, size/10, 4); // Spawn de size/4 poções de cura (representação 4)
    spawn(map, size, size/10, 5); // Spawn de size/4 poções de enhance (representação 5)

    // Spawn do personagem em algum canto do labirinto
    int r = rand() % 4;
    player.x = (r % 2) * (size-5) - (~r % 2) * 4;
    player.y = (r < 2) * (size-5) + (r > 1) * 4;

    map[player.y][player.x] = -1;

    // Começa o jogo
    isPlaying = true;
    while(1 == 1){
        render(player, map, size, 2, timeToNextEnemy); // should I render after inputs??

        opt = getch();

        // Se o input for um comando válido
        // necessário para passar o round corretamente
        if(strchr(i, opt)){
            // Passa o round...

            // limpa o buffer a cada ação
            strcpy(message, "\0");
            // reduz o efeito de potencialização a cada turno
            if(player.stronger)
                player.stronger--;
            // Spawn dos monstros
            if(!(--timeToNextEnemy) && entityCount < 10){
                enemies[++entityCount] = spawnEnemy(map, size, player);
                timeToNextEnemy = 60 + rand() % 20;
            }
            // move os monstros

            // Switch/Case evitado pelos valores não serem integrais/constantes
            if(opt == i[0]){ // Up
                move(&player, map, 0, -1);
            }else if(opt == i[1]){ // Down
                move(&player, map, 0, 1);
            }else if(opt == i[2]){ // Rigth
                move(&player, map, 1, 0);
            }else if(opt == i[3]){ // Left
                move(&player, map, -1, 0);
            }else if(opt == i[4]){
                strcpy(message, ITL "procurando armadilhas..." RST);
                checkTraps(&player, map, false);
            }else if(opt == i[5]){
                inventory(&player);
            }
        }
    }
}

void move(Player * player, int ** grid, int dir_x, int dir_y){
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
        // Verifica se é poção / item
        else if(grid[player->y + dir_y][player->x + dir_x] == 4 || grid[player->y + dir_y][player->x + dir_x] == 5){
            if(player->items < 9){
            // adiciona poção ao inventário
                player->inventory[player->items] = grid[player->y + dir_y][player->x + dir_x];
                if(player->inventory[player->items] == 4)
                    strcpy(message, ITL GRN "poção de regeneração" RST ITL " adicionada ao inventário" RST);
                else{
                    strcpy(message, ITL BLU "poção de potencialização" RST ITL " adicionada ao inventário" RST);
                }
                player->items++;
            }
        }

        grid[player->y + dir_y][player->x + dir_x] = -1;
        grid[player->y][player->x] = 0;

        player->x += dir_x;
        player->y += dir_y;
    }
}


// Verifica se há traps na região (automaticamente ou não)
void checkTraps(Player * player, int ** grid, bool isAuto){
    
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


void inventory(Player * player){
    bool exit = false; // TODO: change this name
    int opt;

    system(CLEAR);
    while(!exit){
        system(CLEAR);
        printf("\n");

        for(int i = 0; i < player->items; i++){
            printf("    ( %d)", i+1);
            if(player->inventory[i] == 4){
                printf("poção de regeneração\n");
            }else{
                printf("poção de potencialização\n");
            }
        }

        printf("    ( i) voltar ao jogo");

        opt = getch();
        printf("%d\n", opt);

        if(opt > 48 && opt <= player->items + 48){
            // Se for poção de regeneração
            if(player->inventory[opt-49] == 4){
                strcpy(message, "usou " GRN "poção de regeneração" RST);

                // regenera a vida do player
                player->hp += player->constitution;
                // limita a vida
                if(player->hp > player->maxhp)
                    player->hp = player->maxhp;
            }else{
                strcpy(message, "usou " BLU "poção de potencialização" RST);
                
                // potencializa o player por x rounds
                player->stronger = 50;
            }

            // remove o item do inventário
            for(int i = opt - 49; i < player->items; i++){
                player->inventory[opt - 49] = player->inventory[opt-48];
            }
            player->inventory[--player->items] = 0;
            
            exit = true;
        }else if(opt == 'i'){
            exit = true;
        }
    }

}

// Coloca n types em grid aleatóriamente
void spawn(int ** grid, int size, int n, int type){
    int x, y; // posição das traps

    while(n > 0){ // caso nao tenha lugar disponivel, entra em loop infinito
        x = rand() % size;
        y = rand() % size;

        if(grid[y][x] == 0){
            grid[y][x] = type; // Coloca armadilha;
            n--;
        }

        
    }
}

Enemy spawnEnemy(int ** grid, int size, Player player){
    int x, y;
    Enemy enemy = {0, 0, 0};

    while(enemy.class == 0){
        x = max(player.x - 5, 0) + min(rand() % 10, size - player.x);
        y = max(player.y - 5, 0) + min(rand() % 10, size - player.y);

        if(grid[y][x] == 0){
            grid[y][x] = -3;
            enemy.class = 1 + rand() % 3;
            enemy.x = x;
            enemy.y = y;
        }
    }

    return enemy;
}


void render(Player player, int ** map, int size, int fov, int ttl){ // delete ttl
    system(CLEAR);
    printf("\n");
    // 
    for(int i = player.y - fov; i <= player.y + fov; i++){
        printf("    ");
        for(int j = player.x - fov; j <= player.x + fov; j++){
            
            if(i >= 0 && i < size && j >= 0 && j < size){
                switch (map[i][j])
                    {
                    case 0:
                        printf(". ");
                        break;
                    case 1:
                        printf(GRN "# " RST);
                        break;
                    case -1:
                        printf(YEL "@ " RST);
                        break;
                    case -2:
                        printf(RED "x " RST);
                        break;
                    case 4:
                        printf(GRN "º " RST);
                        break;
                    case 5:
                        printf(BLU "º " RST);
                        break;
                    default:
                        printf("%d ",map[i][j]);
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

    // HUD
    printf("HP: %d/%d   ", player.hp, player.maxhp);
    if(player.stronger){
        printf("Pot: %d Rounds", player.stronger);
    }
    printf("    %d", ttl);
    printf("\n");
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
                    case 4:
                        printf(GRN "º " RST);
                        break;
                    case 5:
                        printf(BLU "º " RST);
                        break;
                    case -3:
                        printf("k ");
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

    // começa com a célula mais ao noroeste do labirinto
    // começa com um padding para prevenir acesso a valores
    // fora da matriz durante a movimentação
    walk(4, 4, grid, size);

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
