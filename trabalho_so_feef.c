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
	int enemy_y; 
	int enemy_x;
	int enemy_life;
};

struct Game {
	int max_qt_bullets;
	int current_qt_bullets; 
	int max_qt_enemy;
	int current_qt_enemy;
} game;

int game_state = 0;

// Função para desenhar o jogador na tela
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
    refresh();
    //refresh();  // Atualiza a tela para exibir as mudanças
}

// Função para movimentar o jogador
void *move_player(void *arg) {
    while (true) {
        // Limita o movimento do jogador dentro da tela
        if (player_y < 0) player_y = 0;
        if (player_y > LINES - 3) player_y = LINES - 3; // -3 para a nave caber na tela

		if (player_x < 0) player_x = 0;
		if (player_x > LINES - 3) player_x = LINES - 3; // -3 para a nave caber na tela

        // Redesenha o jogador na nova posição
        draw_player(player_x, player_y, gun_direction);
		
        //usleep(50000); // Pequena pausa para simular movimento suave
		
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

// Função para inicializar o jogo
void init_game() {
    // Inicializa a biblioteca ncurses
    initscr();
    cbreak();   // Desabilita o buffer de linha para leitura imediata de caracteres
    noecho();   // Não ecoa os caracteres digitados pelo usuário
    keypad(stdscr, TRUE);  // Permite a captura de teclas especiais como setas

    // Inicializa o mutex
    pthread_mutex_init(&mutex, NULL);

    // Cria a thread para movimentar o jogador
    pthread_t player_thread;
    pthread_create(&player_thread, NULL, move_player, NULL);
	
	pthread_t scenario_thread;
    pthread_create(&scenario_thread, NULL, draw_scenario, NULL);
	
}


void *enemy_move(void *arg) {
	int enemy_y = 0;
	int enemy_x = 50;
	int enemy_life = true;
	
	while (enemy_life) {
		//clear();
		mvprintw(enemy_y, enemy_x, "\\-/"); 
		enemy_y += 1;
		
		if (enemy_y >= 34) {
			enemy_life = false;
		}
		
		//usleep(50000);
		refresh();
	}
	//clear();
	game.current_qt_enemy -= 1;
	pthread_exit(NULL);
}

void set_difficulty(int difficulty_level) {
	
	game.current_qt_bullets = 0;
	game.current_qt_enemy= 0;
	switch (difficulty_level) {
		case 1:
			game.max_qt_bullets = 10;
			game.max_qt_enemy = 5;
			printf("> Dificuldade alterada para facil\n");
		break;
		case 2:
			game.max_qt_bullets = 10;
			game.max_qt_enemy = 5;
			printf("> Dificuldade alterada para medio\n");
		break;
		case 3:
			printf("> Dificuldade alterada para dificil\n");
		break;
		case 4:
		break;

	}
}

void *bullet_move(void *arg) {
	int bullet_y = player_y;
	int bullet_x = player_x;
	int bullet_direction = gun_direction;
	int bullet_life = true;
	
	while (bullet_life) {

		switch (bullet_direction) {
			case 0:
				mvprintw(bullet_y, bullet_x, "_"); 
				bullet_x -= 1;
			break;
			case 1:
				mvprintw(bullet_y, bullet_x + 2, "\\"); 
				bullet_x -= 1;
				bullet_y -= 1;
			break;
			case 2:
				mvprintw(bullet_y, bullet_x + 2, "|"); 
				bullet_y -= 1;
			break;
			case 3:
				mvprintw(bullet_y, bullet_x + 2, "/"); 
				bullet_x += 1;
				bullet_y -= 1;
			break;
			case 4:
				mvprintw(bullet_y, bullet_x + 4, "_"); 
				bullet_x += 1;
			break;
		}
		
		
		if (bullet_y <= -2 || bullet_x < -2 || bullet_x >= 70 + 5) {
			bullet_life = false;
		}
		
        //usleep(50000);
	}
	//clear();
	game.current_qt_bullets -= 1;
	pthread_exit(NULL);
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

			// Leitura segura da opção do menu
			input_success = scanf("%d", &option);

			// Verifica se a entrada é um número inteiro
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
					printf("1. Facil\n");
					printf("2. Medio\n");
					printf("3. Dificil\n");
					printf("4. Voltar\n");
					
					if (difficulty_option > 0 && difficulty_option < 4) {
						set_difficulty(difficulty_option);
					}
					switch (difficulty_option) {
						case 1:
							game.max_qt_bullets = 10;
							game.max_qt_enemy = 5;
							printf("> Dificuldade alterada para facil\n");
						break;
						case 2:
							printf("> Dificuldade alterada para medio\n");
						break;
						case 3:
							printf("> Dificuldade alterada para dificil\n");
						break;
						case 4:
						break;
					}
					/*
					int max_qt_bullets;
	int current_qt_bullets; 
	int max_qt_enemy;
	int current_qt_enemy;
} game;
					*/
				break;
				case 3:
					game_state = -1;
				break;
				default:
					printf("> Opcao invalida. \n");
				break;
			}
			
		} else if (game_state == 1) {
			init_game();
			srand(time(NULL));
			int c;
			int numeroAleatorio = rand() % 100 + 1;
			
			while (true) {
				for (int i = 0; i <= 100; ++i) {
			if (i == numeroAleatorio) {
				//mvprintw(0 + 1, 10, "%d",numeroAleatorio);
			} else {
				//mvprintw(0 + 1, 10, " i %d",i);
			}
		}		
		
        c = getch();  // Get keyboard option
		switch (c) {
			case 97: //move left
				player_x += -1;
			break;
			case 100: //move right
				player_x += +1;
			break;
			case 32: //shoot
				if (game.current_qt_bullets < game.max_qt_bullets) {
					pthread_mutex_lock(&mutex); 
					game.current_qt_bullets += 1;
					pthread_mutex_unlock(&mutex); 
					pthread_t bullet_thread;
					pthread_create(&bullet_thread, NULL, bullet_move, NULL);
				}
			break;
			
			case 102:
				if (game.current_qt_enemy < game.max_qt_enemy) {
					pthread_mutex_lock(&mutex); 
					game.current_qt_enemy += 1;
					pthread_mutex_unlock(&mutex); 
					pthread_t enemy_thread;
					pthread_create(&enemy_thread, NULL, enemy_move, NULL);
				}
				
			break;
			
			case 260: // left
				gun_direction -= 1;
				break;
			case 261:  // right 
				//pthread_mutex_lock(&mutex); // Início da região crítica
				gun_direction += 1;
				//pthread_mutex_unlock(&mutex); // Fim da região crítica

				break;
		}
		
		// Check gun direction
		if (gun_direction < 0) {
			gun_direction = 0;
		} else if (gun_direction > 4) {
			gun_direction = 4;
		}
			}
		}
    }

	printf("\n> Jogo encerrado!\n");
    // Finaliza o jogo
    pthread_mutex_destroy(&mutex);
    endwin();

    return 0;
}
