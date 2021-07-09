/**
 * @file gpio_test.c
 * Tests system GPIO
 *
 * This code, when executing correctly, should do the following:
 * - Blink the user LED at boot (D4 on development board)
 * - Pressing B1 (user button) should switch the user LED between a fast and
 * slow blink cycle
 */

#include <drivers/gpio/gpio.h>
#include <sys/err.h>

#define DELAY_SHORT 50000
#define DELAY_LONG 500000
volatile int delay = DELAY_SHORT;

/**
 * Handles interrupts on press of user button B1
 */
void gpio_inthandler(void) {
    if (delay == DELAY_SHORT) {
        delay = DELAY_LONG;
    } else {
        delay = DELAY_SHORT;
    }
}

/**
 * GPIO initialization function
 * Sets up PB13 as an output for the user LED, and
 * PC13 as an input for the user button
 */
syserr_t gpio_init() {
    syserr_t ret;
    // Set both pins to default config
    GPIO_config_t led_cfg = GPIO_DEFAULT_CONFIG;
    GPIO_config_t btn_cfg = GPIO_DEFAULT_CONFIG;
    // Modify led cfg to be output
    led_cfg.mode = GPIO_mode_output;
    // button should be input
    btn_cfg.mode = GPIO_mode_input;
    // configure both GPIOs
    ret = GPIO_config(GPIO_PB13, &led_cfg);
    if (ret != SYS_OK) {
        return ret;
    }
    ret = GPIO_config(GPIO_PC13, &btn_cfg);
    if (ret != SYS_OK) {
        return ret;
    }
    // Enable rising edge interrupts for user button
    ret = GPIO_interrupt_enable(GPIO_PC13, GPIO_trig_falling, gpio_inthandler);
    if (ret != SYS_OK) {
        return ret;
    }
    return SYS_OK;
}

int main() {
    volatile int i;
    if (gpio_init() != SYS_OK) {
        // Spin
        while (1)
            ;
    }
    while (1) {
        // Illuminate Led D4
        GPIO_write(GPIO_PB13, GPIO_HIGH);
        // Delay
        for (i = 0; i < delay; i++)
            ;
        // Turn the LED off
        GPIO_write(GPIO_PB13, GPIO_LOW);
        for (i = 0; i < delay; i++)
            ;
    }
    return SYS_OK;
}