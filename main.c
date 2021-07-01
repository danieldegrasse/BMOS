
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <gpio/gpio.h>
#include <sys/clock.h>
#include <uart/uart.h>
#include <util/logging.h>


/**
 * Initializes system
 */
void system_init() {
    clock_cfg_t clk_cfg = CLOCK_DEFAULT_CONFIG;
    clock_init(&clk_cfg);
}


/**
 * Initializes GPIO for UART
 */
void gpio_init() {
    GPIO_config_t uart_pinconf = GPIO_DEFAULT_CONFIG;
    uart_pinconf.alternate_func = GPIO_af8;
    uart_pinconf.mode = GPIO_mode_afunc;
    uart_pinconf.output_speed = GPIO_speed_vhigh;
    uart_pinconf.pullup_pulldown = GPIO_pullup;
    // TX pin
    GPIO_config(GPIO_PORT_A, GPIO_PIN_2, &uart_pinconf);
    // RX pin
    GPIO_config(GPIO_PORT_A, GPIO_PIN_3, &uart_pinconf);
}

/**
 * Testing point for development
 */
int main() {
    syserr_t err;
    gpio_init();
    /** Init uart device */
    UART_handle_t lpuart;
    UART_config_t uart_conf = UART_DEFAULT_CONFIG;
    uart_conf.UART_baud_rate = UART_baud_115200;
    uart_conf.UART_echomode = UART_echo_en;
    uart_conf.UART_textmode = UART_txtmode_en;
    lpuart = UART_open(LPUART_1, &uart_conf, &err);
    if (lpuart == NULL) {
        LOG_E("Failed to open LPUART\n");
        exit(-1);
    }
    UART_write(lpuart, (uint8_t*)"Hello!\n", 7, &err);
    /*
    printf("Hello world\n"); 
    LOG_D("Not I");
    LOG_E("I'll print"); 
    */
    UART_write(lpuart, (uint8_t*)"Done!\n", 6, &err);
    delay_ms(1000);
    UART_close(lpuart);
    return SYS_OK;
}