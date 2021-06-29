
#include <stdlib.h>
#include <string.h>

#include <gpio/gpio.h>
#include <sys/clock.h>
#include <uart/uart.h>
/**
 * Basic code to blink D4 LED on STM32L433 MCU
 */
int main() {
    UART_handle_t lpuart;
    UART_config_t lpuart_confg = UART_DEFAULT_CONFIG;
    GPIO_config_t config = GPIO_DEFAULT_CONFIG;
    GPIO_config_t uart_gpio = GPIO_DEFAULT_CONFIG;
    clock_cfg_t clk_cfg = CLOCK_DEFAULT_CONFIG;
    char data[] = "Echoing characters";
    char buf[80];
    syserr_t err;
    int len;
    // Set system clock to 80MHz
    clock_init(&clk_cfg);
    GPIO_config(GPIO_PORT_B, GPIO_PIN_13, &config);
    while (0) {
        GPIO_write(GPIO_PORT_B, GPIO_PIN_13, GPIO_HIGH);
        for (int i = 0; i < 100000; i++) {
            // Delay
        }
        GPIO_write(GPIO_PORT_B, GPIO_PIN_13, GPIO_LOW);
        for (int i = 0; i < 100000; i++) {
            // Delay
        }
    }
    // Test UART
    // Configure PA2 and PA3 for AF8 (to use LPUART)
    uart_gpio.alternate_func = GPIO_af8;
    uart_gpio.mode = GPIO_mode_afunc;
    uart_gpio.pullup_pulldown = GPIO_pullup;
    uart_gpio.output_speed = GPIO_speed_vhigh;
    // TX pin (pullup)
    GPIO_config(GPIO_PORT_A, GPIO_PIN_2, &uart_gpio);
    // RX pin (pullup)
    GPIO_config(GPIO_PORT_A, GPIO_PIN_3, &uart_gpio);
    lpuart = UART_open(LPUART_1, &lpuart_confg, &err);
    if (lpuart == NULL) {
        while (1)
            ; // spin
    }
    UART_write(lpuart, (uint8_t *)data, strlen(data), &err);
    while (1) {
        len = UART_read(lpuart, (uint8_t *)buf, sizeof(buf), &err);
        UART_write(lpuart, (uint8_t *)buf, len, &err);
    }
    return SYS_OK;
}