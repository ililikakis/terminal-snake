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
#include <iostream>

#include "settings.h"

/* size of arena, do NOT touch */
#define WIDTH 26
#define HEIGHT 20

/* Speed of Snake */
#define DELAY 7777
#define MOVEINTERVAL (DELAY/1000) * ( (300 - level*13) /5)

/* left upper corner of SnakeArena 
 * coordinate is related to terminal
 */
int  lucorner[2]; // global
char arena[HEIGHT][WIDTH];
char infos[WIDTH*2];

/* coordinates are related to lucorner[] */
int head[2];
int tail[2];
int food[2];

int  tmp_xy[2];  // buffer tail position
char tmp_c;      // buffer tail or head direction

int dead   = 0;
int level  = 5;
int length = 2;

/* List of food that passes through the snake. 
 * Value of the element, corresponding to position in
 * the List. Value is zero if position is equal to tail.
 */
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
    
    /* LevelIndex 32 
     * LengthIndex 46 */
    memcpy(infos,
           "Name:                    Level: 1     Length: 2     "
           , WIDTH*2*sizeof(char));
           
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
 
    arena[head[0]][head[1]] = 'e';
    arena[tail[0]][tail[1]] = 'e';

}

/* If the snake is long enough, the method could become 
 * problematic due to too many recursions.
 * Alternative idea is needed!!
 */
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
    // print information of player, level and length
    attron(COLOR_PAIR(SCRIPT_PAIR));
    for (int i = 0; i < WIDTH*2; i++)
    {
        mvprintw(lucorner[0]-2, lucorner[1]+i, "%c", infos[i]);
    }
    attroff(COLOR_PAIR(SCRIPT_PAIR));
    //print arena and stuff inside
    for (int i = 0; i < HEIGHT; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {  
            if (arena[i][j] == '+')
            {
                // Border
                attron(COLOR_PAIR(BORDER_PAIR));
                mvprintw(lucorner[0]+i, lucorner[1]+2*j, 
                         "%c%c", arena[i][j], arena[i][j]);
                attroff(COLOR_PAIR(BORDER_PAIR));
            }else if ( arena[i][j] == 'n' || 
                       arena[i][j] == 'e' ||
                       arena[i][j] == 's' ||
                       arena[i][j] == 'w' )
            {
                // Snake
                attron(COLOR_PAIR(1));
                mvprintw(lucorner[0]+i, lucorner[1]+2*j,
                         "%c%c", arena[i][j], arena[i][j]);
                attroff(COLOR_PAIR(1));
            }else if ( arena[i][j] == 'f' ){
                // Food
                attron(COLOR_PAIR(FOOD_PAIR));
                mvprintw(lucorner[0]+i, lucorner[1]+2*j,
                         "%c%c", arena[i][j], arena[i][j]);
                attroff(COLOR_PAIR(FOOD_PAIR));
            }          
        }
    }
    refresh();
}

/* update length and level */
void
update_info(int lev, int len)
{
    length += len;
    level += lev;
    int index;
    std::string l =  std::to_string(level);    
    index = 32;
    for (char i : l)
    {
        infos[index] = i;
        index += 1;
    }
    
    l = std::to_string(length);
    index = 46;
    for (char i : l)
    {
        infos[index] = i;
        index += 1;
    }
    
}

/* Move possition of head and write direction 
 * in corresponding field of arena.
 */
void
move_head()
{
    /* Store current head direction to 
     * write it into the coming field. */
    tmp_c = arena[head[0]][head[1]];
    /* If target field has food 'f', eat it and move afterwards.
     * If target field has neither food nor is empty, end game.
     * Else field has to be empty. */
    switch (tmp_c)
    {
    case 'n':
        if (arena[head[0]-1][head[1]] == 'f' )
        {
            digestion.push_front(length);
            place_food();
        }
        
        head[0]-=1;
        break;
    case 'e':
        if (arena[head[0]][head[1]+1] == 'f')
        {
            digestion.push_front(length);
            place_food();
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
            place_food();
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
            place_food();
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

/* Move possition of tail and clear
 * corresponding field of arena with ' '.
 */
void
move_tail()
{
    /* Store coordinate to clear coresponding field 
     * in arena after moving. */
    memcpy(tmp_xy, tail, 2*sizeof(int));

    switch (arena[tail[0]][tail[1]])
    {
    case 'n':
        tail[0]-=1;
        break;
    case 'e':
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
    /* clear field */
    arena[tmp_xy[0]][tmp_xy[1]] = ' ';
}

/* Update the direction of the head 
 * according to the input.
 */
void
update_head(char ch)
{
    /* ch       desired direction
     * tmp_c    current direction
     */
    tmp_c = arena[head[0]][head[1]];
    /* if current direction is opposite of desired, do nothing */
    switch (ch)
    {
    case UP:
        if ( tmp_c != 's' )
        {
            arena[head[0]][head[1]] = 'n';
        } 
        break;
    case RIGHT:
        if ( tmp_c != 'w' )
        {
            arena[head[0]][head[1]] = 'e';
        }
        break;
    case DOWN:
        if ( tmp_c != 'n' )
        {
            arena[head[0]][head[1]] = 's';
        }
        break;
    case LEFT:
        if ( tmp_c != 'e' )
        {
            arena[head[0]][head[1]] = 'w';
        }
        break;
    default:
        break;
    } 
}

void
game()
{
    char ch;
    srand(time(NULL));
    init_arena();
    place_food();
    update_scr();
    cbreak();
    nodelay(stdscr, TRUE);
    gettimeofday(&t1, NULL);
    while(!dead && !usleep(DELAY))
    {
        ch = getch();
        update_head(ch);
        //noecho();

        gettimeofday(&t2, NULL);
        if( (((t2.tv_sec-t1.tv_sec) * 1000) + ((t2.tv_usec-t1.tv_usec)/1000)) 
            > MOVEINTERVAL)
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
                    if (length % 3 == 0) {update_info(1,1); }
                    else { update_info(0,1); }
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

    /*
     * COLOR_BLACK
     * COLOR_RED
     * COLOR_GREEN
     * COLOR_YELLOW
     * COLOR_BLUE
     * COLOR_MAGENTA
     * COLOR_CYAN
     * COLOR_WHITE
     */
    init_pair(SCRIPT_PAIR, COLOR_RED, COLOR_BLACK);
    init_pair(SNAKE_PAIR, COLOR_MAGENTA, COLOR_MAGENTA);
    init_pair(BORDER_PAIR, COLOR_BLUE, COLOR_BLUE);
    init_pair(FOOD_PAIR, COLOR_GREEN, COLOR_GREEN);
    //init_pair(4, COLOR_YELLOW, COLOR_BLACK);

    game();

    endwin();

    return 0;
}