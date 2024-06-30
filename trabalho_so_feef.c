#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h> 
#include <stdlib.h>

int gun_direction = 2;  // Direcao da arma
int player_x = 10;   // Posição x do jogador 
int player_y = 30;    // Posição y do jogador
pthread_mutex_t mutex;

struct Enemy {
	int y; 
	int x;
	int life;
};

struct Game {
	int max_qt_bullets;
	int current_qt_bullets; 
	int qt_enemy;
	int qt_destroyed_enemy;
	int fall_velocity;
	int difficulty_level;
} game;

struct Bullet {
	int y;
	int x;
	int life;
	int direction_x;
	int direction_y;
	char img[0];
};

struct EnemiesList {
    struct Enemy *enemies;
} enemy_list;  


struct BulletsList {
    struct Bullet *bullets;
} bullet_list;  


int game_state = 0;

void set_difficulty(int difficulty_level) {
	switch (difficulty_level) {
		case 1: //Easy
			game.max_qt_bullets = 10;
			game.qt_enemy = 10;
			game.fall_velocity = 1;
			printf("> Dificuldade alterada para facil!\n");
			
		break;
		case 2: //Medium (default)
			game.max_qt_bullets = 7;
			game.qt_enemy = 16;
			game.fall_velocity = 2;
			printf("> Dificuldade alterada para medio!\n");
		break;
		case 3: //Hard
			game.max_qt_bullets = 4;
			game.qt_enemy = 20;
			game.fall_velocity = 3;
			printf("> Dificuldade alterada para dificil!\n");
		break;
	}
	game.difficulty_level = difficulty_level;
	game.current_qt_bullets = game.max_qt_bullets;
}

void draw_player(int x, int y, int gun_direction) {
    //clear();  // Limpa a tela antes de redesenhar

	if (gun_direction == 0) {
		mvprintw(y, x, " _.");
	} else if (gun_direction == 1) {
		mvprintw(y, x, "  \\  "); 
	} else if (gun_direction == 2) {
		mvprintw(y, x, "  |  ");
	} else if (gun_direction == 3) {
		mvprintw(y, x, "  /  ");
	} else if (gun_direction == 4) {
		mvprintw(y, x+1, " ._  ");
	}
    mvprintw(y + 1, x, " /_\\ ");     
    mvprintw(y + 2, x, "|___|"); 
	
	mvprintw(40, 0, "Municao: %d / %d", game.current_qt_bullets, game.max_qt_bullets); 
	switch (game.difficulty_level) {
		case 1:
			mvprintw(41, 0, "Dificuldade: Facil"); 
		break;
		case 2:
			mvprintw(41, 0, "Dificuldade: Medio"); 
		break;
		case 3:
			mvprintw(41, 0, "Dificuldade: Dificil"); 
		break;
	}
	mvprintw(42, 0, "Quantidade inimigos destruidos: %d%%", (game.qt_destroyed_enemy * 100) /game.qt_enemy); 
    refresh();
}

void *move_player(void *arg) {
    while (true) {
        // Check if player is inside of the screen
        if (player_y < 0) player_y = 0;
        if (player_y > LINES - 3) player_y = LINES - 3;

		if (player_x < 0) player_x = 0;
		if (player_x > LINES - 3) player_x = LINES - 3;

        draw_player(player_x, player_y, gun_direction);
		
        usleep(50000); 
    }
    return NULL;
}

void *enemy_move(void *arg) {
	struct Enemy enemy;

	enemy.life = true;
	
	while (enemy.life) {
		clear();
		mvprintw(enemy.y, enemy.x, "\\-/"); 
		enemy.y += game.fall_velocity;
		
		if (enemy.y >= 34) {
			enemy.life = false;
		}
		
		usleep(80000);
		refresh();
	}
	clear();
	pthread_exit(NULL);
}


void *create_enemy(void *arg) {
    // Encontra um slot livre no array de inimigos
	if (rand() % 100 < 50) {
		for (int i = 0; i < game.qt_enemy; ++i) {
			if (!enemy_list.enemies[i].life) {
				enemy_list.enemies[i].y = 0;
				enemy_list.enemies[i].x = rand() % (COLS - 2) + 1; // Posição aleatória na largura da tela
				enemy_list.enemies[i].life = true;

				// Cria a thread para o novo inimigo
				pthread_t enemy_thread;
				pthread_create(&enemy_thread, NULL, enemy_move, (void *)&enemy_list.enemies[i]);

				break;  // Sai do loop ao criar um novo inimigo
			}
		}
	}
	return NULL;
}

void *draw_scenario(void *arg) {
	while (true) {
		//clear();
		usleep(50000);
	}
	return NULL;
}

void init_game() {
    initscr();
    cbreak();  
    noecho(); 
    keypad(stdscr, TRUE); 

    // Init o mutex
    pthread_mutex_init(&mutex, NULL);

    // Create a thread for player
    pthread_t player_thread;
    pthread_create(&player_thread, NULL, move_player, NULL);
	
    // Create a thread for scenario
	pthread_t scenario_thread;
    pthread_create(&scenario_thread, NULL, draw_scenario, NULL);

	pthread_t create_enemy_thread;
    pthread_create(&create_enemy_thread, NULL, create_enemy, NULL);

	bullet_list.bullets = (struct Bullet *)malloc(game.max_qt_bullets * sizeof (struct Bullet));
	enemy_list.enemies = (struct Enemy *)malloc(20 * sizeof (struct Enemy));
}


