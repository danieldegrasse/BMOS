#ifndef GPIO_H
#define GPIO_H

/**
 * @file gpio.h
 * Driver for STM32L4xxxx GPIO
 */

#include <sys/err.h>

/**
 * Available GPIO Ports
 */
typedef enum GPIO_port {
    GPIO_PORT_A,
    GPIO_PORT_B,
    GPIO_PORT_C,
    GPIO_PORT_D,
    GPIO_PORT_E,
    GPIO_PORT_H,
} GPIO_port_t;

/**
 * GPIO Pins
 */
typedef enum GPIO_pin {
    GPIO_PIN_0 = 0,
    GPIO_PIN_1 = 1,
    GPIO_PIN_2 = 2,
    GPIO_PIN_3 = 3,
    GPIO_PIN_4 = 4,
    GPIO_PIN_5 = 5,
    GPIO_PIN_6 = 6,
    GPIO_PIN_7 = 7,
    GPIO_PIN_8 = 8,
    GPIO_PIN_9 = 9,
    GPIO_PIN_10 = 10,
    GPIO_PIN_11 = 11,
    GPIO_PIN_12 = 12,
    GPIO_PIN_13 = 13,
    GPIO_PIN_14 = 14,
    GPIO_PIN_15 = 15,
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
 * @param port: GPIO port to use
 * @param pin: GPIO pin to configure
 * @param config: GPIO configuration structure
 */
syserr_t GPIO_config(GPIO_port_t port, GPIO_pin_t pin, GPIO_config_t *config);

/**
 * Write a voltage level (high or low) to a GPIO pin
 * @param port: GPIO Port pin is on
 * @param pin: pin to set
 * @param lvl: GPIO level to set
 */
syserr_t GPIO_write(GPIO_port_t port, GPIO_pin_t pin, GPIO_level_t lvl);

/**
 * Read the digital voltage level from a pin
 * @param port: GPIO Port pin is on
 * @param pin: pin to set
 * @return GPIO pin level
 */
GPIO_level_t GPIO_read(GPIO_port_t port, GPIO_pin_t pin);

#endif