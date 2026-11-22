#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define NUM_POSITIONS 13
#define NUM_SHIELDS 3

// Vetores separados permitem atualizar inimigos e armas
// independentemente.
const char ENEMY_TYPES[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'N'
};
const char ENEMYS_NIGHT[] = {'0', '2', '4', '6', '8', 'N'};
const char WEAPONS[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'n'
};

// 14 10 5)))       1071
// 14 pontos, 10 tiros, arma 5. inimigos 1071
// N vira n ao tomar tiro

// A cada onda recarrega a munição do jogador e ataques inativos do inimigo ( feito )
// A onda termina quando os ataques do inimigo terminam ( feito )
// Espaço emite sons de cada elemento no campo ( feito )
// Ao finalizar uma onda:
    // Contabiliza os pontos ( feito )
    // aperta 'r' para começar a próxima ( feito )
    // No início da nova onda sorteia se será noturna ou diurna ( feito )

// Pontuacao por eliminacao de inimigos e bonus de fim de onda ( feito )
 // 1 ponto na posição mais a direita ( feito )
 // soma 1 ponto conforme está mais a esquerda ( feito )
  // inimigo do tipo n vale o dobro ( feito )
  // no final da onda, os tiros restantes valem 2 pontos cada, cada escudo ( feito )
  // não destruído vale 10 ( feito )

// Diurno: ( feito )
    // Tempo entre movimentação dos aliens 2s - 10% a cada rodada
    // Número de tiros: 30
    // Número de ataques: 20
    // 11 tipos de ataques
    // 13 posições
// Noturno: ( feito )
    // Tempo de movimentação é o triplo do diurno
    // Número de tiros: 30
    // Número de ataques: 15
    // 6 tipos de ataques
    // 8 posições

// Primeira onda tem 100% de chance de ser diurna, vai diminuindo 20% por
// rodada até chegar em 20% de chance

typedef struct timespec crono;

// Guarda a hora atual no cronometro.
void crono_inicia(crono *c)
{
    clock_gettime(CLOCK_MONOTONIC, c);
}

// Retorna os segundos decorridos desde o inicio do cronometro.
double crono_parcial(crono *c)
{
    crono agora;
    clock_gettime(CLOCK_MONOTONIC, &agora);

    double segundos = agora.tv_sec - c->tv_sec;
    double nanosegundos = agora.tv_nsec - c->tv_nsec;

    return segundos + 1e-9 * nanosegundos;
}

typedef struct {
    int wave;
    int score;
    int ammo;
    int pending_attacks;
    int records[3];
    bool night;
    int field_size;

    char weapon;
    char field[NUM_POSITIONS]; //Responsável pelas posições e mapa
    bool lost;

    crono timer;
    double interval;
    double next_move;
} Game;

// Sorteia se a onda sera noturna conforme seu numero.
bool isNight(int wave)
{
    int day_chance = 100 - 20 * (wave - 1);

    if (day_chance < 20) {
        day_chance = 20;
    }

    return rand() % 100 >= day_chance;
}

// Carrega os tres recordes, usando zero nas entradas ausentes.
void load_records(Game *game)
{
    for (int i = 0; i < 3; i++) {
        game->records[i] = 0;
    }

    FILE *file = fopen("recordes.txt", "r");
    if (file == NULL) {
        return;
    }

    for (int i = 0; i < 3; i++) {
        if (fscanf(file, "%d", &game->records[i]) != 1) {
            break;
        }
    }

    fclose(file);
}

// Acrescenta ao comando o arquivo de som de um objeto.
void append_sound(char *command, char object)
{
    char file[3] = {object, '\0'};
    switch (object) {
        case 'N':
        case 'n':
            strcpy(file, "11");
            break;
        case ')':
            strcpy(file, "12");
            break;
        case ' ':
            strcpy(file, "x");
            break;
        default:
            break;
    }

    char path[32];
    snprintf(path, sizeof(path), " Sons/%s.3.wav", file);
    strcat(command, path);
}

// Reproduz os objetos do campo em sequencia.
void sonar(const Game *game)
{
    char command[512] = "aplay -q";

    for (int i = 0; i < game->field_size; i++) {
        append_sound(command, game->field[i]);
    }

    system(command);
}

// Verifica se existe um inimigo na posicao da base.
bool isGameLost(const Game *game)
{
    return game->field[0] != ' ' && game->field[0] != ')';
}

// Move os inimigos e resolve as colisoes com os escudos.
void attack_foward(Game *game)
{
    for (int i = 1; i < game->field_size; i++) {
        char attack = game->field[i];

        if (attack == ' ' || attack == ')') {
            continue;
        }

        if (game->field[i - 1] == ')') {
            game->field[i - 1] = ' ';
        } else {
            game->field[i - 1] = attack;
        }

        game->field[i] = ' ';
    }
}