void *bullet_move(void *arg) {
	struct Bullet *bullet = NULL;
	for (int i = 0; i < game.max_qt_bullets; ++i) {
		if (!bullet_list.bullets[i].life) {
			bullet = &bullet_list.bullets[i];
			break;
		}
	}
	bullet->y = player_y;
	bullet->x = player_x;
	bullet->life = true;
	bullet->direction_x = 0;
	bullet->direction_y = 0;
	bullet->img[0] = '.';

	switch (gun_direction) {
		case 0:
			bullet->x -= 1;
			bullet->direction_x = -1;
			bullet->img[0] = '_';
		break;
		case 1:
			bullet->direction_x = -1;
			bullet->direction_y = -1;
			bullet->img[0] = '\\';
		break;
		case 2:
			bullet->direction_y = -1;
			bullet->img[0] = '|';
		break;
		case 3:
			bullet->direction_x = 1;
			bullet->direction_y = -1;
			bullet->img[0] = '/';
		break;
		case 4:
			bullet->x += 2;
			bullet->direction_x = 1;
			bullet->img[0] = '_';
		break;
	}
	
	bullet->x -= 2;
	while (bullet->life) {
		clear();
		mvprintw(bullet->y, bullet->x + 4, bullet->img); 
		bullet->x += bullet->direction_x;
		bullet->y += bullet->direction_y;
		if (bullet->y <= -2 || bullet->x < -2 || bullet->x >= 70 + 5) {
			bullet->life = false;
		}
		
        usleep(50000);
		refresh();
	}
	clear();
	pthread_exit(NULL);
}


void hud_interactions (int input) {
	switch (input) {
		case 27: //Esc - leave game
			game_state = 0;
		break;
		case 32: //Space - shoot
			if (game.current_qt_bullets > 0) {
				pthread_mutex_lock(&mutex); 
				game.current_qt_bullets -= 1;
				pthread_mutex_unlock(&mutex); 
				pthread_t bullet_thread;
				pthread_create(&bullet_thread, NULL, bullet_move, NULL);
			}
		break;
		case 97: //D - move left
			player_x += -1;
		break;
		case 100: //A - move right
			player_x += +1;
		break;
		case 114: //R - reload gun
			if (game.current_qt_bullets < game.max_qt_bullets) {
				mvprintw(40, 20, "Recarregando..."); 
				usleep(500000); 
				pthread_mutex_lock(&mutex); 
				game.current_qt_bullets = game.max_qt_bullets;
				pthread_mutex_unlock(&mutex); 
				refresh();
				clear();
			} else {
				mvprintw(40, 20, "Arma ja municiada."); 
			}

		break;
		case 260: //Arrow left - turns gun to left
			gun_direction -= 1;
			break;
		case 261:  //Arrow right - turns gun to right
			gun_direction += 1;
			break;
	}
}

void game_loop() {
	srand(time(NULL));
	int input;
	//create_enemy();	
	while (true) {
		
		input = getch();  // Get keyboard option
		hud_interactions(input);
		
		// Gun direction 
		/* 0: -180º / 1: -45º / 2: +90º / 3: +45º / 4: +180º*/
		if (gun_direction < 0) {
			gun_direction = 0;
		} else if (gun_direction > 4) {
			gun_direction = 4;
		}
	}
}

int main() {
    int option = -1;
	int difficulty_option = -1;
    int input_success;
	set_difficulty(2);
    while (game_state >= 0) {
		if (game_state == 0) {
			clear();
			printf("1. Jogar\n");
			printf("2. Selecionar Dificuldade\n");
			printf("3. Sair\n");
			printf("Escolha uma opcao: ");

			// Check if input is an integer
			input_success = scanf("%d", &option);			
			if (input_success != 1) {
				printf("\n> Opcao invalida. Escolha novamente.\n");
				while (getchar() != '\n');
				continue;
			}
			
			switch(option) {
				case 1:
					game_state = 1;
				break;
				case 2:
					printf("\n");
					printf("1. Facil\n");
					printf("2. Medio\n");
					printf("3. Dificil\n");
					printf("4. Voltar\n");
					printf("Escolha uma opcao: ");
					
					input_success = scanf("%d", &difficulty_option);

					if (difficulty_option > 0 && difficulty_option < 4) {
						set_difficulty(difficulty_option);
					} else {
						printf("\n> Opcao invalida. Escolha novamente.\n");
					}
				break;
				case 3:
					game_state = -1;
				break;
				default:
					printf("\n> Opcao invalida. Escolha novamente.\n");
				break;
			}
			
		} else if (game_state == 1) {
			init_game();
			game_loop();
		}
    }

	printf("\n> Jogo encerrado!\n");
    // Finaliza o jogo
    pthread_mutex_destroy(&mutex);
    endwin();

    return 0;
}