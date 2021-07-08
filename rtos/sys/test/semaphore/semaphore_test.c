/**
 * @file semaphore_test.c
 * Test RTOS semaphore pending and posting
 * When this test runs correctly, The foreground test should start and pend
 * on the semaphore for 1500ms. Then, the foreground task should create a
 * background task of lower priority. The foreground task should then pend
 * on the semaphore with no timeout. At this point, the background task should
 * run, enter into a 3000ms delay, and then post to the semaphore, which will
 * wake the foreground task (instantly if preemption is enabled). The foreground
 * test will then notify the user it woke from the semaphore pend, and pend
 * again. This ping-pong process of the foreground task pending and the
 * background task posting should continue indefinitely.
 *
 * Here is the expected output from LPUART1 (115200 baud, 8n1):
 * Foreground task waiting on semaphore with timeout of 1500ms
 * Foreground task correctly timed out from semaphore pend
 * Foreground task running
 * Foreground task pending on semaphore
 * Foreground task woke from semaphore
 * Foreground task running
 * Foreground task pending on semaphore
 * Foreground task woke from semaphore
 * Foreground task running
 * Foreground task pending on semaphore
 * Foreground task woke from semaphore
 * Foreground task running
 * Foreground task pending on semaphore
 * .... (this pend/post cycle will continue) ......
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

static void fg_task(void *arg);
static void bg_task(void *arg);

static semaphore_t semaphore_handle;
static UART_handle_t lpuart1;

/**
 * Initializes system
 */
static void system_init() {
    clock_cfg_t clk_cfg = CLOCK_DEFAULT_CONFIG;
    clock_init(&clk_cfg);
}

/**
 * Sets up LPUART 1
 */
static void init_lpuart1() {
    const char *TAG = "init_lpuart1";
    syserr_t err;
    UART_config_t lpuart_conf = UART_DEFAULT_CONFIG;
    GPIO_config_t gpio_config = GPIO_DEFAULT_CONFIG;
    /* Init GPIO pins A2 and A3 for tx/rx */
    gpio_config.mode = GPIO_mode_afunc;
    gpio_config.alternate_func = GPIO_af8;
    gpio_config.output_speed = GPIO_speed_vhigh;
    gpio_config.pullup_pulldown = GPIO_pullup;
    err = GPIO_config(GPIO_PORT_A, GPIO_PIN_2, &gpio_config);
    if (err != SYS_OK) {
        LOG_E(TAG, "Could not init GPIO A2");
        exit(err);
    }
    err = GPIO_config(GPIO_PORT_A, GPIO_PIN_3, &gpio_config);
    if (err != SYS_OK) {
        LOG_E(TAG, "Could not init GPIO A3");
        exit(err);
    }
    /* Configure UART device */
    lpuart_conf.UART_baud_rate = UART_baud_115200;
    lpuart_conf.UART_textmode = UART_txtmode_en;
    lpuart_conf.UART_echomode = UART_echo_en;
    lpuart1 = UART_open(LPUART_1, &lpuart_conf, &err);
    if (lpuart1 == NULL) {
        LOG_E(TAG, "Could not init LPUART1");
        exit(err);
    }
}

/**
 * Foreground task entry point. Runs with a high priority, and will pend on
 * a semaphore after printing data to LPUART1
 * @param arg: unused.
 */
static void fg_task(void *arg) {
    syserr_t err;
    task_config_t bg_taskconf = DEFAULT_TASK_CONFIG;
    const char *TAG = "Foreground Task";
    LOG_I(TAG, "Foreground Task starting");
    /* Create semaphores and UART handle */
    init_lpuart1();
    semaphore_handle = semaphore_create_binary();
    if (semaphore_handle == NULL) {
        LOG_E(TAG, "Could not create semaphore");
    }
    UART_write(
        lpuart1,
        (uint8_t
             *)"Foreground task waiting on semaphore with timeout of 1500ms\n",
        60, &err);
    LOG_I(TAG, "Attempting to pend on semaphore with timeout of 1500ms");
    if (semaphore_pend(semaphore_handle, 1500) == SYS_OK) {
        LOG_E(TAG, "Semaphore test failed, pend had successful return value on "
                   "timeout");
    };
    LOG_I(TAG, "Returned from pend with timeout");
    UART_write(
        lpuart1,
        (uint8_t *)"Foreground task correctly timed out from semaphore pend\n",
        56, &err);
    LOG_I(TAG, "Creating low priority background task");
    bg_taskconf.task_priority = DEFAULT_PRIORITY - 1;
    bg_taskconf.task_name = "Background Task";
    if (task_create(bg_task, NULL, &bg_taskconf) == NULL) {
        LOG_E(TAG, "Could not create background task");
    }
    /** Main runloop. Print to UART device, then pend on semaphore */
    while (1) {
        UART_write(lpuart1, (uint8_t *)"Foreground task running\n", 24, &err);
        if (err != SYS_OK) {
            LOG_E(TAG, "Failed to write to UART device");
            exit(err);
        }
        UART_write(lpuart1, (uint8_t *)"Foreground task pending on semaphore\n",
                   37, &err);
        if (err != SYS_OK) {
            LOG_E(TAG, "Failed to write to UART device");
            exit(err);
        }
        LOG_D(TAG, "Foreground task pending on semaphore");
        semaphore_pend(semaphore_handle, SYS_TIMEOUT_INF);
        UART_write(lpuart1, (uint8_t *)"Foreground task woke from semaphore\n",
                   36, &err);
        LOG_D(TAG, "Foreground task awoke from semaphore");
        if (err != SYS_OK) {
            LOG_E(TAG, "Failed to write to UART device");
            exit(err);
        }
    }
    semaphore_destroy(semaphore_handle);
}

/**
 * Background task entry point. Runs with low priority, and will post to
 * semaphore after a delay
 * @param arg: Unused
 */
static void bg_task(void *arg) {
    const char *TAG = "Background Task";
    while (1) {
        LOG_I(TAG, "Task sleeping for 3000ms");
        task_delay(3000);
        LOG_I(TAG, "Posting to semaphore");
        semaphore_post(semaphore_handle);
    }
}

/**
 * Testing entry point. Tests semaphore pending and posting
 */
int main() {
    const char *TAG = "main";
    task_config_t fgtask_conf = DEFAULT_TASK_CONFIG;
    fgtask_conf.task_name = "Foreground Task";
    /* Init system */
    system_init();
    /* Create foreground task */
    if (task_create(fg_task, NULL, &fgtask_conf) == NULL) {
        LOG_E(TAG, "Failed to create rtos task");
        return ERR_FAIL;
    }
    LOG_I(TAG, "Starting RTOS");
    rtos_start();
    return SYS_OK;
}