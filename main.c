#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>
#include <stdbool.h>
#include <limits.h>

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

#define POTION1 4
#define POTION2 5
#define TRAP1 -2
#define TRAP2 2
#define TRAP3 3

bool DEBUG = false;
bool DIST = false;

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
    int stronger, attr;
}Player;

typedef struct{
    int atk;
    int intel;
    int dex;
    int hp;
    int x, y;
    int frozen;
}Enemy;

// Use it later, if has rendering/raytracing
struct Trap{
    int x, y;
};

char message[100]; // Buffer de mensagens no estilo Rogue/Nethack
bool isPlaying = false; // Variável para indicar se o jogo está rodando ou pausado

int input(); // talvez vai morrer

int ** genMap(int size);
void walk(int x, int y, int ** grid, int size);

int game(Player player, char controls[8], int size);

int spawn(int ** grid, int size, int n, int type);
void checkTraps(Player * player, int ** grid, bool isAuto);

void move(Player * player, int ** grid, int dir_x, int dir_y);
int attack(Player player, Enemy * enemies, int nenemies, int ** grid);

void updateDist(Player player, int ** grid, int ** distMap, int size);
void enemyAction(Player * player, Enemy * enemy, int ** grid, int ** distMap, int size);

void render(Player player, int ** map, int size, int fov, int ttl, int ** distMap);
void inventory(Player * player, char key);

void bossFight(Player player);

Player create();
Enemy spawnEnemy(int ** grid, int size, Player player);
void config(char * controls);

int main(int argc, char *argv[]){
    srand(time(NULL));

    // default input vector
    char controls[8] = {'A', 'B', 'C', 'D', 'c', 'i', 'e','\0'};
    Player player;

    int opt = 0;
    while (opt != 52){ // 52 ~> 4 in ascii
        system(CLEAR);
        printf("\n");
        printf("    [ 1 ] Continuar\n    [ 2 ] Novo Jogo\n    [ 3 ] Opções\n    [ 4 ] Sair");

        opt = getch();
        switch (opt){
            case '2':
                player = create();
                game(player, controls, 33);
                break;
            case '3':
                config(controls);
                break;
            case 'd':
                DEBUG = true;
                break;
        }
    }    
}

// Função para pegar um input de opções
int input(char * txt, int opts){
    int chosen = 0;
    // enquanto a escolha não estiver no intervalo válido
    while(chosen > opts + 48 || chosen <= 48){ 
        system(CLEAR);
        printf("%s ", txt);
        chosen = getch();
    }
    return chosen - 48;
}

int min(int a, int b){
    return((a+b - abs(a-b))/2);
}

int max(int a, int b){
    return((a+b + abs(a-b))/2);
}

int sign(int x){
    return (x > 0) - (x < 0);
}

void config(char * controls){
    char opt = 0, key = 0;
    
    while (opt != 'x'){
        system(CLEAR);

        printf("    Controles\n");
        printf("    [ 1 ] Andar para cima - [ %c ]\n", controls[0]);
        printf("    [ 2 ] Andar para baixo - [ %c ]\n", controls[1]);
        printf("    [ 3 ] Andar para direita - [ %c ]\n", controls[2]);
        printf("    [ 4 ] Andar para esquerda - [ %c ]\n", controls[3]);
        printf("    [ 5 ] Verificar armadilhas - [ %c ]\n", controls[4]);
        printf("    [ 6 ] Inventário - [ %c ]\n", controls[5]);
        printf("    [ 7 ] Atacar - [ %c ]\n", controls[6]);
        printf("\n[ x ] sair\n");

        opt = getch();

        if(opt >  48 && opt < 56){
            system(CLEAR);
            printf("Pressione a tecla desejada\n [ x ] cancelar");
            key = '[';
            while (key == '\033' || key == '['){
                key = getch();
                if(key != 'x'){
                    controls[opt-49] = key;
                }
            }
        }
    }
}

