/*
 * -----------------------------------------------------------------------------
 * Project       : esp32-snake-game
 * File          : buttons.hpp
 * Author        : nktsb
 * Created on    : 11.07.2024
 * GitHub        : https://github.com/nktsb/esp32-snake-game
 * -----------------------------------------------------------------------------
 * Copyright (c) 2024 nktsb
 * All rights reserved.
 * -----------------------------------------------------------------------------
 */

#ifndef __BUTTONS_HPP_
#define __BUTTONS_HPP_

#include "main.hpp"

class button {
public:
    button(void (*Handler)(void*), void * handler_arg, gpio_num_t pin);
    ~button();
private:
    gpio_num_t pin;
};

#endif /* __BUTTONS_HPP_ */