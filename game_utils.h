#ifndef GAME_UTILS_H
#define GAME_UTILS_H

#include <ncurses.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>


typedef struct _win_border_struct {
    chtype 	ls, rs, ts, bs,
        tl, tr, bl, br;
}WIN_BORDER;

typedef struct _WIN_struct {

    int startx, starty;
    int height, width;
    WIN_BORDER border;
}WIN;


void init_win_params(WIN *p_win)
{
    srand (time(NULL));
    // p_win->height = rand()%10 + 3;
    // p_win->width = rand()%10 + 3;
    p_win->height = 3;
    p_win->width = 10;
//    p_win->starty = (LINES - p_win->height)/2;
//    p_win->startx = (COLS - p_win->width)/2;

    p_win->border.ls = '|';
    p_win->border.rs = '|';
    p_win->border.ts = '-';
    p_win->border.bs = '-';
    p_win->border.tl = '+';
    p_win->border.tr = '+';
    p_win->border.bl = '+';
    p_win->border.br = '+';

}






void print_win_params(WIN *p_win)
{
#ifdef _DEBUG
    mvprintw(25, 0, "%d %d %d %d", p_win->startx, p_win->starty,
                p_win->width, p_win->height);
    refresh();
#endif
}
void create_box(WIN *p_win, bool flag)
{	int i, j;
    int x, y, w, h;

    x = p_win->startx;
    y = p_win->starty;
    w = p_win->width;
    h = p_win->height;

    if(flag == TRUE)
    {	mvaddch(y, x, p_win->border.tl);
        mvaddch(y, x + w, p_win->border.tr);
        mvaddch(y + h, x, p_win->border.bl);
        mvaddch(y + h, x + w, p_win->border.br);
        mvhline(y, x + 1, p_win->border.ts, w - 1);
        mvhline(y + h, x + 1, p_win->border.bs, w - 1);
        mvvline(y + 1, x, p_win->border.ls, h - 1);
        mvvline(y + 1, x + w, p_win->border.rs, h - 1);

    }
    else
        for(j = y; j <= y + h; ++j)
            for(i = x; i <= x + w; ++i)
                mvaddch(j, i, ' ');

    refresh();

}

//void make_bullet_from_parent(WIN *parent_win, WIN *enemie){
//    int max_y = 0, max_x = 0;
//    getmaxyx(stdscr, max_y, max_x);
//    WIN bullet;
//    init_bullet_params(&bullet, parent_win->startx, parent_win->starty);
//    create_box(&bullet, TRUE);
//    refresh();
//    int i=parent_win->starty;
//    while(i>=0){
//        usleep(DELAY);
//        create_box(&bullet, FALSE);
//        if(bullet.startx >= enemie->startx && bullet.startx <= enemie->startx+enemie->width && bullet.starty == enemie->starty){
//            create_box(enemie, FALSE);
//            create_box(&bullet, FALSE);
//            return;
//        }
//        --bullet.starty;
//        create_box(&bullet, TRUE);
//        refresh();
//        --i;
//    }
//    create_box(&bullet, FALSE);
//}

//void bullet_in_position(int x,int y){
//    WIN bullet;
//    init_bullet_params(&bullet,x,y);
//    create_box(&bullet, TRUE);
//    refresh();
//    int i=y;
//    while(i>=0){
//        usleep(DELAY);
//        create_box(&bullet, FALSE);
//        --bullet.starty;
//        create_box(&bullet, TRUE);
//        refresh();
//        --i;
//    }
//    create_box(&bullet, FALSE);
//}

//void make_bullet_from_enemy(WIN *parent_win, WIN *enemie){
//    int max_y = 0, max_x = 0;
//    getmaxyx(stdscr, max_y, max_x);
//    WIN bullet;
//    init_bullet_params(&bullet, parent_win->startx, parent_win->starty);
//    create_box(&bullet, TRUE);
//    refresh();
//    int i=parent_win->starty;
//    while(i>=0){
//        usleep(DELAY);
//        create_box(&bullet, FALSE);
//        if(bullet.startx >= enemie->startx && bullet.startx <= enemie->startx+enemie->width && bullet.starty == enemie->starty){
//            endwin();
//            return;
//        }
//        --bullet.starty;
//        create_box(&bullet, TRUE);
//        refresh();
//        --i;
//    }
//    create_box(&bullet, FALSE);

//}

#endif // GAME_UTILS_H
