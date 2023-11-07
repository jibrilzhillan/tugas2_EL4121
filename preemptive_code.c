#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define LED1R GPIO_NUM_5
#define LED1B GPIO_NUM_18
#define LED2R GPIO_NUM_19
#define LED2B GPIO_NUM_21

#define GPIO_OUTPUT_PIN_SEL                                                    \
  ((1ULL << LED1R) | (1ULL << LED1B) | (1ULL << LED2R) | (1ULL << LED2B))

TaskHandle_t task1Handle, task2Handle;

static const char *TAG = "RateMonotonic";

void task1(void *pvParameters) {
  bool LED = true;
  printf("Task1 running on core: ");
  int core = xPortGetCoreID();
  printf("%d", core);
  printf("\n");

  gpio_set_level(LED1R, 1);
  printf("Mulai task 1\n");
  int64_t t1 = esp_timer_get_time();
  for (int i = 0; i <= 6000000; i++) {
    if ((i % 300000) == 0) {
      gpio_set_level(LED1B, LED);
      LED = !LED;
    }
  }

  printf("Selesai task 1\n");
  gpio_set_level(LED1B, 0);
  gpio_set_level(LED1R, 0);
  int64_t t2 = esp_timer_get_time();
  ESP_LOGI(TAG, "Task 1 duration: %lld us\n", (t2 - t1));

  vTaskDelete(NULL);
}

void task2(void *pvParameters) {
  bool LED = true;
  printf("Task2 running on core: ");
  int core = xPortGetCoreID();
  printf("%d", core);
  printf("\n");

  gpio_set_level(LED2R, 1);
  printf("Mulai task 2\n");
  int64_t t1 = esp_timer_get_time();
  for (int i = 0; i <= 14500000; i++) {
    if (i % 100000 == 0) {
      gpio_set_level(LED2B, LED);
      LED = !LED;
    }
  }

  printf("Selesai task 2\n");
  gpio_set_level(LED2B, 0);
  gpio_set_level(LED2R, 0);
  int64_t t2 = esp_timer_get_time();
  ESP_LOGI(TAG, "Task 2 duration: %lld us\n", (t2 - t1));

  vTaskDelete(NULL);
}

void IRAM_ATTR onTimerTask1() {
  xTaskCreate(task1,       /* Task function. */
              "Task1",     /* name of task. */
              10000,       /* Stack size of task */
              NULL,        /* parameter of the task */
              1,           /* priority of the task */
              &task1Handle /* Task handle to keep track of created task*/
  );
}

void IRAM_ATTR onTimerTask2() {
  xTaskCreate(task2,       /* Task function. */
              "Task2",     /* name of task. */
              10000,       /* Stack size of task */
              NULL,        /* parameter of the task */
              0,           /* priority of the task */
              &task2Handle /* Task handle to keep track of created task */
  );
}

void app_main(void) {
  // Initialize LED output
  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
  io_conf.mode = GPIO_MODE_OUTPUT;
  gpio_config(&io_conf);

  /* Create two timers:
   * 1. Periodic timer which will run every 4s and blink LED
   * 2. Periodic timer which will run every 10s and blnk
   */

  const esp_timer_create_args_t periodic1_timer_args = {
      .callback = &onTimerTask1,
      /* name is optional, but may help identify the timer when debugging */
      .name = "periodic1"};

  esp_timer_handle_t periodic1_timer;
  ESP_ERROR_CHECK(esp_timer_create(&periodic1_timer_args, &periodic1_timer));
  /* The timer has been created but is not running yet */

  const esp_timer_create_args_t periodic2_timer_args = {
      .callback = &onTimerTask2,
      /* name is optional, but may help identify the timer when debugging */
      .name = "periodic2"};

  esp_timer_handle_t periodic2_timer;
  ESP_ERROR_CHECK(esp_timer_create(&periodic2_timer_args, &periodic2_timer));
  /* The timer has been created but is not running yet */

  /* Start timer */
  ESP_ERROR_CHECK(esp_timer_start_once(periodic1_timer, 0));
  ESP_ERROR_CHECK(esp_timer_start_once(periodic2_timer, 0));
  ESP_ERROR_CHECK(esp_timer_start_periodic(periodic2_timer, 10000000));
  ESP_ERROR_CHECK(esp_timer_start_periodic(periodic1_timer, 4000000));
}
