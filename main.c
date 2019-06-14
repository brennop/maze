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
void enter();
struct Player create();

int main(){
    srand(time(NULL));

    //struct Player player = create();

    game();
}

int game(){
    int map[30][30], opt;

    for(int i = 0; i < 10; i++){
        for(int j = 0; j < 10; j++){
            map[i][j] = rand() % 3 - 1;
        }
    }


    while(1 == 1){

        system(CLEAR);
        for(int i = 0; i < 10; i++){
            for(int j = 0; j < 10; j++){
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
            printf("\n");
        }
        
        switch (getch()){
            case -1: //EOF
                exit(0);
                break;
            case 65:
                map[0][1]++;
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

