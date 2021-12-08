#include <stdio.h>
#include <unistd.h>

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <ncurses.h>
#include <list>
#include <vector>

#include "settings.h"

#define WIDTH 26
#define HEIGHT 20

#define DELAY 7777
#define MOVEINTERVAL (DELAY/1000) * ((300 - level*13)/5)
//delay is the delay between two commands, here to avoid raping processor

/*
 * A_NORMAL        Normal display (no highlight)
 * A_STANDOUT      Best highlighting mode of the terminal.
 * A_UNDERLINE     Underlining
 * A_REVERSE       Reverse video
 * A_BLINK         Blinking
 * A_DIM           Half bright
 * A_BOLD          Extra bright or bold
 * A_PROTECT       Protected mode
 * A_INVIS         Invisible or blank mode
 * A_ALTCHARSET    Alternate character set
 * A_CHARTEXT      Bit-mask to extract a character
 * COLOR_PAIR(n)   Color-pair number n 
 */

int head[2]; // local variables
int tail[2];
int food[2];

int  tmp_xy[2];  // temp tail
char tmp_c;

int  lucorner[2]; // global
char arena[HEIGHT][WIDTH];
char infos[HEIGHT][WIDTH];


int dead   = 0;
int level  = 1;
int length = 2;

std::list<int> digestion;

struct timeval t1, t2;

void
init_arena()
{
    int center[2];
    //calculate center Point of window
    getmaxyx(stdscr,center[0],center[1]);

    center[0] = center[0]/2;
    center[1] = center[1]/2;

    lucorner[0] = center[0] - (HEIGHT/2);
    lucorner[1] = center[1] - WIDTH;
    
    memcpy(infos,
           "++++++++++++++++++++++++++"
           "++                        "
           "++                        "
           "++  Name:                 "
           "++                        "
           "++  Level:                "
           "++  Length:               "
           "++                        "
           "++                        "
           "++                        "
           "++                        "
           "++                        "
           "++                        "
           "++                        "
           "++                        "
           "++                        "
           "++                        "
           "++                        "
           "++                        "
           "++++++++++++++++++++++++++",
           HEIGHT*WIDTH*sizeof(char));
           
    memcpy(arena,
           "++++++++++++++++++++++++++"
           "+                        +"
           "+                        +"
           "+                        +"
           "+                        +"
           "+                        +"
           "+                        +"
           "+                        +"
           "+                        +"
           "+                        +"
           "+                        +"
           "+                        +"
           "+                        +"
           "+                        +"
           "+                        +"
           "+                        +"
           "+                        +"
           "+                        +"
           "+                        +"
           "++++++++++++++++++++++++++",
           HEIGHT*WIDTH*sizeof(char));


    head[0] = HEIGHT/2; 
    head[1] = WIDTH/2;
    tail[0] = HEIGHT/2; 
    tail[1] = WIDTH/2-1;
 
    arena[head[0]][head[1]] = 'o';
    arena[tail[0]][tail[1]] = 'o';

}

void
place_food()
{

    food[0] = (rand() % (HEIGHT-2)) + 1;
    food[1] = (rand() % (WIDTH -2)) + 1;

    if ( arena[food[0]][food[1]] != ' ' )
    {
        place_food();
    }else
    {
        arena[food[0]][food[1]  ] = 'f';
    }
    
}

void
game_over()
{
    dead = 1;
}

void
update_scr()
{
    clear();
    //infos
    for (int i = 0; i < HEIGHT; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            if (infos[i][j] == '+')
            {       
                attron(COLOR_PAIR(BORDER_PAIR));
                mvprintw(lucorner[0]+i, lucorner[1]-WIDTH+j, "%c", infos[i][j]);
                attroff(COLOR_PAIR(BORDER_PAIR));
            }else if (infos[i][j] != ' ')
            { 
                attron(COLOR_PAIR(SCRIPT_PAIR));
                mvprintw(lucorner[0]+i, lucorner[1]-WIDTH+j, "%c", infos[i][j]);
                attroff(COLOR_PAIR(SCRIPT_PAIR));
            }         
        }
    }
    //arena
    for (int i = 0; i < HEIGHT; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {  
            if (arena[i][j] == '+')
            {
                attron(COLOR_PAIR(BORDER_PAIR));
                mvprintw(lucorner[0]+i, lucorner[1]+2*j, "%c%c", arena[i][j], arena[i][j]);
                attroff(COLOR_PAIR(BORDER_PAIR));
            }else if ( arena[i][j] == 'n' || 
                       arena[i][j] == 'o' ||
                       arena[i][j] == 's' ||
                       arena[i][j] == 'w' )
            {
                attron(COLOR_PAIR(SNAKE_PAIR));
                mvprintw(lucorner[0]+i, lucorner[1]+2*j, "%c%c", arena[i][j], arena[i][j]);
                attroff(COLOR_PAIR(SNAKE_PAIR));
            }else if ( arena[i][j] == 'f' ){
                attron(COLOR_PAIR(FOOD_PAIR));
                mvprintw(lucorner[0]+i, lucorner[1]+2*j, "%c%c", arena[i][j], arena[i][j]);
                attroff(COLOR_PAIR(FOOD_PAIR));
            }          
        }
    }
    refresh();
}

