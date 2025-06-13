#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"

/* ---------- FreeRTOS ---------- */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* ---------- SSD1306 ---------- */
#include "include/ssd1306.h"
#include "include/ssd1306_i2c.h"
#include "include/ssd1306_font.h"

/* ---------- Pinos ---------- */
#define LED_R_PIN   11
#define LED_G_PIN   12
#define LED_B_PIN   13
#define BUZZER_PIN  21
#define BTN_A_PIN    5
#define BTN_B_PIN    6
#define I2C_SDA_PIN 14
#define I2C_SCL_PIN 15

/* ---------- Semáforos ---------- */
static SemaphoreHandle_t sem_led;
static SemaphoreHandle_t sem_buz;

/* ---------- Estado global p/ OLED ---------- */
typedef struct {
    volatile bool led_run;
    volatile bool buz_run;
} state_t;
static state_t gstate = { true, true };

/* ---------- OLED helpers ---------- */
static ssd1306_t disp;

static void oled_init(void)
{
    i2c_init(i2c1, 400000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    disp.external_vcc = false;
    ssd1306_init(&disp, 128, 64, 0x3C, i2c1);
}

static void oled_show_state(void)
{
    char line[24];
    ssd1306_clear(&disp);

    sprintf(line, "LED : %s", gstate.led_run ? "RUN" : "PAUSE");
    ssd1306_draw_string(&disp, 0, 22, 2, line);

    sprintf(line, "BUZ : %s", gstate.buz_run ? "RUN" : "PAUSE");
    ssd1306_draw_string(&disp, 0, 40, 2, line);

    ssd1306_show(&disp);
}

/* ---------- Tarefa LED ---------- */
static void task_led(void *arg)
{
    const uint8_t seq[3] = { LED_R_PIN, LED_G_PIN, LED_B_PIN };
    size_t idx = 0;

    for (;;)
    {
        /* Bloqueia se semáforo estiver confiscado (pausa) */
        xSemaphoreTake(sem_led, portMAX_DELAY);
        xSemaphoreGive(sem_led);                  /* devolve já! */

        /* ciclo de cor */
        gpio_put(seq[idx], 1);
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_put(seq[idx], 0);
        idx = (idx + 1) % 3;
    }
}

/* ---------- Tarefa Buzzer ---------- */
static void task_buzzer(void *arg)
{
    for (;;)
    {
        xSemaphoreTake(sem_buz, portMAX_DELAY);
        xSemaphoreGive(sem_buz);

        gpio_put(BUZZER_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(100));
        gpio_put(BUZZER_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(900));
    }
}

/* ---------- Tarefa Botões ---------- */
static void task_buttons(void *arg)
{
    bool prevA = true, prevB = true;

    for (;;)
    {
        bool a = gpio_get(BTN_A_PIN);
        bool b = gpio_get(BTN_B_PIN);

        /* ----- LED ----- */
        if (prevA && !a) {                        /* borda de descida */
            if (gstate.led_run) {
                /* Pausar → tomar o token e NÃO devolver */
                xSemaphoreTake(sem_led, 0);
                gstate.led_run = false;
            } else {
                /* Retomar → devolver o token */
                xSemaphoreGive(sem_led);
                gstate.led_run = true;
            }
        }

        /* ----- Buzzer ----- */
        if (prevB && !b) {
            if (gstate.buz_run) {
                xSemaphoreTake(sem_buz, 0);
                gstate.buz_run = false;
            } else {
                xSemaphoreGive(sem_buz);
                gstate.buz_run = true;
            }
        }

        prevA = a;
        prevB = b;
        vTaskDelay(pdMS_TO_TICKS(50));            /* debounce 20 Hz */
    }
}

/* ---------- Tarefa OLED ---------- */
static void task_oled(void *arg)
{
    for (;;)
    {
        oled_show_state();
        vTaskDelay(pdMS_TO_TICKS(500));           /* 2 Hz */
    }
}

/* ---------- Inicialização de GPIO ---------- */
static void init_gpio(void)
{
    /* Saídas */
    const uint outs[] = { LED_R_PIN, LED_G_PIN, LED_B_PIN, BUZZER_PIN };
    for (uint i = 0; i < 4; i++) {
        gpio_init(outs[i]);
        gpio_set_dir(outs[i], GPIO_OUT);
        gpio_put(outs[i], 0);
    }

    /* Entradas (pull-up) */
    const uint ins[] = { BTN_A_PIN, BTN_B_PIN };
    for (uint i = 0; i < 2; i++) {
        gpio_init(ins[i]);
        gpio_set_dir(ins[i], GPIO_IN);
        gpio_pull_up(ins[i]);
    }
}

/* ---------- main ---------- */
int main(void)
{
    stdio_init_all();
    init_gpio();
    oled_init();
    oled_show_state();

    /* Semáforos binários (liberados = RUN) */
    sem_led = xSemaphoreCreateBinary();  xSemaphoreGive(sem_led);
    sem_buz = xSemaphoreCreateBinary();  xSemaphoreGive(sem_buz);

    /* --- Tarefas --- */
    xTaskCreate(task_led,     "LED",     256, NULL, 1, NULL);
    xTaskCreate(task_buzzer,  "BUZZER",  256, NULL, 1, NULL);
    xTaskCreate(task_buttons, "BUTTONS", 256, NULL, 3, NULL);
    xTaskCreate(task_oled,    "OLED",    512, NULL, 0, NULL);

    /* Inicia o agendador */
    vTaskStartScheduler();

    /* Nunca deve chegar aqui */
    while (true) { }
}
