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
#define BLK "\x1B[30m"
#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define YEL "\x1B[33m"
#define BLU "\x1B[34m"
#define MAG "\x1B[35m"
#define CYN "\x1B[36m"
#define WHT "\x1B[37m"
#define RST "\x1B[0m"
#define GRNB "\x1B[42m"

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
    c = getchar();
    //—— restore old settings ———
    res=tcsetattr(STDIN_FILENO, TCSANOW, &org_opts);
    assert(res==0);
    // Previne comportamentos inexperados com caracteres de escape
    fflush(NULL);
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

// Buffer de mensagens
char message[100]; 

// Protótipos de funções
int input();
int ** genMap(int size);
void walk(int x, int y, int ** grid, int size);
int game(Player player, int ** map, int size, Enemy * enemies, int nenemies, int maxEnemies, int rounds, char * controls);
void gameOver(Player player, int size, int round);
int spawn(int ** grid, int size, int n, int type);
void checkTraps(Player * player, int ** grid, bool isAuto);
void move(Player * player, int ** grid, int dir_x, int dir_y);
int attack(Player player, Enemy * enemies, int nenemies, int ** grid);
void updateDist(Player player, int ** grid, int ** distMap, int size);
void enemyAction(Player * player, Enemy * enemy, int ** grid, int ** distMap, int size, bool canWalk);
void render(Player player, int ** map, int size);
void inventory(Player * player, char key);
bool riddle(int n);
bool bossFight(Player player, int ** map, int size);
bool save(Player player, int ** map, int size, Enemy * enemies, int nenemies, int maxEnemies, int rounds, char * controls);
bool load(Player * player, int *** map, int * size, Enemy * enemies, int * nenemies, int * maxEnemies, int * round, char * controls);

Player create(char * controls);
Enemy createEnemy(Player player, int **map, int size);

void config(char * controls);

int main(){
	// define a seed do PRNG
    srand(time(NULL));

    // default input vector
    char controls[10] = {'A', 'B', 'C', 'D', 'c', 'i', 'e', 's', 'a','\0'};
    Player player;
    Enemy enemies[30];
    int nenemies, size, **map, maxEnemies, round;
    bool saveExists = false;

	// stuk intro ascii art
    char splash[] = "    #############\n    #    #      #\n    #  #####  ###\n    ###  ###  ###\n    #############\n    # #  ##  #  #\n    # #  ##    ##\n    ##   ##  #  #\n    #############\n    ";
    system(CLEAR);
    for(int i = 0; i < strlen(splash); i++){
		if(splash[i] == '#'){
    		printf(GRN GRNB "#" RST);
		}else{
			printf("%c", splash[i]);
		}
    }

    getch();
	// end of intro ascii art
	
	// frases do menu inicial
	char quotes[4][100] = {"2019 - Cruelty Free Studios", "1994 - Fresh Vegs Soft", "East Nobern (c)", "a brn joint"};
	int quote = rand() % 4;

	// verifica a existência de um savefile
    FILE* savefile;
    if(savefile = fopen("stuk.save", "r")){
        saveExists = true;
        fclose(savefile);
    }


	// Menu Inicial
    int opt = 0;
    while (opt != '4'){ // 4 -> Sair
        system(CLEAR);
        printf("\n");
        if(saveExists){
            printf("    [ 1 ] Continuar\n    [ 2 ] Novo Jogo\n    [ 3 ] Opções\n    [ 4 ] Sair");
        }else{
            printf("\n    [ 2 ] Novo Jogo\n    [ 3 ] Opções\n    [ 4 ] Sair");
        }

		// splash text
		printf("\n\n    %s", quotes[quote]);

        opt = getch();
        switch (opt){
            case '1':
                if(saveExists){
                    // carrega o jogo do savefile e inicia
                    load(&player, &map, &size, enemies, &nenemies, &maxEnemies, &round, controls);
                    saveExists = game(player, map, size, enemies, nenemies, maxEnemies, round, controls);
                }
                break;
            case '2':
                // Seleciona a dificuldade
                size = input("    Dificuldade\n    [ 1 ] 30x30\n    [ 2 ] 50x50\n    [ 3 ] 90x90", 3);

                // inicializa variáveis que dependem da dificuldade
                maxEnemies = size * 10;

                // tamanho do labirinto = área jogável + padding da matrix
                size = (size == 1) * 37 + (size == 2) * 57 + (size == 3) * 97;

                // inializa as variáveis pro jogo
                player = create(controls);
                Enemy enemies[10];
                nenemies = 0;
                round = 0;
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
                map[size - 1 - player.y][size - 1 - player.x] = -10;
                
                // mostra a história por trás do jogo
                system(CLEAR);
                printf("    Durante a festa real, pessoas beberam um pouco além da conta.\n\
    O rei decidiu que seria uma boa ideia que todos fossem se aventurar no labirinto do castelo.\n\
    A noite caiu e você está com muita dor de cabeça.\n\n    [ x ] Continuar");

                getch();
                // inicia o jogo
                saveExists = game(player, map, size, enemies, nenemies, maxEnemies, round, controls);
                break;
            case '3':
                // entra na tela de configuração dos controles
                config(controls);
                break;
        }
    }    
}

