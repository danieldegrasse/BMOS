
#include <gpio/gpio.h>
#include <sys/clock.h>
/**
 * Basic code to blink D4 LED on STM32L433 MCU
 */
int main() {
    // Set system clock to 80MHz
    select_sysclock(CLK_MSI_32MHz);
    GPIO_config_t config = GPIO_DEFAULT_CONFIG;
    GPIO_config(GPIO_PORT_B, GPIO_PIN_13, &config);
    while (1) {
        GPIO_write(GPIO_PORT_B, GPIO_PIN_13, GPIO_HIGH);
        for (int i = 0; i < 100000; i++) {
            // Delay
        }
        GPIO_write(GPIO_PORT_B, GPIO_PIN_13, GPIO_LOW);
        for (int i = 0; i < 100000; i++) {
            // Delay
        }
    }
    return SYS_OK;
}