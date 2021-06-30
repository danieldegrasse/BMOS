/**
 * @file gpio_test.c
 * Tests system GPIO
 * 
 * This code, when executing correctly, should do the following:
 * - Blink the user LED at boot (D4 on development board)
 * - When B1 on the development board is held, blink D4 much quicker
 */

#include <gpio/gpio.h>
#include <sys/err.h>

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
    ret = GPIO_config(GPIO_PORT_B, GPIO_PIN_13, &led_cfg);
    if (ret != SYS_OK) {
        return ret;
    }
    ret = GPIO_config(GPIO_PORT_C, GPIO_PIN_13, &btn_cfg);
    if (ret != SYS_OK) {
        return ret;
    }
    return SYS_OK;
}

int main() {
    int i, delay = 500000;
    if (gpio_init() != SYS_OK) {
        // Spin
        while (1);
    }
    while (1) {
        // Illuminate Led D4
        GPIO_write(GPIO_PORT_B, GPIO_PIN_13, GPIO_HIGH);
        // Delay
        for (i = 0; i < delay; i++);
        // Turn the LED off
        GPIO_write(GPIO_PORT_B, GPIO_PIN_13, GPIO_LOW);
        for (i = 0; i < delay; i++);
        if (GPIO_read(GPIO_PORT_C, GPIO_PIN_13) == GPIO_HIGH) {
            // User button B1 is pressed. Change delay.
            delay = 10000;
        } else {
            // Reset delay
            delay = 500000;
        }
    }
    return SYS_OK;
}