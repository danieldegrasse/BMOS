#include <stdint.h>

#include <device/device.h>
#include "gpio.h"
#include <util/util.h>

/**
 * Configure a GPIO port for use with driver
 * @param port: GPIO port to use
 * @param pin: GPIO pin to configure
 * @param config: GPIO configuration structure
 */
syserr_t GPIO_config(GPIO_port_t port, GPIO_pin_t pin, GPIO_config_t *config) {
    // Begin by converting port into base register
    GPIO_TypeDef *periph;
    __IO uint32_t *af_sel;
    uint32_t shift = pin << 1; // 2 times value of pin
    /**
     * In this switch statement, we will also verify that the relevant
     * AHB2 peripheral clock is enabled for the selected port, since otherwise
     * registers will not accept writes
     */
    switch (port) {
    case GPIO_PORT_A:
        // Enable GPIOA
        SETBITS(RCC->AHB2ENR, RCC_AHB2ENR_GPIOAEN);
        periph = GPIOA;
        break;
    case GPIO_PORT_B:
        SETBITS(RCC->AHB2ENR, RCC_AHB2ENR_GPIOBEN);
        periph = GPIOB;
        break;
    case GPIO_PORT_C:
        SETBITS(RCC->AHB2ENR, RCC_AHB2ENR_GPIOCEN);
        periph = GPIOC;
        break;
    case GPIO_PORT_D:
        SETBITS(RCC->AHB2ENR, RCC_AHB2ENR_GPIODEN);
        periph = GPIOD;
        break;
    case GPIO_PORT_E:
        SETBITS(RCC->AHB2ENR, RCC_AHB2ENR_GPIOEEN);
        periph = GPIOE;
        break;
    case GPIO_PORT_H:
        SETBITS(RCC->AHB2ENR, RCC_AHB2ENR_GPIOHEN);
        periph = GPIOH;
        break;
    default:
        return ERR_BADPARAM;
    }
    /* Configure GPIO mode */
    CLEARFIELD(periph->MODER, GPIO_MODER_MASK, shift); // clear bits
    switch (config->mode) {
    case GPIO_mode_input:
        SETFIELD(periph->MODER, GPIO_MODER_INPUT, shift);
        break;
    case GPIO_mode_output:
        SETFIELD(periph->MODER, GPIO_MODER_OUTPUT, shift);
        break;
    case GPIO_mode_afunc:
        SETFIELD(periph->MODER, GPIO_MODER_AFUNC, shift);
        break;
    case GPIO_mode_analog:
        SETFIELD(periph->MODER, GPIO_MODER_ANALOG, shift);
        break;
    default:
        return ERR_BADPARAM;
        break;
    }
    /* Configure GPIO output type */
    // GPIO_OTYPER only uses lower 16 bits of register
    CLEARFIELD(periph->OTYPER, GPIO_OTYPER_MASK, pin);
    switch (config->output_type) {
    case GPIO_pushpull:
        SETFIELD(periph->OTYPER, GPIO_OTYPER_PUSHPULL, pin);
        break;
    case GPIO_opendrain:
        SETFIELD(periph->OTYPER, GPIO_OTYPER_ODRAIN, pin);
        break;
    default:
        return ERR_BADPARAM;
        break;
    }
    /* Configure GPIO output speed */
    CLEARFIELD(periph->OSPEEDR, GPIO_OSPEEDR_MASK, shift);
    switch (config->output_speed) {
    case GPIO_speed_low:
        SETFIELD(periph->OSPEEDR, GPIO_OSPEEDR_LOW, shift);
        break;
    case GPIO_speed_med:
        SETFIELD(periph->OSPEEDR, GPIO_OSPEEDR_MED, shift);
        break;
    case GPIO_speed_high:
        SETFIELD(periph->OSPEEDR, GPIO_OSPEEDR_HIGH, shift);
        break;
    case GPIO_speed_vhigh:
        SETFIELD(periph->OSPEEDR, GPIO_OSPEEDR_VHIGH, shift);
        break;
    default:
        return ERR_BADPARAM;
        break;
    }
    /* Configure GPIO pullup/pulldown */
    CLEARFIELD(periph->PUPDR, GPIO_PUPDR_MASK, shift);
    switch (config->pullup_pulldown) {
    case GPIO_no_pull:
        SETFIELD(periph->PUPDR, GPIO_PUPDR_NONE, shift);
        break;
    case GPIO_pullup:
        SETFIELD(periph->PUPDR, GPIO_PUPDR_PU, shift);
        break;
    case GPIO_pulldown:
        SETFIELD(periph->PUPDR, GPIO_PUPDR_PD, shift);
        break;
    default:
        return ERR_BADPARAM;
        break;
    }
    /* Configure GPIO alternate function select */
    if (pin < 7) {
        shift = pin << 2;
        // Use low alternate function register
        af_sel = &(periph->AFR[0]);
    } else {
        // Use high alternate function register
        shift = (pin - 8) << 2;
        af_sel = &(periph->AFR[1]);
    }
    CLEARFIELD(*af_sel, GPIO_AF_MASK, shift);
    switch (config->alternate_func) {
    case GPIO_af_dis: // Alternate function is disabled
        break;
    case GPIO_af0:
        SETFIELD(*af_sel, GPIO_AF0, shift);
        break;
    case GPIO_af1:
        SETFIELD(*af_sel, GPIO_AF1, shift);
        break;
    case GPIO_af2:
        SETFIELD(*af_sel, GPIO_AF2, shift);
        break;
    case GPIO_af3:
        SETFIELD(*af_sel, GPIO_AF3, shift);
        break;
    case GPIO_af4:
        SETFIELD(*af_sel, GPIO_AF4, shift);
        break;
    case GPIO_af5:
        SETFIELD(*af_sel, GPIO_AF5, shift);
        break;
    case GPIO_af6:
        SETFIELD(*af_sel, GPIO_AF6, shift);
        break;
    case GPIO_af7:
        SETFIELD(*af_sel, GPIO_AF7, shift);
        break;
    case GPIO_af8:
        SETFIELD(*af_sel, GPIO_AF8, shift);
        break;
    case GPIO_af9:
        SETFIELD(*af_sel, GPIO_AF9, shift);
        break;
    case GPIO_af10:
        SETFIELD(*af_sel, GPIO_AF10, shift);
        break;
    case GPIO_af11:
        SETFIELD(*af_sel, GPIO_AF11, shift);
        break;
    case GPIO_af12:
        SETFIELD(*af_sel, GPIO_AF12, shift);
        break;
    case GPIO_af13:
        SETFIELD(*af_sel, GPIO_AF13, shift);
        break;
    case GPIO_af14:
        SETFIELD(*af_sel, GPIO_AF14, shift);
        break;
    case GPIO_af15:
        SETFIELD(*af_sel, GPIO_AF15, shift);
        break;
    default:
        return ERR_BADPARAM;
        break;
    }
    return SYS_OK;
}