Player create(){
    Player p;
    int sum = 0;

    p.race = input("    " RED "Raça" RST "\n    [ 1 ] Humano\n    [ 2 ] Anão\n    [ 3 ] Elfo", 3);
    p.aling = input("    Alinhamento\n    [ 1 ] Mal\n    [ 2 ] Neutro\n    [ 3 ] Bom", 3);
    if(p.aling == 3){
        p.class = input("    Profissão\n    [ 1 ] Gurreiro\n    [ 2 ] Mago", 2);
    }else{
        p.class = input("    Profissão\n    [ 1 ] Gurreiro\n    [ 2 ] Mago\n    [ 3 ] Ladino", 3);
    }

    /* system(CLEAR);
    printf("    História Prévia\n");
    scanf("%*c%*[^\n]s"); */

    if(p.race == 2){
        p.size = input("    Porte\n    [ 1 ] Médio\n    [ 2 ] Pequeno", 2) + 1;
    }else{
        p.size = input("    Porte\n    [ 1 ] Grande\n    [ 2 ] Médio\n    [ 3 ] Pequeno", 3);
    }

    p.strength = 0;
    p.constitution = 0;
    p.dexterity = 0;
    p.intelligence = 0;

    int * current = &p.strength;
    bool ready = false;
    char opt;
    while (!ready){
        system(CLEAR);

        printf("       "UND "F" RST "orça [ - ] ");
        for(int i = 0; i < 5; i++){
            if(p.strength > i){
                printf(".");
            }else{
                printf(" ");
            }
        }

        printf(" [ + ]\n" UND "C" RST "onstituição [ - ] ");
        for(int i = 0; i < 5; i++){
            if(p.constitution > i){
                printf(".");
            }else{
                printf(" ");
            }
        }

        printf(" [ + ]\n    " UND "D" RST "estreza [ - ] ");
        for(int i = 0; i < 5; i++){
            if(p.dexterity > i){
                printf(".");
            }else{
                printf(" ");
            }
        }

        printf(" [ + ]\n" UND "I" RST "nteligência [ - ] ");
        for(int i = 0; i < 5; i++){
            if(p.intelligence > i){
                printf(".");
            }else{
                printf(" ");
            }
        }

        printf(" [ + ]\n\nPontos restantes: %2d", 10 - sum);
        printf("\n[ x ] Continuar");

        switch (opt = getch()){
            case 'x':
                ready = true;
                break;
            case 'f': // seta para cima
                current = &p.strength;
                break;
            case 'c':
                current = &p.constitution;
                break;
            case 'd':
                current = &p.dexterity;
                break;
            case 'i':
                current = &p.intelligence;
                break;
            case '-':
                if(*current!=0){
                    * current -= 1;
                    sum--;
                }
                break;
            case '+':
                if(*current!=5 && sum < 10){
                    * current += 1;
                    sum++;
                }
                break;
        }
    }

    if(p.race == 2){
        p.strength += 1;
        p.dexterity -= 1;
    }else if(p.race == 3){
        p.dexterity += 1;
        p.constitution -= 1 * (p.constitution > 0);
    }

    // HP é 7 * Constituição
    // Se constiuição for zero, HP = 1
    p.hp = p.constitution * 7 + !p.constitution;
    p.maxhp = p.hp;

    // Deixa o inventário vazio
    for(int i = 0; i < 10; i++){
        p.inventory[i] = 0;
    }
    p.items = 0;
    p.stronger = 0;
    p.attr = (p.class == 1) * p.strength + (p.class == 2) * p.intelligence + (p.class == 3) * p.dexterity;

    return p;
    
}

