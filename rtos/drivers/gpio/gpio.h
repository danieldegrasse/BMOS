#ifndef GPIO_H
#define GPIO_H

/**
 * @file gpio.h
 * Driver for STM32L4xxxx GPIO
 */

#include <sys/err.h>

/**
 * GPIO pin number and port defs. Masked together to form pin definitions
 */
#define PORTSHIFT 4
#define PORTMASK (0xFU << PORTSHIFT)
#define PORT_A (1U << PORTSHIFT)
#define PORT_B (2U << PORTSHIFT)
#define PORT_C (3U << PORTSHIFT)
#define PORT_D (4U << PORTSHIFT)
#define PORT_E (5U << PORTSHIFT)
#define PORT_H (6U << PORTSHIFT)

#define PINMASK 0xFU

#define PIN_0 0U
#define PIN_1 1U
#define PIN_2 2U
#define PIN_3 3U
#define PIN_4 4U
#define PIN_5 5U
#define PIN_6 6U
#define PIN_7 7U
#define PIN_8 8U
#define PIN_9 9U
#define PIN_9 9U
#define PIN_10 10U
#define PIN_11 11U
#define PIN_12 12U
#define PIN_13 13U
#define PIN_14 14U
#define PIN_15 15U

/**
 * GPIO Pins defined on the LQFP64 STM32L433RC package
 */
typedef enum GPIO_pin {
    GPIO_PA0 = PORT_A | PIN_0,
    GPIO_PA1 = PORT_A | PIN_1,
    GPIO_PA2 = PORT_A | PIN_2,
    GPIO_PA3 = PORT_A | PIN_3,
    GPIO_PA4 = PORT_A | PIN_4,
    GPIO_PA5 = PORT_A | PIN_5,
    GPIO_PA6 = PORT_A | PIN_6,
    GPIO_PA7 = PORT_A | PIN_7,
    GPIO_PA8 = PORT_A | PIN_8,
    GPIO_PA9 = PORT_A | PIN_9,
    GPIO_PA10 = PORT_A | PIN_10,
    GPIO_PA11 = PORT_A | PIN_11,
    GPIO_PA12 = PORT_A | PIN_12,
    GPIO_PA13 = PORT_A | PIN_13,
    GPIO_PA14 = PORT_A | PIN_14,
    GPIO_PA15 = PORT_A | PIN_15,
    GPIO_PB0 = PORT_B | PIN_0,
    GPIO_PB1 = PORT_B | PIN_1,
    GPIO_PB2 = PORT_B | PIN_2,
    GPIO_PB3 = PORT_B | PIN_3,
    GPIO_PB4 = PORT_B | PIN_4,
    GPIO_PB5 = PORT_B | PIN_5,
    GPIO_PB6 = PORT_B | PIN_6,
    GPIO_PB7 = PORT_B | PIN_7,
    GPIO_PB8 = PORT_B | PIN_8,
    GPIO_PB9 = PORT_B | PIN_9,
    GPIO_PB10 = PORT_B | PIN_10,
    GPIO_PB11 = PORT_B | PIN_11,
    GPIO_PB12 = PORT_B | PIN_12,
    GPIO_PB13 = PORT_B | PIN_13,
    GPIO_PB14 = PORT_B | PIN_14,
    GPIO_PB15 = PORT_B | PIN_15,
    GPIO_PC0 = PORT_C | PIN_0,
    GPIO_PC1 = PORT_C | PIN_1,
    GPIO_PC2 = PORT_C | PIN_2,
    GPIO_PC3 = PORT_C | PIN_3,
    GPIO_PC4 = PORT_C | PIN_4,
    GPIO_PC5 = PORT_C | PIN_5,
    GPIO_PC6 = PORT_C | PIN_6,
    GPIO_PC7 = PORT_C | PIN_7,
    GPIO_PC8 = PORT_C | PIN_8,
    GPIO_PC9 = PORT_C | PIN_9,
    GPIO_PC10 = PORT_C | PIN_10,
    GPIO_PC11 = PORT_C | PIN_11,
    GPIO_PC12 = PORT_C | PIN_12,
    GPIO_PC13 = PORT_C | PIN_13,
    GPIO_PC14 = PORT_C | PIN_14, /*!< OSC32_IN */
    GPIO_PC15 = PORT_C | PIN_15, /*!< OSC32_OUT */
    GPIO_PD2 = PORT_D | PIN_2,
    GPIO_PH0 = PORT_H | PIN_0, /*!< OSC_IN*/
    GPIO_PH1 = PORT_H | PIN_1, /*!< OSC_OUT*/
    GPIO_PH3 = PORT_H | PIN_3, /*!< B00T0 */
} GPIO_pin_t;

/**
 * GPIO I/O modes
 */
typedef enum GPIO_mode {
    GPIO_mode_input,
    GPIO_mode_output,
    GPIO_mode_afunc,
    GPIO_mode_analog,
} GPIO_mode_t;

/**
 * GPIO Output type modes
 */
typedef enum GPIO_otype {
    GPIO_pushpull,
    GPIO_opendrain,
} GPIO_otype_t;

/**
 * GPIO output speed
 */
typedef enum GPIO_ospeed {
    GPIO_speed_low,
    GPIO_speed_med,
    GPIO_speed_high,
    GPIO_speed_vhigh,
} GPIO_ospeed_t;

/**
 * Pullup/pulldown modes
 */
typedef enum GPIO_pupd {
    GPIO_no_pull, /**! No pullup or pulldown */
    GPIO_pullup,
    GPIO_pulldown
} GPIO_pupd_t;

/**
 * GPIO alternate function modes
 */
typedef enum GPIO_af {
    GPIO_af0,
    GPIO_af1,
    GPIO_af2,
    GPIO_af3,
    GPIO_af4,
    GPIO_af5,
    GPIO_af6,
    GPIO_af7,
    GPIO_af8,
    GPIO_af9,
    GPIO_af10,
    GPIO_af11,
    GPIO_af12,
    GPIO_af13,
    GPIO_af14,
    GPIO_af15,
    GPIO_af_dis, /*!< Alternate function disabled */
} GPIO_af_t;

typedef enum GPIO_level {
    GPIO_LOW = 0,
    GPIO_HIGH = 1,
} GPIO_level_t;

typedef struct {
    GPIO_mode_t mode;
    GPIO_otype_t output_type;
    GPIO_ospeed_t output_speed;
    GPIO_pupd_t pullup_pulldown;
    GPIO_af_t alternate_func;
} GPIO_config_t;

/**
 * Default GPIO configuration:
 * Output mode
 * pushpull output
 * low speed output
 * no pullup/pulldown
 * alternate function disabled
 */
#define GPIO_DEFAULT_CONFIG                                                    \
    {                                                                          \
        .mode = GPIO_mode_output, .output_type = GPIO_pushpull,                \
        .output_speed = GPIO_speed_low, .pullup_pulldown = GPIO_no_pull,       \
        .alternate_func = GPIO_af_dis                                          \
    }

/**
 * Configure a GPIO port for use with driver
 * @param pin: GPIO pin to configure
 * @param config: GPIO configuration structure
 */
syserr_t GPIO_config(GPIO_pin_t pin, GPIO_config_t *config);

/**
 * Write a voltage level (high or low) to a GPIO pin
 * @param pin: pin to set
 * @param lvl: GPIO level to set
 */
syserr_t GPIO_write(GPIO_pin_t pin, GPIO_level_t lvl);

/**
 * Read the digital voltage level from a pin
 * @param pin: pin to set
 * @return GPIO pin level
 */
GPIO_level_t GPIO_read(GPIO_pin_t pin);

#endif