// Função para pegar um input de opções
int input(char * txt, int opts){
    int chosen = 0;
    // enquanto a escolha não estiver no intervalo válido
    while(chosen > opts + '0' || chosen <= '0'){ 
        system(CLEAR);
        printf("%s ", txt);
        chosen = getch();
    }
    return chosen - '0';
}

// Mínimo entre a e b
int min(int a, int b){
    return((a+b - abs(a-b))/2);
}

// Máximo entre a e b
int max(int a, int b){
    return((a+b + abs(a-b))/2);
}

// Configuração dos controles
void config(char * controls){
    char opt = 0, key = 0, *swap;
    
    // x -> sair
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
        printf("    [ 8 ] Salvar - [ %c ]\n", controls[7]);
        printf("    [ 9 ] Salvar e sair - [ %c ]\n", controls[8]);
        printf("\n    [ x ] sair\n");

        opt = getch();

        // Input entreo 1 e 8
        if(opt >= '1' && opt <= '9'){
            system(CLEAR);
            printf("Pressione a tecla desejada\n    [ x ] cancelar");
            key = '[';
            // Igorara caracteres de escape
            while (key == '\033' || key == '['){
                key = getch();
                // x -> Cancela
                if(key != 'x'){
                    // troca a entrada para previnir entradas repetidas
                    if(swap = strchr(controls, key)){
                        *swap = controls[opt-49];
                    }
                    // atribui a nova tecla
                    controls[opt-49] = key;
                }
            }
        }
    }
}

