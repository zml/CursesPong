#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <signal.h>
#include "window.h"
#include "items.h"
#define SPEED 80000



int end = 0;
int ch;

pthread_mutex_t mux_char = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t player_lock[2];

void *read_keyboard(){
	
	while(ch!='q')
	{ch = getch();
	
	if(ch==KEY_UP||ch==KEY_DOWN)
		pthread_mutex_unlock(&player_lock[0]);
	if(ch=='w'||ch=='s')
		pthread_mutex_unlock(&player_lock[1]);
	}
	
	end=1;
	pthread_mutex_unlock(&player_lock[0]);
	pthread_mutex_unlock(&player_lock[1]);
	
}
void *player(void *arg){
	int player_num;
	WINDOW *bumper_win;
	int x_pos;
	int up,down;
	
	cbreak(); //line buffering disabled
	keypad(stdscr, TRUE); //enables F_keys
	noecho(); //disables echoing of characters
	
	
	player_num = *(int*)arg; //retrieves player number (0 or 1);

	if(player_num == 0){
		x_pos = win_x + win_width -2; //x coordinate
		bumper_win = create_newwin(win_height-2,1,win_y+1,x_pos,FALSE); //creates bumper window for player 1
		bumpers_y[0] = win_height/2; //initial bumper position
		up = KEY_UP;
		down = KEY_DOWN;
	}
	else if(player_num == 1){
				x_pos = win_x + 1; //x coordinate
				bumper_win = create_newwin(win_height-2,1,win_y+1,win_x+1,FALSE); //creates bumper window for player 2
				bumpers_y[1] = win_height/2;
				up = 'w';
				down = 's';
	}
	
	draw_bumper(bumper_win,win_height/2,0); //draws bumper in the middle of the game window
	
	while(1)
	{

		pthread_mutex_lock(&player_lock[player_num]);
		pthread_mutex_lock(&mux_char);
		
		if(ch=='q')
			{
		      pthread_mutex_unlock(&mux_char); 
			  break;
			}

		if(ch==up)
			move_bumper(bumper_win,0,1); //moves bumper up
					
		if(ch==down)
			move_bumper(bumper_win,0,0); //moves bumper down
					
			
	
		pthread_mutex_unlock(&mux_char);
	}
	
	delwin(bumper_win); //deallocates window
	pthread_exit(0);	
}

int main(int argc, char **argv){

	WINDOW  *game_win;
	int *player_num ;
	int i;
	ball_t ball;
	unsigned int t_inter;
	pthread_t playertid[2],tid;

	
	WindowInit();
	
	refresh();// refresh window
	getch();

	game_win = GameWindowInit();
	
	mvwprintw(win,LINES-1,0,"Press 'q' to Exit      "); //changes bottom message
	
	new_ball(&ball,SPEED,game_win); //creates new ball
	
	WinRefresh(game_win);
	
	for(i=0;i<2; i++){ //creates player threads
		pthread_mutex_init(&mux_bumper[i],NULL); //initializes mutexes
		pthread_mutex_init(&player_lock[i],NULL);
			
		player_num = (int*)malloc(sizeof(int));
		if(player_num == NULL)
			exit(-1);
			
		*player_num = i;
		if(pthread_create(&playertid[i],NULL,player,(void *)player_num) != 0){
			fprintf(stderr,"Thread initialization failed\n");
			endwin();
			exit(-1);
		}
	}
	
	if(pthread_create(&tid,NULL,read_keyboard,NULL) != 0){
		fprintf(stderr,"Thread initialization failed\n");
		end=1;}
	
	while(!end){
		
		move_ball(game_win,win,&ball);
		WinRefresh(game_win);
		usleep(ball.speed);
	}
	
	pthread_join(tid,NULL);
	pthread_join(playertid[0],NULL);
	pthread_join(playertid[1],NULL);
	pthread_mutex_destroy(&player_lock[0]);
	pthread_mutex_destroy(&player_lock[1]);
	endwin(); //end ncurses
	printf("Acabou tudo\n");
	return 0;
}
