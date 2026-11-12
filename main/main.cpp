/*
 * -----------------------------------------------------------------------------
 * Project       : esp32-snake-game
 * File          : main.cpp
 * Author        : nktsb
 * Created on    : 11.07.2024
 * GitHub        : https://github.com/nktsb/esp32-snake-game
 * -----------------------------------------------------------------------------
 * Copyright (c) 2024 nktsb
 * All rights reserved.
 * -----------------------------------------------------------------------------
 */

#include "buttons.hpp"
#include "led_matrix.hpp"
#include "snake.hpp"

namespace {

    namespace config {
        constexpr int snake_step_period_ms = 250;
        constexpr int field_width = 8;
        constexpr int field_height = 8;
        constexpr int loop_period_ms = 10;

        constexpr spi_host_device_t spi_host = HSPI_HOST;
        constexpr gpio_num_t spi_sck_pin = GPIO_NUM_14;
        constexpr gpio_num_t spi_miso_pin = GPIO_NUM_12;
        constexpr gpio_num_t spi_mosi_pin = GPIO_NUM_13;
        constexpr gpio_num_t spi_cs_pin = GPIO_NUM_15;
        constexpr int spi_frequency_hz = 1000000;
        constexpr int matrix_brightness = 0x0F;

        constexpr gpio_num_t left_button_pin = GPIO_NUM_27;
        constexpr gpio_num_t right_button_pin = GPIO_NUM_26;
        constexpr gpio_num_t down_button_pin = GPIO_NUM_25;
        constexpr gpio_num_t up_button_pin = GPIO_NUM_10;

        constexpr int button_queue_length = 16;
        constexpr int matrix_task_stack_bytes = 4 * 1024;
        constexpr int matrix_task_priority = 10;
    } // namespace config

    QueueHandle_t buttons_queue = nullptr;

    led_matrix matrix(config::spi_host, config::spi_sck_pin, config::spi_miso_pin,
                      config::spi_mosi_pin, config::spi_cs_pin,
                      config::spi_frequency_hz, config::matrix_brightness);

    // Adapt the game's callbacks to the display driver and FreeRTOS clock.
    int current_time_ms()
    {
        return pdTICKS_TO_MS(xTaskGetTickCount());
    }

    void add_point(int x, int y)
    {
        matrix.Add_Dot(static_cast<uint8_t>(x), static_cast<uint8_t>(y));
    }

    void remove_point(int x, int y)
    {
        matrix.Remove_Dot(static_cast<uint8_t>(x), static_cast<uint8_t>(y));
    }

    void delay_ms(int time_ms)
    {
        vTaskDelay(pdMS_TO_TICKS(time_ms));
    }

    // Each button uses the same ISR logic with its direction fixed at compile time.
    // Game state is only changed by the main loop after receiving the event.
    template <direction_t Direction>
    void IRAM_ATTR on_button_pressed(void *)
    {
        direction_t direction = Direction;
        BaseType_t higher_priority_task_woken = pdFALSE;
        xQueueSendFromISR(buttons_queue, &direction, &higher_priority_task_woken);
    }

    void matrix_task(void *)
    {
        matrix.Setting();

        for (;;) {
            matrix.Update_Rows_Task();
            delay_ms(config::loop_period_ms);
        }
    }

} // namespace

extern "C" void app_main(void)
{
    buttons_queue = xQueueCreate(config::button_queue_length, sizeof(direction_t));
    configASSERT(buttons_queue != nullptr);

    // Start the display task before the game runs its startup animation.
    const BaseType_t task_created = xTaskCreate(
        matrix_task, "led_matrix_task", config::matrix_task_stack_bytes,
        nullptr, config::matrix_task_priority, nullptr);
    configASSERT(task_created == pdPASS);

    snake_game snake(config::snake_step_period_ms,
                     add_point, remove_point, current_time_ms, delay_ms,
                     config::field_width, config::field_height);

    ESP_ERROR_CHECK(gpio_install_isr_service(0));

    // Keep the button objects alive for as long as their ISRs are registered.
    button left_button(on_button_pressed<LEFT>, nullptr, config::left_button_pin);
    button right_button(on_button_pressed<RIGHT>, nullptr, config::right_button_pin);
    button down_button(on_button_pressed<DOWN>, nullptr, config::down_button_pin);
    button up_button(on_button_pressed<UP>, nullptr, config::up_button_pin);

    direction_t direction = UP;

    for (;;) {
        snake.Snake_Task();

        if (xQueueReceive(buttons_queue, &direction, 0) == pdPASS) {
            snake.Set_Direction(direction);
        }

        delay_ms(config::loop_period_ms);
    }
}