// Reproduz o som de um objeto sem esperar sua conclusao.
void play_sound(char object)
{
    char command[80];

    if (object >= '0' && object <= '9') {
        snprintf(command, sizeof(command),
                 "aplay -q Sons/%c.3.wav &", object);
    } else if (object == 'N' || object == 'n') {
        snprintf(command, sizeof(command), "aplay -q Sons/11.3.wav &");
    } else if (object == ')') {
        snprintf(command, sizeof(command), "aplay -q Sons/12.3.wav &");
    } else if (object == ' ') {
        snprintf(command, sizeof(command), "aplay -q Sons/x.3.wav &");
    } else {
        return;
    }

    system(command);
}

// Sorteia um inimigo e desconta um ataque pendente.
void create_enemy(Game *game)
{
    if (game->pending_attacks == 0) {
        return;
    }

    char enemy = game->night ? ENEMYS_NIGHT[rand() % 6]
                            : ENEMY_TYPES[rand() % 11];
    game->field[game->field_size - 1] = enemy;
    game->pending_attacks--;
    play_sound(game->field[game->field_size - 1]);
}

// Processa os movimentos previstos para o tempo decorrido.
void check_time(Game *game)
{
    double elapsed = crono_parcial(&game->timer);

    while (!game->lost && elapsed >= game->next_move) {
        game->lost = isGameLost(game);

        if (!game->lost) {
            attack_foward(game);
            create_enemy(game);
        }

        game->next_move += game->interval;
    }
}

// Inicializa a partida, preservando os recordes carregados.
void initialize_game(Game *game)
{
    game->wave = 1;
    game->score = 0;
    game->ammo = 30;
    game->weapon = WEAPONS[0];
    game->lost = false;
    game->pending_attacks = 20;
    game->night = false;
    game->field_size = NUM_POSITIONS;

    for (int i = 0; i < NUM_POSITIONS; i++) {
        game->field[i] = ' ';
    }

    for (int i = 0; i < NUM_SHIELDS; i++) {
        game->field[i] = ')';
    }

    game->interval = 2.0;
    game->next_move = game->interval;
    crono_inicia(&game->timer);
}

// Le uma tecla ou retorna zero quando nenhuma tecla foi recebida.
char lechar(void)
{
    fflush(stdout);
    char c;

    if (fread(&c, 1, 1, stdin) == 1) {
        return c;
    }

    clearerr(stdin);
    return 0;
}

// Configura o terminal para a leitura individual das teclas.
void configura_terminal(void)
{
    if (system("stty raw opost -echo min 0 time 1") != 0) {
        perror("erro na execução de system(\"stty\")");
        fprintf(stderr, "você tem o programa stty instalado?\n");
        exit(1);
    }
    if (setvbuf(stdin, NULL, _IONBF, 0) != 0) {
        perror("erro na execução de setvbuf()");
        exit(1);
    }
}

// Restaura o terminal ao finalizar o programa.
void normaliza_terminal(void)
{
    system("stty sane");
}

// Mostra o estado do jogo, ocultando o campo durante a noite.
void render_game(const Game *game)
{
    if (game->night) {
        printf("%3d", game->score);
    } else {
        printf("%3d %2d %c", game->score, game->ammo, game->weapon);

        for (int i = 0; i < game->field_size; i++) {
            putchar(game->field[i]);
        }
    }

    printf("%40s\r", "");
    fflush(stdout);
}

// Verifica se acabaram os ataques pendentes e ativos.
bool isWaveFinished(const Game *game)
{
    if (game->pending_attacks > 0) {
        return false;
    }

    for (int i = 0; i < game->field_size; i++) {
        if (game->field[i] != ' ' && game->field[i] != ')') {
            return false;
        }
    }

    return true;
}

// Seleciona a proxima arma disponivel e toca seu som.
void change_weapon(Game *game)
{
    int i = 0;
    int counter = game->night ? 2 : 1;

    // Encontra a arma atual
    while (game->weapon != WEAPONS[i]) {
        i += counter;
    }

    // Pega a próxima arma
    i += counter;

    // Se for a última, reseta pra zero
    if (i >= 11) {
        i = 0;
    }

    game->weapon = WEAPONS[i];
    play_sound(game->weapon);
}

// Pontua a eliminacao e remove o inimigo do campo.
void destroy_enemy(Game *game, int position)
{
    int points = game->field_size - position;

    if (game->field[position] == 'n') {
        points *= 2;
    }

    game->score += game->night ? points * 2 : points;
    game->field[position] = ' ';
    play_sound(game->weapon);
}

