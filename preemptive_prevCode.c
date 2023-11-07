#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_strip.h"
#include "sdkconfig.h"
#include <stdio.h>

#define LED1R GPIO_NUM_5
#define LED1B GPIO_NUM_18
#define LED2R GPIO_NUM_19
#define LED2B GPIO_NUM_21

#define GPIO_OUTPUT_PIN_SEL                                                    \
  ((1ULL << LED1R) | (1ULL << LED1B) | (1ULL << LED2R) | (1ULL << LED2B))

static const char *TAG = "RateMonotonic";

TaskHandle_t task1Handle, task2Handle;

void task1(void *pvParameters) {
  // ESP_LOGI(TAG, "TUGAS 1 MULAI");
  bool statusLED = 1; // on for the first time
  static uint8_t duration =
      20; // 1 second run time. 20 iteration cause blink period is 50ms
  TickType_t xLastWakeTime;
  TickType_t periodTask = 4000 / portTICK_PERIOD_MS; // Repeat every 4seconds
  while (1) {
    // initialize the xLastWakeTime variable with the current time
    xLastWakeTime = xTaskGetTickCount();

    for (size_t i = 0; i < duration; i++) {
      gpio_set_level(LED1R, 1);
      gpio_set_level(LED1B, statusLED);
      statusLED = !statusLED;
      vTaskDelay(50 / portTICK_PERIOD_MS);
    }

    gpio_set_level(LED1R, 0);
    gpio_set_level(LED1B, 0);

    vTaskDelayUntil(&xLastWakeTime, periodTask); // Period time
  }
}

void task2(void *pvParameters) {
  bool statusLED = 1; // on for the first time
  static uint16_t duration2 =
      1000; // 5 second runtime. 1000 iteration cause blink period is 50ms
  TickType_t xLastWakeTime;
  TickType_t periodTask = 10000 / portTICK_PERIOD_MS; // Repeat every 10 seconds
  while (1) {
    // initialize the xLastWakeTime variable with the current time
    xLastWakeTime = xTaskGetTickCount();

    for (size_t i = 0; i < duration2; i++) {
      gpio_set_level(LED2R, 1);
      gpio_set_level(LED2B, statusLED);
      statusLED = !statusLED;
      vTaskDelay(50 / portTICK_PERIOD_MS);
    }

    gpio_set_level(LED2R, 0);
    gpio_set_level(LED2B, 0);

    vTaskDelayUntil(&xLastWakeTime, periodTask); // Period time
  }
}

void app_main(void) {
  // Initialize LED output
  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
  io_conf.mode = GPIO_MODE_OUTPUT;
  gpio_config(&io_conf);

  // Create task and pinned it to core 1
  xTaskCreate(task1, "Task1", 1024, NULL, 0, &task1Handle);
  xTaskCreate(task2, "Task2", 1024, NULL, 1, &task2Handle);

  // while (1) {
  // }
}
