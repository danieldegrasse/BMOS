
/**
 * @file clock_test.c
 * Tests system clock configuration. Starts by blinking the LED (User LED D4)
 * with the system clock set to 4MHz. This should result in the LED blinking
 * slowly.
 *
 * Once this led has cycled 5 times, clock frequency is raised to 80MHz,
 * and the LED is blinked with the same delay. This should result in a very
 * fast cycle of 5 blinks.
 *
 * After the LED cycles 5 times, the led will be blinked 5 times with a 1000ms
 * delay. This tests the delay() function within the clock code
 *
 * After the LED cycles 5 times with the delay, clock speed will be set to 16MHz
 * using the HSI16 oscillator. The LED will then be blinked with the same preset
 * delay. This should result in a "somewhat" fast blink cycle
 * 
 * Finally, after this blink cycle of 5, the LED will blink 5 times with a
 * 10000 ms delay. This tests a longer delay
 *
 * After these cycles complete, the device will simply spin in a while loop.
 */

#include <stdlib.h>

#include <drivers/gpio/gpio.h>
#include <drivers/clock/clock.h>
#include <drivers/uart/uart.h>

#define DELAY 100000
#define CYCLES 5

/**
 * Blinks the user LED using the preset delay
 */
void blink_led_delay() {
    // Turn on user LED
    GPIO_write(GPIO_PORT_B, GPIO_PIN_13, GPIO_HIGH);
    for (volatile int i = 0; i < DELAY; i++) {
        // Delay
    }
    // Turn off user LED
    GPIO_write(GPIO_PORT_B, GPIO_PIN_13, GPIO_LOW);
    for (volatile int i = 0; i < DELAY; i++) {
        // Delay
    }
}

int main() {
    syserr_t err;
    GPIO_config_t led_cfg = GPIO_DEFAULT_CONFIG;
    clock_cfg_t clk_cfg = CLOCK_DEFAULT_CONFIG;
    int cycles = CYCLES;
    // Set clock config to use MSI at 4MHz
    clk_cfg.PLL_en = false;
    clk_cfg.sysclk_src = CLK_MSI;
    clk_cfg.MSI_freq = MSI_freq_4MHz;
    clk_cfg.HSI16_freq = HSI16_freq_disabled;
    // Set system clock to 4MHz
    err = clock_init(&clk_cfg);
    if (err != SYS_OK) {
        // Spin
        while (1)
            ;
    }
    // Enable LED D4
    GPIO_config(GPIO_PORT_B, GPIO_PIN_13, &led_cfg);
    while (cycles--) {
        blink_led_delay();
    }
    // Now reconfigure the clock at 80MHz
    clk_cfg.PLL_en = true;
    clk_cfg.PLLN_mul = 40;
    clk_cfg.PLLR_div = PLLR_2;
    clk_cfg.sysclk_src = CLK_PLL;
    clk_cfg.MSI_freq = MSI_freq_4MHz;
    err = clock_init(&clk_cfg);
    if (err != SYS_OK) {
        // Spin
        while (1)
            ;
    }
    // blink the LED again
    cycles = CYCLES;
    while (cycles--) {
        blink_led_delay();
    }
    // Now blink with a delay of 1000ms
    cycles = CYCLES;
    while (cycles--) {
        // Turn on user LED
        GPIO_write(GPIO_PORT_B, GPIO_PIN_13, GPIO_HIGH);
        blocking_delay_ms(1000);
        // Turn off user LED
        GPIO_write(GPIO_PORT_B, GPIO_PIN_13, GPIO_LOW);
        blocking_delay_ms(1000);
    }
    /**
     * Verify function failures
     */
    // Check to make sure PLL cannot be modified when it is sysclk
    clk_cfg.PLL_en = false;
    err = clock_init(&clk_cfg);
    if (err == SYS_OK) {
        while (1)
            ;
        // spin
    }
    // Check to make sure a disabled oscillator cannot be used for sysclock
    clk_cfg.PLL_en = true;
    clk_cfg.sysclk_src = CLK_HSI16;
    err = clock_init(&clk_cfg);
    if (err == SYS_OK) {
        while (1)
            ;
        // spin
    }
    // Switch to HSI16 as the oscillator
    clk_cfg.HSI16_freq = HSI16_freq_16MHz;
    clk_cfg.sysclk_src = CLK_HSI16;
    err = clock_init(&clk_cfg);
    if (err != SYS_OK) {
        while (1)
            ;
        // spin
    }
    // Verify that the PLL can now be disabled
    clk_cfg.PLL_en = false;
    err = clock_init(&clk_cfg);
    if (err != SYS_OK) {
        while (1) // spin
            ;
    }
    if (pllclock_freq() != CLOCK_DISABLED) {
        while (1) // spin
            ;
    }
    // Blink LED
    cycles = CYCLES;
    while (cycles--) {
        blink_led_delay();
    }
    cycles = CYCLES;
    while (cycles--) {
        // Turn on user LED
        GPIO_write(GPIO_PORT_B, GPIO_PIN_13, GPIO_HIGH);
        blocking_delay_ms(10000);
        // Turn off user LED
        GPIO_write(GPIO_PORT_B, GPIO_PIN_13, GPIO_LOW);
        blocking_delay_ms(10000);
    }
    while (1)
        ; // Spin
    return SYS_OK;
}