// Consome um tiro e acerta o primeiro alvo compativel.
void shoot(Game *game)
{
    if (game->ammo == 0) {
        return;
    }
    game->ammo--;
    int i = 0;

    while (i < game->field_size) {
        if (game->field[i] == game->weapon) {
            destroy_enemy(game, i);
            return;
        } else if (game->field[i] == 'N' && game->weapon == 'n') {
            game->field[i] = 'n';
            play_sound(game->weapon);
            return;
        }
        i++;
    }

    play_sound(' ');
}

// Processa uma tecla e retorna false quando ela e Esc.
bool process_key(Game *game, char key)
{
    if (key == 27) {
        return false;
    }
    if (key == '\t') {
        change_weapon(game);
    }
    if (key == '\r' || key == '\n') {
        shoot(game);
    }
    if (key == ' ') {
        sonar(game);
    }

    return true;
}

// Executa uma onda e retorna false quando o jogador desiste.
bool execute_wave(Game *game)
{
    while (!game->lost && !isWaveFinished(game)) {
        char key = lechar();
        if (!process_key(game, key)) {
            return false;
        }
        if (!isWaveFinished(game)) {
            check_time(game);
        }
        render_game(game);
    }

    return true;
}

// Soma os pontos bonus ( 10 pra cada escudo, qtd de municao * 2), dobra se
// for noite
void wave_bonus(Game *game)
{
    int bonus = game->ammo * 2;

    for (int i = 0; i < NUM_SHIELDS; i++) {
        if (game->field[i] == ')') {
            bonus += 10;
        }
    }

    if (game->night) {
        bonus *= 2;
    }

    game->score += bonus;
}

// Aguarda r para continuar ou Esc para encerrar.
char want_continue(void)
{
    char key;

    do {
        key = lechar();
    } while (key != 'r' && key != 27);

    return key;
}

// Prepara outra onda, preservando os pontos e escudos.
void next_wave(Game *game)
{
    if (game->night) {
        game->interval /= 3.0;
    }

    game->interval *= 0.9;
    game->wave++;
    game->night = isNight(game->wave);

    game->ammo = 30;
    game->field_size = game->night ? 8 : NUM_POSITIONS;
    game->pending_attacks = game->night ? 15 : 20;
    game->weapon = WEAPONS[0];

    if (game->night) {
        game->interval *= 3.0;
    }

    game->next_move = game->interval;
    crono_inicia(&game->timer);
}

// Toca o som de conclusao e apresenta o resumo da onda.
void show_wave_summary(const Game *game)
{
    system("aplay -q Sons/0.3.wav Sons/4.3.wav Sons/8.3.wav");
    printf("\nOnda %d concluída! Pontos: %d\n",
           game->wave, game->score);
    printf("Pressione r para continuar ou Esc para sair.\n");
}

// Inicializa a partida e controla a passagem entre suas ondas.
void call_match(Game *game)
{
    initialize_game(game);
    render_game(game);

    while (!game->lost) {
        if (!execute_wave(game) || game->lost) {
            break;
        }

        wave_bonus(game);
        show_wave_summary(game);
        if (want_continue() == 27) {
            break;
        }

        next_wave(game);
        render_game(game);
    }
}

// Insere a pontuacao no ranking e informa se entrou nos recordes.
void update_records(Game *game)
{
    int aux = game->score;
    bool isGreat = false;

    for (int i = 0; i < 3; i++) {
        if (aux > game->records[i]) {
            int previous = game->records[i];
            game->records[i] = aux;
            aux = previous;
            isGreat = true;
        }
    }

    if (isGreat) {
        printf("\nSua pontuacao entrou nos tres recordes!\n");
    } else {
        printf("\nSua pontuacao ficou fora dos tres recordes.\n");
    }
}

// Grava os tres recordes e informa falhas de escrita.
void save_records(const Game *game)
{
    FILE *file = fopen("recordes.txt", "w");
    if (file == NULL) {
        perror("Erro ao abrir os recordes");
        return;
    }

    bool success = true;
    for (int i = 0; i < 3; i++) {
        if (fprintf(file, "%d\n", game->records[i]) < 0) {
            success = false;
            break;
        }
    }

    if (fclose(file) != 0) {
        success = false;
    }

    if (!success) {
        fprintf(stderr, "Nao foi possivel salvar todos os recordes.\n");
    }
}

// Prepara o terminal e permite jogar novas partidas.
int main(void)
{
    Game game;
    load_records(&game);
    configura_terminal();
    srand((unsigned int)time(NULL));

    do {
        call_match(&game);
        update_records(&game);
        save_records(&game);
        system("aplay -q Sons/8.3.wav Sons/4.3.wav Sons/0.3.wav");
        printf("\nFim da partida! Pontos: %d\n", game.score);
        printf("Pressione r para nova partida ou Esc para sair.\n");
    } while (want_continue() == 'r');

    normaliza_terminal();
    return 0;
}
