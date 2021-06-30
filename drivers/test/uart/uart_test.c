
/**
 * @file uart_test.c
 * Implements a series of tests for the UART driver, using the LPUART1 device
 * The test requires a serial terminal at 115200 baud (with around 80 visible
 * text columns or line wrap enabled) be open on to view the UART output. The
 * output of a successful test is reproduced below:
 */

#include <stdlib.h>
#include <string.h>

#include <gpio/gpio.h>
#include <sys/clock.h>
#include <uart/uart.h>

/**
 * Sets up the GPIO pins used by the UART device
 */
syserr_t init_uart_gpio() {
    GPIO_config_t uart_gpio = GPIO_DEFAULT_CONFIG;
    syserr_t err;
    uart_gpio.alternate_func = GPIO_af8;
    uart_gpio.mode = GPIO_mode_afunc;
    uart_gpio.pullup_pulldown = GPIO_pullup; // UART is idle high
    uart_gpio.output_speed = GPIO_speed_vhigh;
    // TX pin (pullup)
    err = GPIO_config(GPIO_PORT_A, GPIO_PIN_2, &uart_gpio);
    if (err != SYS_OK) {
        return err;
    }
    // RX pin (pullup)
    GPIO_config(GPIO_PORT_A, GPIO_PIN_3, &uart_gpio);
    if (err != SYS_OK) {
        return err;
    }
    return SYS_OK;
}

#define READBUF_LEN 90
#define ECHO_COUNT 10

int main() {
    UART_handle_t lpuart;
    UART_config_t lpuart_config = UART_DEFAULT_CONFIG;
    clock_cfg_t clk_cfg = CLOCK_DEFAULT_CONFIG;
    lpuart_config.UART_textmode = UART_txtmode_en;
    char *promptstrings[] = {
        "Welcome to the UART device test!\n",
        "This string tests the ability of the uart driver to write very long \n"
        "data strings. If you do not see 'AAAA' at the end of this string, \n"
        "the test failed: 'AAAA'\n",
        "The UART device will now echo characters. Type 10 characters into \n"
        "the serial terminal, and you should see them echoed back to you as \n"
        "you type:\n",
        "The UART device will now attempt a bulk read. Type 90 characters \n"
        "into the serial terminal, and you should see them all echoed back \n"
        "at once\n",
        "Successfully read 90 characters. Now, wait several seconds \n"
        "without typing. The UART device should print that it read 0 \n"
        "characters from the device.\n",
        "The UART device successfully read 0 characters\n",
        "This is the final UART test. It verifies the write timeout. This\n"
        "message is delibrately very long. You should not see the end of \n"
        "this message. The ending characters will be 'EEEE'. If you see a\n"
        "second instance of those characters in this message, the test has\n"
        "failed 'EEEE'\n",
        "\nAll UART tests passed! The device will enter echo mode now.\n"
        "It should echo all characters typed to the prompt.\n"};
    syserr_t err;
    uint8_t buf[READBUF_LEN];
    int len, count;
    // Set system clock to 80MHz
    clock_init(&clk_cfg);
    if (init_uart_gpio() != SYS_OK) {
        while (1)
            ; // Spin
    }
    /**
     * Verify that the UART will not open with too low a baud rate
     */
    lpuart_config.UART_baud_rate = UART_baud_9600;
    lpuart = UART_open(LPUART_1, &lpuart_config, &err);
    if (lpuart != NULL) {
        while (1)
            ; // spin
    }
    lpuart_config.UART_baud_rate = UART_baud_115200;
    lpuart = UART_open(LPUART_1, &lpuart_config, &err);
    if (lpuart == NULL || err != SYS_OK) {
        while (1)
            ; // spin
    }
    // Write opening prompt
    len = UART_write(lpuart, (uint8_t *)promptstrings[0],
                     strlen(promptstrings[0]), &err);
    if (len != strlen(promptstrings[0])) {
        while (1)
            ; // spin
    }
    // Write long data string
    len = UART_write(lpuart, (uint8_t *)promptstrings[1],
                     strlen(promptstrings[1]), &err);
    if (len != strlen(promptstrings[1])) {
        while (1)
            ; // spin
    }
    // Write echo prompt
    len = UART_write(lpuart, (uint8_t *)promptstrings[2],
                     strlen(promptstrings[2]), &err);
    if (len != strlen(promptstrings[2])) {
        while (1)
            ; // spin
    }
    count = ECHO_COUNT;
    // Echo ECHO_COUNT characters one by one
    while (count--) {
        len = UART_read(lpuart, buf, 1, &err);
        UART_write(lpuart, buf, len, &err);
    }
    // Read a large block of characters
    len = UART_write(lpuart, (uint8_t *)promptstrings[3],
                     strlen(promptstrings[3]), &err);
    if (len != strlen(promptstrings[3])) {
        while (1)
            ; // spin
    }
    len = UART_read(lpuart, buf, READBUF_LEN, &err);
    if (len != READBUF_LEN) {
        while (1)
            ; // spin
    }
    len = UART_write(lpuart, buf, len, &err);
    if (len != READBUF_LEN) {
        while (1)
            ; // spin
    }
    // Close and reopen the UART device with a timeout set
    if (UART_close(lpuart) != SYS_OK) {
        while (1)
            ; // Spin
    }
    lpuart_config.UART_read_timeout = 2000;
    lpuart_config.UART_write_timeout = 10;
    lpuart = UART_open(LPUART_1, &lpuart_config, &err);
    if (lpuart == NULL || err != SYS_OK) {
        while (1)
            ; // spin
    }
    len = UART_write(lpuart, (uint8_t *)promptstrings[4],
                     strlen(promptstrings[4]), &err);
    if (len != strlen(promptstrings[4])) {
        while (1)
            ; // spin
    }
    // Flush the read buffer
    UART_read(lpuart, buf, READBUF_LEN, &err);
    delay_ms(5000); // To give user time to see message
    len = UART_read(lpuart, buf, READBUF_LEN, &err);
    if (len != 0) {
        while (1)
            ; // spin
    }
    // Print message noting that read timeout passed
    len = UART_write(lpuart, (uint8_t *)promptstrings[5],
                     strlen(promptstrings[5]), &err);
    if (len != strlen(promptstrings[5])) {
        while (1)
            ; // spin
    }
    // Print long message to verify UART write timeout
    len = UART_write(lpuart, (uint8_t *)promptstrings[6],
                     strlen(promptstrings[6]), &err);
    if (len == strlen(promptstrings[6])) {
        while (1)
            ; // spin
    }
    // delay for a few ms, so the UART tx can end
    delay_ms(100);
    // Close the UART and reopen it with echo enabled.
    if (UART_close(lpuart) != SYS_OK) {
        while (1)
            ; // Spin
    }
    lpuart_config.UART_read_timeout = UART_TIMEOUT_INF;
    lpuart_config.UART_write_timeout = UART_TIMEOUT_INF;
    lpuart_config.UART_echomode = UART_echo_en;
    lpuart = UART_open(LPUART_1, &lpuart_config, &err);
    if (lpuart == NULL || err != SYS_OK) {
        while (1)
            ; // spin
    }
    // Print success message
    len = UART_write(lpuart, (uint8_t *)promptstrings[7],
                     strlen(promptstrings[7]), &err);
    if (len != strlen(promptstrings[7])) {
        while (1)
            ; // spin
    }
    return SYS_OK;
}