void
move_head()
{
    //current head direction
    tmp_c = arena[head[0]][head[1]];
    switch (tmp_c)
    {
    case 'n':
        if (arena[head[0]-1][head[1]] == 'f' )
        {
            digestion.push_front(length);
            length+=1;
            place_food();
            if (length % 3 == 0) {level+=1;}
        }else if (arena[head[0]-1][head[1]] != ' ')
        {
            game_over();
            return;
        }
        head[0]-=1;
        break;
    case 'o':
        if (arena[head[0]][head[1]+1] == 'f')
        {
            digestion.push_front(length);
            length+=1;
            place_food();
            if (length % 3 == 0) {level+=1;}
        }else if (arena[head[0]][head[1]+1] != ' ')
        {
            game_over();
            return;
        }
        head[1]+=1;
        break;
    case 's':
        if (arena[head[0]+1][head[1]] == 'f')
        {
            digestion.push_front(length);
            length+=1;
            place_food();
            if (level % 3 == 0) {level+=1;}
        }else if (arena[head[0]+1][head[1]] != ' ')
        {
            game_over();
            return;
        }
        head[0]+=1;
        break;
    case 'w':
        if (arena[head[0]][head[1]-1] == 'f')
        {
            digestion.push_front(length);
            length+=1;
            place_food();
            if (length % 3 == 0) {level+=1;}
        }else if (arena[head[0]][head[1]-1] != ' ')
        {
            game_over();
            return;
        }
        head[1]-=1;
        break;
    default:
        break;
    }
    
    arena[head[0]][head[1]] = tmp_c;

}

void
move_tail()
{
    memcpy(tmp_xy, tail, 2*sizeof(int));  
    switch (arena[tail[0]][tail[1]])
    {
    case 'n':
        tail[0]-=1;
        break;
    case 'o':
        tail[1]+=1;
        break;
    case 's':
        tail[0]+=1;
        break;
    case 'w':
        tail[1]-=1;
    default:
        break;
    }
    arena[tmp_xy[0]][tmp_xy[1]] = ' ';
}

void
update_head(char ch)
{
    tmp_c = arena[head[0]][head[1]];
    switch (ch)
    {
    case UP:
        if ( tmp_c != 's' )
        {
            arena[head[0]][head[1]]   = 'n';
        } 
        break;
    case RIGHT:
        if ( tmp_c != 'w' )
        {
            arena[head[0]][head[1]]   = 'o';
        }
        break;
    case DOWN:
        if ( tmp_c != 'n' )
        {
            arena[head[0]][head[1]]   = 's';
        }
        break;
    case LEFT:
        if ( tmp_c != 'o' )
        {
            arena[head[0]][head[1]]   = 'w';
        }
        break;
    default:
        break;
    } 
}

void
game()
{
    srand(time(NULL));
    init_arena();
    place_food();
    update_scr();
    gettimeofday(&t1, NULL);
    while(!dead)
    { 

        nodelay(stdscr, true);
        update_head(getch());

        gettimeofday(&t2, NULL);
        if( (((t2.tv_sec-t1.tv_sec) * 1000) + ((t2.tv_usec-t1.tv_usec)/1000)) > MOVEINTERVAL)
        {
            if (!digestion.empty())
            {
                std::list<int>::iterator it;
                for (it = digestion.begin(); it != digestion.end(); it++)
                {
                    *it -= 1;
                }
 
                if (digestion.back() == 0)
                {
                    digestion.pop_back();
                    //grow_tail();
                }else
                {
                    move_tail();
                }
            }else
            {
                move_tail();
            }           
            move_head();
            gettimeofday(&t1, NULL);
            update_scr();
        }

    }

}

int main(int argc, char const *argv[])
{

    initscr();
    start_color();
    curs_set(0); // hide cursor 

    init_pair(SCRIPT_PAIR, COLOR_RED, COLOR_BLACK);
    init_pair(SNAKE_PAIR, COLOR_MAGENTA, COLOR_MAGENTA);
    init_pair(BORDER_PAIR, COLOR_BLUE, COLOR_BLUE);
    init_pair(FOOD_PAIR, COLOR_GREEN, COLOR_GREEN);
    //init_pair(4, COLOR_YELLOW, COLOR_BLACK);

    game();

    endwin();

    return 0;
}