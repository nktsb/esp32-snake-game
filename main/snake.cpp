/*
 * -----------------------------------------------------------------------------
 * Project       : esp32-snake-game
 * File          : snake.cpp
 * Author        : nktsb
 * Created on    : 11.07.2024
 * GitHub        : https://github.com/nktsb/esp32-snake-game
 * -----------------------------------------------------------------------------
 * Copyright (c) 2024 nktsb
 * All rights reserved.
 * -----------------------------------------------------------------------------
 */

#include "snake.hpp"

static const int DEFAULT_FIELD_WIDTH = 8;
static const int DEFAULT_FIELD_HEIGHT = 8;

snake_game::snake_game(
        int snake_step_period,
        void (*Add_Point_Func)(int, int),
        void (*Remove_Point_Func)(int, int),
        int (*Get_Time_Func)(),
        void (*Delay_Func)(int),
        int field_width = DEFAULT_FIELD_WIDTH,
        int field_height = DEFAULT_FIELD_HEIGHT
        ) :
        snake_step_period(snake_step_period),
        field_width(field_width),
        field_height(field_height),
        direction(UP),
        Add_Point(Add_Point_Func),
        Remove_Point(Remove_Point_Func),
        Get_Current_Time_Ms(Get_Time_Func),
        Delay(Delay_Func)
{
    this->Start_Game();
}

snake_game::~snake_game(void)
{
    this->Stop_Game();
}

void snake_game::Start_Animation()
{
    bool matrix[this->field_width][this->field_height];
    memset((void*)matrix, 0, sizeof(matrix));
    int filled = 0;

    while(filled < (this->field_width * this->field_height))
    {
        int x = rand() % this->field_width;
        int y = rand() % this->field_height;
        if (!matrix[x][y])
        {
            this->Add_Point(x, y);
            matrix[x][y] = true;
            filled++;
            this->Delay(10);
        }
    }

    this->Delay(500);

    while(filled > 0)
    {
        int x = rand() % this->field_width;
        int y = rand() % this->field_height;
        if (matrix[x][y])
        {
            this->Remove_Point(x, y);
            matrix[x][y] = false;
            filled--;
            this->Delay(10);
        }
    }
}

void snake_game::Start_Game()
{
    srand(this->Get_Current_Time_Ms());

    this->Start_Animation();

    this->Snake_Body_Attach({(this->field_width / 2), (this->field_height / 2)});
    this->Snake_Body_Attach(this->Snake_Calc_New_Point());

    this->Spawn_Rand_Food_Piece();
}

void snake_game::Stop_Game()
{
    this->snake_body.clear();
    this->Start_Game();
}

void snake_game::Set_Direction(direction_t direction)
{
    const std::map<direction_t, direction_t> opposite_directions = {
        {RIGHT, LEFT},
        {LEFT, RIGHT},
        {DOWN, UP},
        {UP, DOWN}
    };

    if (direction != opposite_directions.at(this->direction))
        this->direction = direction;
}

point_st snake_game::Snake_Calc_New_Point()
{
    point_st new_point = *this->snake_body.begin();

    switch(this->direction) {
        case UP:
            new_point.y += 1;
            break;
        case DOWN:
            new_point.y -= 1;
            break;
        case LEFT:
            new_point.x -= 1;
            break;
        case RIGHT:
            new_point.x += 1;
            break;
        default:
            break;
    };
    
    new_point.x = (new_point.x + this->field_width) % this->field_width;
    new_point.y = (new_point.y + this->field_height) % this->field_height;
    return new_point;
}

void snake_game::Snake_Body_Attach(point_st new_point)
{
    this->snake_body.push_front(new_point);
    this->Add_Point(new_point.x, new_point.y);
}

void snake_game::Snake_Body_Detach()
{
    point_st last_point = this->snake_body.back();
    this->snake_body.pop_back();
    this->Remove_Point(last_point.x, last_point.y);
}

bool snake_game::Check_Is_Point_Food_Piece(point_st point)
{
    return (bool)(point == this->food_piece);
}

bool snake_game::Check_Is_Point_Snake_part(point_st point)
{
    auto it = std::find(this->snake_body.begin(), this->snake_body.end(), point);
    return (bool)(it != this->snake_body.end());
}

void snake_game::Spawn_Rand_Food_Piece()
{
    std::vector<point_st> free_points;
    for (int x = 0; x < this->field_width; ++x)
    {
        for (int y = 0; y < this->field_height; ++y)
        {
            point_st point = {x, y};
            if (!this->Check_Is_Point_Snake_part(point))
                free_points.push_back(point);
        }
    }

    if (!free_points.empty())
    {
        int rand_idx = rand() % free_points.size();
        this->food_piece = free_points[rand_idx];
        this->Add_Point(this->food_piece.x, this->food_piece.y);
    }
}

void snake_game::Snake_Body_Move()
{
    static int last_step_time_ms = 0;
    static bool no_detach_next_step = false;

    int current_time_ms = this->Get_Current_Time_Ms();

    if(current_time_ms - last_step_time_ms >= this->snake_step_period)
    {
        point_st new_point = this->Snake_Calc_New_Point();

        if(!no_detach_next_step)
            this->Snake_Body_Detach();
        else
            no_detach_next_step = false;

        this->Delay(this->snake_step_period / 2);

        if(this->Check_Is_Point_Snake_part(new_point) == true)
            this->Stop_Game();
        else
            this->Snake_Body_Attach(new_point);
    

        if(this->Check_Is_Point_Food_Piece(new_point) == true)
        {
            no_detach_next_step = true;
            this->Spawn_Rand_Food_Piece();
        }

        last_step_time_ms = current_time_ms;
    }
}

void snake_game::Snake_Task(void)
{
    this->Snake_Body_Move();
}