int game(Player player, char controls[8], int size){
    int **map;
    char opt;
    int boss_x, boss_y;

    int timeToNextEnemy = 10, entityCount = 0;
    //Enemy *enemies;
    //enemies = (Enemy*) malloc(10*sizeof(Enemy));
    Enemy enemies[10];

    map = genMap(size);

    spawn(map, size, size/4, 2);  // Spawn de size/4 armadilhas (representação 2)
    spawn(map, size, size/10, 4); // Spawn de size/4 poções de cura (representação 4)
    spawn(map, size, size/10, 5); // Spawn de size/4 poções de enhance (representação 5)

    // Spawn do personagem em algum canto do labirinto
    int r = rand() % 4;
    player.x = (r % 2) * (size-5) - (~r % 2) * 4;
    player.y = (r < 2) * (size-5) + (r > 1) * 4;
    map[player.y][player.x] = -1;
    
    // posiciona o boss no canto oposto ao player
    boss_x = size - 1 - player.x;
    boss_y = size - 1 - player.y;
    map[boss_y][boss_x] = -10;

    // cria o mapa de distâncias
    int **distMap;
    distMap = (int**) malloc(size*sizeof(int*));
    for(int i = 0; i < size; i++){
        distMap[i] = (int*) malloc(size*sizeof(int));
    }

    int round = 0;

    // Começa o jogo
    while(1 == 1){
        render(player, map, size, 2, timeToNextEnemy, distMap); 

        opt = getch();

        // Passa o round toda vez que um input válido é recebido
        if(strchr(controls, opt)){
            // Passa o round...

            // limpa o buffer a cada ação
            strcpy(message, "\0");

            // verifica se entrou no range da boss fight
            if(abs(player.x - boss_x) <= 1 && abs(player.y - boss_y) <= 1){
                //bossFight(player);
            }
            // reduz o efeito de potencialização a cada turno
            if(player.stronger)
                player.stronger--;

            // Spawn dos monstros
            if(!(--timeToNextEnemy) && entityCount < 9){

                // TODO criar um construtor de inimigos
                int points = 8;
                enemies[entityCount].atk = rand() % 5;
                points -= enemies[entityCount].atk;
                enemies[entityCount].dex = rand() % min(points,5);
                points -= enemies[entityCount].dex;
                enemies[entityCount].intel = rand() % min(points,5);
                
                int pos = spawn(map, size, 1, -3);
                enemies[entityCount].x = pos / size;
                enemies[entityCount].y = pos % size;

                enemies[entityCount].hp = rand() % 10 + 1;
                enemies[entityCount].frozen = 0;

                entityCount++;

                timeToNextEnemy = 10 + rand() % 20;
            }

            // Atualiza as distâncias
            updateDist(player, map, distMap, size);

            // atualiza os monstros
            for(int e = entityCount-1; e >= 0; e--){
                if(enemies[e].hp <= 0){
                    // remove o inimigo do jogo
                    map[enemies[e].y][enemies[e].x] = 0;
                    Enemy buffer = enemies[entityCount-1];
                    enemies[entityCount-1] = enemies[e];
                    enemies[e] = buffer;
                    entityCount--;
                }else{
                    if(!enemies[e].frozen && !(round % ((player.class == 3)+1))){
                        enemyAction(&player, &enemies[e], map, distMap, size);
                    }else{
                        map[enemies[e].y][enemies[e].x] = -3;
                        enemies[e].frozen = false;
                    }
                }
            }

            round++;

            // Switch/Case evitado pelos valores não serem integrais/constantes
            if(opt == controls[0]){ // Up
                move(&player, map, 0, -1);
            }else if(opt == controls[1]){ // Down
                move(&player, map, 0, 1);
            }else if(opt == controls[2]){ // Rigth
                move(&player, map, 1, 0);
            }else if(opt == controls[3]){ // Left
                move(&player, map, -1, 0);
            }else if(opt == controls[4]){
                strcpy(message, ITL "procurando armadilhas..." RST);
                checkTraps(&player, map, false);
            }else if(opt == controls[5]){
                inventory(&player, controls[5]);
            }else if(opt == controls[6]){
                attack(player, enemies, entityCount, map);
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

int attack(Player player, Enemy * enemies, int nenemies, int ** grid){
    // distância de ataque
    // 3 para magos
    // 1 para o resto
    int dist = (player.class % 2) * 2 + 1; 
    for(int e = 0; e < nenemies; e++){
        // se há um inimigo 'in range'
        if(abs(player.x - enemies[e].x) <= dist && abs(player.y - enemies[e].y) <= dist){
            if(rand() % 100 < (player.attr + 1 - enemies[e].dex)*20){
                int hit = player.attr + 1 - enemies[e].intel + (player.stronger > 0) * player.attr; // maior ataque se for 
                if(player.class == 1){
                    hit += player.strength;
                    sprintf(message, "acertou o monstro com %d hitpoints", hit);
                }
                if(player.class == 2){
                    if(rand() % 2 == 1){
                        enemies[e].frozen = true;
                        sprintf(message, "congelou o monstro por 1 turno");
                        grid[enemies[e].y][enemies[e].x] = -4;
                    }
                }
                enemies[e].hp -= hit;
            }
        }
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


void inventory(Player * player, char key){
    bool exit = false; // TODO: change this name
    int opt;

    system(CLEAR);
    while(!exit){
        system(CLEAR);
        printf("\n");

        for(int i = 0; i < player->items; i++){
            printf("    [ %d ]", i+1);
            if(player->inventory[i] == 4){
                printf("poção de regeneração\n");
            }else{
                printf("poção de potencialização\n");
            }
        }

        printf("    [ %c ] ) voltar ao jogo", key);

        opt = getch();

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
        }else if(opt == key){
            exit = true;
        }
    }

}

// Coloca n types em grid aleatóriamente
int spawn(int ** grid, int size, int n, int type){
    int x, y; // posição das traps

    while(n > 0){ // caso nao tenha lugar disponivel, entra em loop infinito
        x = rand() % size;
        y = rand() % size;

        if(grid[y][x] == 0){
            grid[y][x] = type; // Coloca type;
            n--;
        }
    }

    return y + x * size;
}

void updateDist(Player player, int ** grid, int ** distMap, int size){
    // Copia o mapa para o mapa de distância
    for(int i = 0; i < size; i++){
        for(int j = 0; j < size; j++){
            if(grid[i][j] == 1 || grid[i][j] == POTION1 || grid[i][j] == POTION2){
                distMap[i][j] = INT_MAX; // paredes e objetos tem distância máxima
            }else{
                distMap[i][j] = 0;
            }
        }
    }

    int x = player.x, y = player.y, dist;

    int queue[size*size]; 
    queue[0] = x* size + y; // adiciona a posição do jogador como primeiro na fila
    int q = 1; // indice da fila
    int * current = queue; // célula atual
    distMap[y][x] = 0;

    // enquanto a fila não está vazia
    while (q > current - queue){
        x = * current / size;
        y = * current % size;
        dist = distMap[y][x] + 1; // aumenta a distância

        // TODO: REFACTOR THIS!!!

        // verifica os vizinhos e muda sua distância
        if(distMap[y - 1][x] == 0){ //up
            queue[q++] = x * size + (y - 1);
            distMap[y-1][x] = dist;
        }
        if(distMap[y][x - 1] == 0){ // left
            queue[q++] = (x - 1) * size + y;
            distMap[y][x-1] = dist;
        }
        if(distMap[y + 1][x] == 0){ // down
            queue[q++] = x * size + (y + 1);
            distMap[y+1][x] = dist;
        }
        if(distMap[y][x + 1] == 0){ // right
            queue[q++] = (x + 1) * size + y;
            distMap[y][x+1] = dist;
        }

        current++; // vai para o próximo da fila
    }
}


void enemyAction(Player * player, Enemy * enemy, int ** grid, int ** distMap, int size){
    int dist_x = abs(enemy->x - player->x);
    int dist_y = abs(enemy->y - player->y);

    // Verifica se o jogador está no raio
    if(dist_x < 10 && dist_y < 10){
        // Andar
        if(dist_x > 1 || dist_y > 1){
            if(rand() % 10 < 7){
                int shortest = enemy->x*size + enemy->y;
                int dir_y = 0, dir_x = 0;

                // TODO: REFACTOR THIS!!!

                // verifica o quadrado com menor distância
                if(distMap[enemy->y - 1][enemy->x] < distMap[shortest%size][shortest/size]){ //up
                    shortest = enemy->y - 1 + enemy->x * size;
                    dir_y = -1;
                    dir_x = 0;
                }
                if(distMap[enemy->y][enemy->x - 1] < distMap[shortest%size][shortest/size]){ // left
                    shortest = enemy->y + (enemy->x -1)* size;
                    dir_y = 0;
                    dir_x = -1;
                }
                if(distMap[enemy->y + 1][enemy->x] < distMap[shortest%size][shortest/size]){ 
                    shortest = enemy->y + 1 + enemy->x * size;
                    dir_y = 1;
                    dir_x = 0;
                }
                if(distMap[enemy->y][enemy->x + 1] < distMap[shortest%size][shortest/size]){ // left
                    shortest = enemy->y + (enemy->x +1)* size;
                    dir_y = 0;
                    dir_x = 1;
                }


                if(grid[enemy->y + dir_y][enemy->x + dir_x] == 0){
                    grid[enemy->y + dir_y][enemy->x + dir_x] = -3;
                    grid[enemy->y][enemy->x] = 0;
                    enemy->y += dir_y;
                    enemy->x += dir_x;
                }

                updateDist(*player, grid, distMap, size);
            }else{
                
            }
        }else{ // atacar
            if(rand() % 100 < (enemy->atk + 1 - player->dexterity)*20){
                int hit = enemy->atk + 1 - player->intelligence;
                sprintf(message, "monstro atacou com %d hitpoints", hit);
                player->hp -= hit;
            }
        }
    }
}


void render(Player player, int ** map, int size, int fov, int ttl, int **  dist){ // delete ttl
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
                        printf(" . ");
                        break;
                    case 1:
                        printf(GRN " # " RST);
                        break;
                    case -1:
                        printf(YEL " @ " RST);
                        break;
                    case -2:
                        printf(RED " x " RST);
                        break;
                    case 4:
                        printf(GRN " o " RST);
                        break;
                    case 5:
                        printf(CYN " o " RST);
                        break;
                    case -3:
                        printf(MAG " k " RST);
                        break;
                    case -4:
                        printf(BLU " k " RST);
                        break;
                    case -10:
                        printf(BLD YEL " m " RST);
                        break;
                    default:
                        printf(" . ");
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
    if(player.hp > player.maxhp * 0.2){
        printf(BLD "HP:" RST "%d/%d   ", player.hp, player.maxhp);
    }else{ // HP fica vermelho quando chega a 20% do HP maximo
        printf(BLD RED "HP:" RST RED "%d" RST "/%d   ", player.hp, player.maxhp);
    }

    if(player.stronger){
        printf("Pot: " BLU "%d" RST " Rounds", player.stronger);
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
                        printf(GRN "o " RST);
                        break;
                    case 5:
                        printf(BLU "o " RST);
                        break;
                    case -3:
                        printf(MAG "k " RST);
                        break;
                    case -10:
                        printf(BLD YEL "m " RST);
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
