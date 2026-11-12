/*
 * -----------------------------------------------------------------------------
 * Project       : esp32-snake-game
 * File          : snake.hpp
 * Author        : nktsb
 * Created on    : 11.07.2024
 * GitHub        : https://github.com/nktsb/esp32-snake-game
 * -----------------------------------------------------------------------------
 * Copyright (c) 2024 nktsb
 * All rights reserved.
 * -----------------------------------------------------------------------------
 */

#ifndef __SNAKE_HPP_
#define __SNAKE_HPP_

#include "main.hpp"
#include "led_matrix.hpp"

typedef enum {
    UP = 0,
    DOWN,
    LEFT,
    RIGHT,

    DIRECTIONS_NUM
} direction_t;

typedef struct point {
    bool operator==(const point& other) const {
        return (bool)((x == other.x) && (y == other.y));
    };
    int x;
    int y;
} point_st;

class snake_game
{
public:
    snake_game(
            int snake_step_period,
            void (*Add_Point_Func)(int, int),
            void (*Remove_Point_Func)(int, int),
            int (*Get_Time_Func)(),
            void (*Delay_Func)(int),
            int field_width,
            int field_height
    );
    ~snake_game(void);

    void Start_Game();
    void Stop_Game();

    void Set_Direction(direction_t direction);
    void Snake_Task();

private:

    void Snake_Body_Move();
    point_st Snake_Calc_New_Point();
    void Snake_Body_Attach(point_st new_point);
    void Snake_Body_Detach();

    void Spawn_Rand_Food_Piece();

    bool Check_Is_Point_Food_Piece(point_st point);
    bool Check_Is_Point_Snake_part(const point_st point);

    void Start_Animation();

    int snake_step_period;
    const int field_width;
    const int field_height;

    point_st food_piece;

    std::deque<point_st> snake_body;
    direction_t direction;

    void (*Add_Point)(int, int);
    void (*Remove_Point)(int, int);
    int (*Get_Current_Time_Ms)();
    void (*Delay)(int);
};

#endif /* __SNAKE_HPP_ */