/**
 * Write a voltage level (high or low) to a GPIO pin
 * @param port: GPIO Port pin is on
 * @param pin: pin to set
 * @param lvl: GPIO level to set
 */
syserr_t GPIO_write(GPIO_port_t port, GPIO_pin_t pin, GPIO_level_t lvl) {
    // Begin by converting port into base register
    GPIO_TypeDef *periph;
    uint32_t shift = pin;
    switch (port) {
    case GPIO_PORT_A:
        periph = GPIOA;
        break;
    case GPIO_PORT_B:
        periph = GPIOB;
        break;
    case GPIO_PORT_C:
        periph = GPIOC;
        break;
    case GPIO_PORT_D:
        periph = GPIOD;
        break;
    case GPIO_PORT_E:
        periph = GPIOE;
        break;
    case GPIO_PORT_H:
        periph = GPIOH;
        break;
    default:
        return ERR_BADPARAM;
    }
    if (lvl == GPIO_HIGH) {
        SETFIELD(periph->ODR, 1UL, shift);
    } else if (lvl == GPIO_LOW) {
        CLEARFIELD(periph->ODR, 1UL, shift);
    } else {
        return ERR_BADPARAM;
    }
    return SYS_OK;
}

/**
 * Read the digital voltage level from a pin
 * @param port: GPIO Port pin is on
 * @param pin: pin to set
 * @return GPIO pin level
 */
GPIO_level_t GPIO_read(GPIO_port_t port, GPIO_pin_t pin) {
    // Begin by converting port into base register
    GPIO_TypeDef *periph;
    switch (port) {
    case GPIO_PORT_A:
        periph = GPIOA;
        break;
    case GPIO_PORT_B:
        periph = GPIOB;
        break;
    case GPIO_PORT_C:
        periph = GPIOC;
        break;
    case GPIO_PORT_D:
        periph = GPIOD;
        break;
    case GPIO_PORT_E:
        periph = GPIOE;
        break;
    case GPIO_PORT_H:
        periph = GPIOH;
        break;
    default:
        return 0;
    }
    return READFIELD(periph->IDR, 1UL, pin);
}
