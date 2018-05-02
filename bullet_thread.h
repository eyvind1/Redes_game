#ifndef BULLET_THREAD_H
#define BULLET_THREAD_H
#include <thread>
#include <ncurses.h>
#include "game_utils.h"
#include <unordered_map>
#define DELAY 120000

void init_bullet_params(WIN *p_win, int x, int y){
    p_win->height = 1;
    p_win->width = 2;
    p_win->starty = y;
    p_win->startx = x;

    p_win->border.ls = '|';
    p_win->border.rs = '|';
    p_win->border.ts = '-';
    p_win->border.bs = '-';
    p_win->border.tl = '+';
    p_win->border.tr = '+';
    p_win->border.bl = '+';
    p_win->border.br = '+';
}

class ThreadBullet{
public:
    void bullet_in_position(int x,int y){
        WIN bullet;
        init_bullet_params(&bullet,x,y);
        create_box(&bullet, TRUE);
        refresh();
        int i=y;
        while(i>=0){
            usleep(DELAY);
            create_box(&bullet, FALSE);
            --bullet.starty;
            create_box(&bullet, TRUE);
            refresh();
            --i;
        }
        create_box(&bullet, FALSE);
    }

    void start_thread(int thread_id,int x,int y){

        std::thread thrd = std::thread(&ThreadBullet::bullet_in_position,this,x,y);
        thrd.detach();
        tm_[thread_id] = std::move(thrd);
    }

    void stop_thread(int thread_id){
        ThreadMap::const_iterator it = tm_.find(thread_id);
        if (it != tm_.end()) {
            it->second.std::thread::~thread(); // thread not killed
            tm_.erase(thread_id);
        }
    }

private:
    typedef std::unordered_map<int, std::thread> ThreadMap;
    ThreadMap tm_;
};
#endif // BULLET_THREAD_H