// Função para a criação do personagem
Player create(char * controls){
    Player p;
    int sum = 0;

    // Pega os atributos do player
    p.race = input("    " RED "Raça" RST "\n    [ 1 ] Humano\n    [ 2 ] Anão\n    [ 3 ] Elfo", 3);
    p.aling = input("    " RED "Alinhamento" RST "\n    [ 1 ] Mal\n    [ 2 ] Neutro\n    [ 3 ] Bom", 3);
    if(p.aling == 3){
        p.class = input("    " RED "Profissão" RST "\n    [ 1 ] Gurreiro\n    [ 2 ] Mago", 2);
    }else{
        p.class = input("    " RED "Profissão" RST "\n    [ 1 ] Gurreiro\n    [ 2 ] Mago\n    [ 3 ] Ladino", 3);
    }

    system(CLEAR);
    printf("    História Prévia\n");
    while(getchar() != '\n');

    if(p.race == 2){
        p.size = input("    " RED "Porte" RST "\n    [ 1 ] Médio\n    [ 2 ] Pequeno", 2) + 1;
    }else{
        p.size = input("    " RED "Porte" RST "\n    [ 1 ] Grande\n    [ 2 ] Médio\n    [ 3 ] Pequeno", 3);
    }
    // Fim dos atributos normais

    int attrs[4] = {0};
    char attrsNames[4][20] = {"       Força", "Constituição", "    Destreza", "Inteligência"};
    int current = 0;
    bool ready = false;
    char opt;

    // Tela de seleção dos atributos
    while (!ready){
        system(CLEAR);

        for(int i = 0; i < 4; i++){
            if(current == i){
                printf("-> %s", attrsNames[i]);
            }else{
                printf("   %s", attrsNames[i]);
            }
            printf(" [ - ] ");
            for(int j = 0; j < 5; j++){
                if(attrs[i] > j){
                    printf(".");
                }else{
                    printf(" ");
                }
            }
            printf(" [ + ] \n");
        }

        printf("\nPontos restantes: %2d", 10 - sum);
        printf("\n    [ x ] Continuar");

        opt = getch();
        if(opt == 'x'){
            ready = true;
            break;
        }else if(opt == '+' || opt == controls[2]){
            if(attrs[current]!=5 && sum < 10){
                attrs[current] += 1;
                sum++;
            }
        }else if(opt == '-' || opt == controls[3]){
            if(attrs[current]!=0){
                attrs[current] -= 1;
                sum--;
            }
        }else if(opt == controls[0]){
            current += 3;
            current %= 4;
        }else if(opt == controls[1]){
            current++;
            current %= 4;
        }
    }

    p.strength = attrs[0];
    p.constitution = attrs[1];
    p.dexterity = attrs[2];
    p.intelligence = attrs[3];

    // Fim da seleção de atributos

    // Modifica valores conforme a raça
    if(p.race == 2){
        p.strength += 1;
        p.dexterity -= 1;
    }else if(p.race == 3){
        p.dexterity += 1;
        p.constitution -= 1 * (p.constitution > 0);
    }

    // HP = 4 * Constituição
    // Se constiuição for zero, HP = 1
    p.hp = p.constitution * 4 + !p.constitution;
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

// Cria um inimigo com atributos aleatórios
Enemy createEnemy(Player player, int **map, int size){
    Enemy e;
    int points = 8;

    // Ataque precisa ser >= que a destreza e inteligecia
    // Caso contrario o inimigo não acerta atques / tem dano 0
    e.atk = max(max(rand() % 5 + 1, player.dexterity), player.intelligence);
    points -= e.atk;

    e.dex = rand() % min(points, 5);
    points -= e.dex;
    e.intel = points;
    
    // posiciona o inimigo aleatoriamento no mapa
    int pos = spawn(map, size, 1, -3);
    e.x = pos / size;
    e.y = pos % size;

    e.hp = rand() % 14 + 1;
    e.hp -= e.atk * (e.hp > e.atk);
    e.frozen = 0;

    return e;
}

int game(Player player, int ** map, int size, Enemy * enemies, int nenemies, int maxEnemies, int round, char * controls){
    char opt;
    int timeToNextEnemy = 15;

    // cria o mapa de distâncias
    int **distMap;
    distMap = (int**) malloc(size*sizeof(int*));
    for(int i = 0; i < size; i++){
        distMap[i] = (int*) malloc(size*sizeof(int));
    }

    // Começa o jogo
    while(1){
        // mostra o jogo na tela
        render(player, map, size); 

        opt = getch();

        // Passa o round toda vez que um input válido é recebido
        if(strchr(controls, opt)){
            // Começo das açõs de round

            // limpa o buffer
            strcpy(message, "\0");

            // verifica se entrou no range da boss fight
            for(int i = player.y-1; i <player.y+2; i++){
                for(int j = player.x-1; j < player.x+2; j++){
                    if(map[i][j] == -10){
                        bossFight(player, map, size);
                        gameOver(player, round, size);
                        return 0;
                    }
                }
            }
            
            // reduz o efeito de potencialização a cada turno
            if(player.stronger)
                player.stronger--;

            // Spawn dos monstros
            if(timeToNextEnemy-- <= 0 && nenemies < maxEnemies){
                enemies[nenemies++] = createEnemy(player, map, size);
                timeToNextEnemy = 15 + rand() % 15;
            }

            // fim das ações de round

            // Input Handling
            // Switch/Case evitado pelos valores não serem integrais/constantes
            if(opt == controls[0]){ // Up
                move(&player, map, 0, -1);
            }else if(opt == controls[1]){ // Down
                move(&player, map, 0, 1);
            }else if(opt == controls[2]){ // Rigth
                move(&player, map, 1, 0);
            }else if(opt == controls[3]){ // Left
                move(&player, map, -1, 0);
            }else if(opt == controls[4]){ // Procura armadilhas
                strcpy(message, ITL "procurando armadilhas..." RST);
                checkTraps(&player, map, false);
            }else if(opt == controls[5]){ // Verifica inventário
                inventory(&player, controls[5]);
            }else if(opt == controls[6]){ // Ataca
                attack(player, enemies, nenemies, map);
            }else if(opt == controls[7]){ // Salva
                save(player, map, size, enemies, nenemies, maxEnemies, round, controls);
            }else if(opt == controls[8]){ // Salvar e sair
                save(player, map, size, enemies, nenemies, maxEnemies, round, controls);
                return 1;
            }

            // Atualiza as distâncias
            updateDist(player, map, distMap, size);

            // atualiza os monstros
            for(int e = nenemies-1; e >= 0; e--){
                if(enemies[e].hp <= 0){
                    // remove o inimigo do jogo
                    map[enemies[e].y][enemies[e].x] = 0;
                    Enemy buffer = enemies[nenemies-1];
                    enemies[nenemies-1] = enemies[e];
                    enemies[e] = buffer;
                    nenemies--;
                }else{
                    if(!enemies[e].frozen){
                        enemyAction(&player, &enemies[e], map, distMap, size, !(round % ((player.class == 3)+1)));
                        map[enemies[e].y][enemies[e].x] = -3;
                    }else{
                        map[enemies[e].y][enemies[e].x] = -4;
                        enemies[e].frozen = false;
                    }
                }
            }

            round++;

            if(player.hp <= 0){
                gameOver(player, round, size);
                return 0;
            }
        }
    }
}

void move(Player * player, int ** grid, int dir_x, int dir_y){
    // verificação automática de armadilhas
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
    int dist = (player.class == 2) * 1 + 1; 
    for(int e = 0; e < nenemies; e++){
        // se há um inimigo 'in range'
        if(abs(player.x - enemies[e].x) <= dist && abs(player.y - enemies[e].y) <= dist){
            // Canche de acertar o ataque = atibuto + 1 - destreza do inimgo / 5
            if(rand() % 100 < (player.attr + 1 - enemies[e].dex)*20){
                // Se inteligência do inimigo > atributo + 1, hit = 1
                int hit = max(player.attr + 1 - enemies[e].intel + (player.stronger > 0) * player.attr, 1);
                sprintf(message, "acertou o monstro com %d hitpoints", hit);
                // Guerreiro tem mais dano
                if(player.class == 1){
                    hit += player.strength;
                }
                // Mago pode congelar
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
            // Verificação automática só ocorre uma vez
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
    bool done = false;
    int opt;

    system(CLEAR);
    while(!done){
        system(CLEAR);
        printf("\n");

        for(int i = 0; i < player->items; i++){
            printf("    [ %d ] ", i+1);
            if(player->inventory[i] == 4){
                printf(GRN "poção de regeneração\n" RST);
            }else{
                printf(BLU "poção de potencialização\n" RST);
            }
        }

        printf("    [ %c ] voltar ao jogo", key);

        opt = getch();

        if(opt > 48 && opt <= player->items + 48){
            // Se for poção de regeneração
            if(player->inventory[opt-49] == 4){
                strcpy(message, "usou " GRN "poção de regeneração" RST);

                // regenera a vida do player
                player->hp = min(player->hp + player->constitution, player->maxhp);
            }else{
                strcpy(message, "usou " BLU "poção de potencialização" RST);
                
                // potencializa o player por x rounds
                player->stronger = 70;
            }

            // remove o item do inventário
            for(int i = opt - 49; i < player->items; i++){
                player->inventory[opt - 49] = player->inventory[opt-48];
            }
            player->inventory[--player->items] = 0;
            
            done = true;
        }else if(opt == key){
            done = true;
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

    // retorna a posição do último objeto colocado
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

    int queue[size*size]; // cria a fila
    queue[0] = x* size + y; // adiciona a posição do jogador como primeiro na fila
    int q = 1; // indice da fila
    int * current = queue; // célula atual
    distMap[y][x] = 0;

    // enquanto a fila não está vazia
    while (q > current - queue){
        x = * current / size;
        y = * current % size;
        dist = distMap[y][x] + 1; // aumenta a distância

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


void enemyAction(Player * player, Enemy * enemy, int ** grid, int ** distMap, int size, bool canWalk){
    int dist_x = abs(enemy->x - player->x);
    int dist_y = abs(enemy->y - player->y);

    // Verifica se o jogador está no raio
    if(dist_x < 10 && dist_y < 10){
        // Andar
        if(dist_x <= 1 && dist_y <= 1){
            // atacar
            if(rand() % 100 < (enemy->atk + 1 - player->dexterity)*20){
                int hit = enemy->atk + 1 - player->intelligence;
                sprintf(message, "monstro atacou com %d hitpoints", hit);
                player->hp -= hit;
            }
        }else if(rand() % 10 < 7 && canWalk){
            int shortest = enemy->x*size + enemy->y;
            int dir_y = 0, dir_x = 0;

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
            if(distMap[enemy->y + 1][enemy->x] < distMap[shortest%size][shortest/size]){ // down
                shortest = enemy->y + 1 + enemy->x * size;
                dir_y = 1;
                dir_x = 0;
            }
            if(distMap[enemy->y][enemy->x + 1] < distMap[shortest%size][shortest/size]){ // left
                shortest = enemy->y + (enemy->x +1)* size;
                dir_y = 0;
                dir_x = 1;
            }

            // movimenta o inimigo para a direção calculada
            if(grid[enemy->y + dir_y][enemy->x + dir_x] == 0){
                grid[enemy->y + dir_y][enemy->x + dir_x] = -3;
                grid[enemy->y][enemy->x] = 0;
                enemy->y += dir_y;
                enemy->x += dir_x;
            }
        }
    }
}

bool bossFight(Player player, int ** map, int size){
    char opt, lines[20][100] = {"Você me achou!", "Hora do desafio final.", "Mas antes...", "Vamos brincar um pouco de jokenpo :)", " ", " ", " ", " ", "Agora sim!", "O desafio final!", " ", " ", " ", "Parabéns!!", "Você completou o desafio final", "Vamos sair daqui agora.", ".", "..", "...", "Onde é a saída mesmo??"};
    int line = 0, r, pastRiddles[2] = {-1,-1};
    int damageMod = 1; // modificador de dano aumenta a cada erro
    while(player.hp > 0){
        render(player, map, size);
        
        opt = getch();
        if(opt != '\033' &&  opt != '['){
            // nas falas 5 - 7 joga jokenpo
            if(line > 4 && line < 8){
                // pedra < papel < tesoura
                switch((input("    [ 1 ] pedra\n    [ 2 ] papel\n    [ 3 ] tesoura", 3) - rand() % 3) % 3){
                    case 2:
                        strcpy(lines[line], "Ótima escolha");
                        break;
                    case 1:
                        strcpy(lines[line], "Não vale me copiar :3");
                        break;
                    default:
                        strcpy(lines[line], "Que pena, eu ganhei xD");
                        player.hp -= damageMod;
                        damageMod *= 2;
                }
            // nas falas 11 - 13 da as charadas;
            }else if(line > 10 && line < 14){
                r = pastRiddles[0];
                while(r == pastRiddles[0] || r == pastRiddles[1]){
                    r = rand() % 10;
                }
                pastRiddles[line%2] = r;
                if(riddle(r) == 0){
                    strcpy(lines[line], "ERRROU!");
                    player.hp--;
                    damageMod *= 2;
                }else{
                    strcpy(lines[line], "Acerto mizeravi");
                }
            }else if(line > 20){
                return 1;
            }
            // coloca a fala do boss no buffer de mensagens
            sprintf(message, ITL "%s" RST, lines[line]);
            line++;
        }
    }
    return 0;
}

// Charadas
bool riddle(int n){
    switch (n)
    {
        case 1:
            return input("Quatro irmãs estão em um quarto: Ana está lendo, Kátia está jogando xadrez, Taca está cozinhando. O que a quarta irmã está fazendo?\n\
    [ 1 ] matando orcs\n    [ 2 ] forjando uma espada\n    [ 3 ] jogando xadrez ", 3) == 3;
        case 2:
            return input("Um homem estava indo para a Bahia com suas 5 irmãs. Cada irmã carregava 5 caixas, cada caixa tinha 5 gatos, cada gato estava com 5 filhotes. Quantos estavam indo para a Bahia?\n\
    [ 1 ] 756\n    [ 2 ] 781\n    [ 3 ] Bahia? ", 3) == 1;
        case 3:
            return input("Você entra em uma sala escura. No quarto há uma estufa à gás, uma luminária de querosene e uma vela. Há uma caixa de fósforo com um só fósforo em seu bolso. O que você acende primeiro.\n\
    [ 1 ] luminária\n    [ 2 ] vela\n    [ 3 ] fósforo", 3) == 3;
        case 4:
            return input("Um empresário comprou um cavalo de 10 moedas e vendeu por 20. Logo comprou o mesmo cavalo por 30 moedas e vendeu por 40. Qual é o lucro total do empresário nessas duas transações?\n\
    [ 1 ] 10\n    [ 2 ] 20\n    [ 3 ] 40 ", 3) == 2;
        case 5:
            return input("Um balão aerostático é levado por uma corrente de ar até o sul. Em que direção vão ondular as bandeiras da cesta?\n\
    [ 1 ] sul\n    [ 2 ] nenhuma\n    [ 3 ] norte", 3) == 2;
        case 6:
            return input("Um homem roubou 80 moedas da caixa de um mercante. Mais tarde, usou 60 moedas para comprar uma espada do mercante, usando as moedas que roubou. Qual foi o prejuízo do mercante?\n\
    [ 1 ] 80\n    [ 2 ] 20\n    [ 3 ] 140 ", 3) == 1;
        case 7:
            return input("Dois pais e dois filhos sentaram-se para comer ovos no café da manhã. Cada um comeu um ovo. Quantos ovos eles comeram no total?\n\
    [ 1 ] 1\n    [ 2 ] 3\n    [ 3 ] 5 ", 3) == 2;
        case 8:
            return input("Se 3 lenhadores derrubam 3 árvores a cada 3 horas, quanto tempo levarão 100 lenhadores para derrubarem 100 árvores?\n\
    [ 1 ] 100\n    [ 2 ] 3\n    [ 3 ] 300 ", 3) == 2;
        case 9:
            return input("Você está diante de três portas. Na primeira há um assassino. Na segunda há um leão que não come há um ano. Na terceira há um incêndio. Qual porta é mais segura?\n\
    [ 1 ] assassino\n    [ 2 ] leão\n    [ 3 ] incêndio ", 3) == 2;
        case 0:
            return input("Há três baús, um contendo 100 moedas de ouro, um contendo 100 moedas de prata, e um contendo 50/50. Os rótulos estão trocados, porém. Você pode tirar uma moeda de um dos baús para identificar qual baú contém apenas moedas de ouro. De qual baú você retira a moeda?\n\
    [ 1 ] só ouro\n    [ 2 ] só prata\n    [ 3 ] 50/50 ", 3) == 3;
    }
}

// Fim do jogo
void gameOver(Player player, int round, int size){
    system(CLEAR);

    if(player.hp > 0){
        printf(GRN BLD"    GAME OVER!" RST);
    }else{
        printf(RED BLD"    GAME OVER!" RST);
    }

    printf(ITL "\n    Dificuldade: " "%.f", (size - 7) / 30.0);
    printf(ITL "\n    Rounds:" RST " %d", round);
    
    strcpy(message, "\0");
    // Deleta o save
    remove("stuk.save");
    getch();
}

// Mostra o jogo na tela
void render(Player player, int ** map, int size){
    system(CLEAR);
    printf("\n");
    // 
    for(int i = player.y - 2; i <= player.y + 2; i++){
        printf("    ");
        for(int j = player.x - 2; j <= player.x + 2; j++){
            if(i >= 0 && i < size && j >= 0 && j < size){
                if(i == player.y && j == player.x){
                    if(map[i][j] == -3 || map[i][j] == -4){
                        printf(MAG " @\u034A\u0356 " RST);
                    }else{
                        printf(YEL " @ " RST);
                    }
                    
                }else{
                    switch (map[i][j])
                        {
                        case 0:
                            printf(" . ");
                            break;
                        case 1:
                            printf(BLK GRNB " # " RST);
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
    printf("\n");
    printf("%s\n", message);
}

bool save(Player player, int ** grid, int size, Enemy * enemies, int nenemies, int maxEnemies, int round, char * controls){
    FILE* savefile;
    savefile = fopen("stuk.save", "wb");

    fwrite(&player, sizeof(Player), 1, savefile);
    fwrite(&size, sizeof(int), 1, savefile);
    for(int i = 0; i < size; i++){
        fwrite(grid[i], sizeof(int), size, savefile);
    }
    fwrite(&nenemies, sizeof(int), 1, savefile);
    fwrite(&maxEnemies, sizeof(int), 1, savefile);
    fwrite(&round, sizeof(int), 1, savefile);
    fwrite(enemies, sizeof(Enemy), nenemies, savefile);
    fwrite(controls, sizeof(char), 9, savefile);
    fclose(savefile);
    strcpy(message, "Jogo Salvo");
}

bool load(Player * player, int *** grid, int * size, Enemy * enemies, int * nenemies, int * maxEnemies, int * round, char * controls){
    FILE* savefile;
    savefile = fopen("stuk.save", "rb");
    
    fread(player, sizeof(Player), 1, savefile);
    fread(size, sizeof(int), 1, savefile);
    *grid = (int **) malloc(*size * sizeof(int*));
    for(int i = 0; i < *size; i++){
        grid[0][i] = (int *) malloc(*size * sizeof(int));
        fread(grid[0][i], sizeof(int), *size, savefile);
    }
    fread(nenemies, sizeof(int), 1, savefile);
    fread(maxEnemies, sizeof(int), 1, savefile);
    fread(round, sizeof(int), 1, savefile);
    fread(enemies, sizeof(Enemy), *nenemies, savefile);
    fread(controls, sizeof(char), 9, savefile);
    fclose(savefile);
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
    
    for(int i = 0; i < 3; i++){ // para cada direção ( não vai na direção que veio )
        int neighbours[3]; // Vetor de vizinhos possíveis
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
