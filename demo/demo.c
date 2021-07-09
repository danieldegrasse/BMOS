/**
 * @file demo.c
 * Demonstrates the features of the RTOS and driver system
 * Produces output via LPUART1 (115200 baud, 8n1) and
 * SWO logging (at 2MHz, with cpu frequency of 80MHz)
 *
 * UART task will print to UART, wait for a button press via a semaphore pend,
 * then print again once the button is pressed.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <drivers/clock/clock.h>
#include <drivers/gpio/gpio.h>
#include <drivers/uart/uart.h>
#include <sys/semaphore/semaphore.h>
#include <sys/task/task.h>
#include <util/logging/logging.h>

semaphore_t buttonpress_sem;

// Function prototypes
static void system_init();
static void init_task(void *arg);
static void foreground_task(void *arg);
static void background_task(void *arg);
static void button_callback();

/* Static stack for background task */
static char background_task_stack[1024];

/**
 * Main entry point
 */
int main() {
    static const char *TAG = "main";
    task_config_t task_cfg = DEFAULT_TASK_CONFIG;
    // Initialize the system
    system_init();
    // Create the initialization task
    task_cfg.task_stacksize = 512;
    task_cfg.task_priority = DEFAULT_PRIORITY + 1;
    task_cfg.task_name = "Init Task";
    if (task_create(init_task, NULL, &task_cfg) == NULL) {
        LOG_E(TAG, "Failed to create initialization task");
        exit(ERR_FAIL);
    }
    LOG_I(TAG, "Starting RTOS");
    rtos_start();
    return SYS_OK;
}

/**
 * Initializes system, setting up clock at 80 MHz
 */
static void system_init() {
    // Default config uses PLL at 80 MHz, and MSI runs at 4MHz.
    // HCLK, PCLK1, and PCLK2 are also 80 MHz
    clock_cfg_t clk_cfg = CLOCK_DEFAULT_CONFIG;
    clock_init(&clk_cfg);
    // Set up GPIO pins A1 and A2 for UART.
    GPIO_config_t uart_config = GPIO_DEFAULT_CONFIG;
    uart_config.alternate_func = GPIO_af8;
    uart_config.output_speed = GPIO_speed_vhigh;
    uart_config.pullup_pulldown = GPIO_pullup;
    uart_config.mode = GPIO_mode_afunc;
    GPIO_config(GPIO_PA2, &uart_config);
    GPIO_config(GPIO_PA3, &uart_config);
    // Set up GPIO pin B13 (user LED) as output, and C13 (user button) as input
    GPIO_config_t gpio_conf = GPIO_DEFAULT_CONFIG;
    gpio_conf.mode = GPIO_mode_output;
    // Configure the user led
    GPIO_config(GPIO_PB13, &gpio_conf);
    gpio_conf.mode = GPIO_mode_input;
    // Configure the user button
    GPIO_config(GPIO_PC13, &gpio_conf);
}

/**
 * Initialization task. Creates the UART foreground task, as well as
 * a background task that logs to SWO (via ITM port 0 at 2MHz, with CPU
 * frequency of 80MHz). The background task will also flash the user LED while
 * active (although it will exit after it logs to SWO 30 times)
 * @param arg: Task argument. Unused.
 */
void init_task(void *arg) {
    task_config_t bg_task_config = DEFAULT_TASK_CONFIG;
    const char *TAG = "init_task";
    LOG_I(TAG, "Init task starting");
    // Create foreground task with default parameters
    if (task_create(foreground_task, NULL, NULL) == NULL) {
        LOG_E(TAG, "Could not create foreground task");
        exit(ERR_FAIL);
    }
    bg_task_config.task_name = "Bg_Task";
    bg_task_config.task_priority = DEFAULT_PRIORITY - 1;
    // Set up the background task with a statically allocated stack
    bg_task_config.task_stack = background_task_stack;
    bg_task_config.task_stacksize = sizeof(background_task_stack);
    if (task_create(background_task, NULL, &bg_task_config) == NULL) {
        LOG_E(TAG, "Could not create background task");
        exit(ERR_FAIL);
    }
    LOG_I(TAG, "Initialization task complete. Exiting...");
}

/**
 * Foreground task. Logs to LPUART1, and then waits for the GPIO button press
 * to post to a semaphore.
 * @param arg: Task argument. Unused.
 */
void foreground_task(void *arg) {
    int len, i;
    const char *TAG = "foreground_task";
    uint8_t buf[80];
    syserr_t err;
    UART_config_t uart_conf = UART_DEFAULT_CONFIG;
    UART_handle_t uart_dev;
    uart_conf.UART_echomode = UART_echo_dis;
    uart_conf.UART_textmode = UART_txtmode_en;
    uart_conf.UART_baud_rate = UART_baud_115200;
    uart_dev = UART_open(LPUART_1, &uart_conf, &err);
    if (!uart_dev) {
        LOG_E(TAG, "Could not open LPUART1");
        exit(err);
    }
    // Create button press semaphore
    buttonpress_sem = semaphore_create_counting(0);
    if (!buttonpress_sem) {
        LOG_E(TAG, "Could not create button press semaphore");
        exit(ERR_FAIL);
    }
    // Set up the GPIO interrupt callback for the user button
    err = GPIO_interrupt_enable(GPIO_PC13, GPIO_trig_rising, button_callback);
    if (err != SYS_OK) {
        LOG_E(TAG, "Could not install button press callback");
        exit(err);
    }
    i = 0;
    while (1) {
        // Format print data
        len = snprintf((char *)buf, sizeof(buf),
                       "Foreground task running, iteration %d\n", i);
        // Write data to UART
        LOG_I(TAG, "Writing data to UART");
        if (UART_write(uart_dev, buf, len, &err) != len) {
            LOG_E(TAG, "UART write failed");
            exit(err);
        }
        i++;
        // Wait for a button press
        semaphore_pend(buttonpress_sem, SYS_TIMEOUT_INF);
        LOG_I(TAG, "Woke from semaphore");
    }
}

/**
 * Background task. Logs to SWO (or semihosting, if configured in config.h)
 * at 2MHz, with a cpu clock of 80MHz. Uses ITM stimulus port 0.
 * This task runs with a lower priority than the foreground one, and should
 * only run when it is pending on the semaphore. It will periodically sleep,
 * at which point the idle task will run (which does not log output)
 *
 * This task will log to SWO a total of 31 times, after which it will exit
 * cleanly.
 *
 * The task will also toggle the user LED to give visual indication it is
 * running
 * @param arg: Unused.
 */
static void background_task(void *arg) {
    const char *TAG = "Background Task";
    int i = 30;
    // Print out 30 times
    while (i--) {
        printf("Background task running\n");
        GPIO_write(GPIO_PB13, GPIO_HIGH);
        task_delay(1000); // Will awake after 1 second and turn off led
        GPIO_write(GPIO_PB13, GPIO_LOW);
        task_delay(500); // Sleep again until it is time to log
    }
    // Task will exit here. Log to SWO one more time
    LOG_I(TAG, "Background task exiting");
    return;
}

/**
 * Button press callback. Called via interrupt when user button B1 is pressed.
 */
static void button_callback(void) {
    // Post to the button press semaphore
    semaphore_post(buttonpress_sem);
}
