
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <gpio/gpio.h>
#include <sys/clock.h>
#include <uart/uart.h>


/**
 * Initializes system
 */
void system_init() {
    clock_cfg_t clk_cfg = CLOCK_DEFAULT_CONFIG;
    clock_init(&clk_cfg);
}

/**
 * Testing point for development
 */
int main() {
    printf("Hello world\n"); 
    return SYS_OK;
}