/*
 * -----------------------------------------------------------------------------
 * Project       : esp32-snake-game
 * File          : buttons.cpp
 * Author        : nktsb
 * Created on    : 11.07.2024
 * GitHub        : https://github.com/nktsb/esp32-snake-game
 * -----------------------------------------------------------------------------
 * Copyright (c) 2024 nktsb
 * All rights reserved.
 * -----------------------------------------------------------------------------
 */

#include "buttons.hpp"

button::button(void (*Handler)(void*), void * handler_arg, gpio_num_t pin): pin(pin)
{
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1 << pin);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    ESP_ERROR_CHECK(gpio_isr_handler_add(pin, Handler, handler_arg));
}

button::~button()
{
    gpio_isr_handler_remove(this->pin);
}