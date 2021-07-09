#include <stdint.h>

#include "gpio.h"
#include <drivers/device/device.h>
#include <sys/isr/isr.h>
#include <util/bitmask.h>

/** GPIO interrupt handler array */
static void (*gpio_interrupt_handlers[16])(void) = {0};

// Static functions
static void GPIO_isr(void);

/**
 * Configure a GPIO port for use with driver
 * @param pin: GPIO pin to configure
 * @param config: GPIO configuration structure
 */
syserr_t GPIO_config(GPIO_pin_t pin, GPIO_config_t *config) {
    // Begin by converting port into base register
    GPIO_TypeDef *periph;
    __IO uint32_t *af_sel;
    uint32_t port = pin & PORTMASK;
    pin &= PINMASK;            // Mask out the port portion of pin defintion
    uint32_t shift = pin << 1; // 2 times value of pin
    /**
     * In this switch statement, we will also verify that the relevant
     * AHB2 peripheral clock is enabled for the selected port, since otherwise
     * registers will not accept writes
     */
    switch (port) {
    case PORT_A:
        // Enable GPIOA
        SETBITS(RCC->AHB2ENR, RCC_AHB2ENR_GPIOAEN);
        periph = GPIOA;
        break;
    case PORT_B:
        SETBITS(RCC->AHB2ENR, RCC_AHB2ENR_GPIOBEN);
        periph = GPIOB;
        break;
    case PORT_C:
        SETBITS(RCC->AHB2ENR, RCC_AHB2ENR_GPIOCEN);
        periph = GPIOC;
        break;
    case PORT_D:
        SETBITS(RCC->AHB2ENR, RCC_AHB2ENR_GPIODEN);
        periph = GPIOD;
        break;
    case PORT_E:
        SETBITS(RCC->AHB2ENR, RCC_AHB2ENR_GPIOEEN);
        periph = GPIOE;
        break;
    case PORT_H:
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
 * @param pin: pin to set
 * @param lvl: GPIO level to set
 */
syserr_t GPIO_write(GPIO_pin_t pin, GPIO_level_t lvl) {
    // Begin by converting port into base register
    GPIO_TypeDef *periph;
    uint32_t port = pin & PORTMASK;
    pin &= PINMASK; // Mask out port definition from pin
    uint32_t shift = pin;
    switch (port) {
    case PORT_A:
        periph = GPIOA;
        break;
    case PORT_B:
        periph = GPIOB;
        break;
    case PORT_C:
        periph = GPIOC;
        break;
    case PORT_D:
        periph = GPIOD;
        break;
    case PORT_E:
        periph = GPIOE;
        break;
    case PORT_H:
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
 * @param pin : pin to set
 * @ return GPIO pin level
 */
GPIO_level_t GPIO_read(GPIO_pin_t pin) {
    // Begin by converting port into base register
    uint32_t port = pin & PORTMASK;
    pin &= PINMASK; // Mask out port from pin
    GPIO_TypeDef *periph;
    switch (port) {
    case PORT_A:
        periph = GPIOA;
        break;
    case PORT_B:
        periph = GPIOB;
        break;
    case PORT_C:
        periph = GPIOC;
        break;
    case PORT_D:
        periph = GPIOD;
        break;
    case PORT_E:
        periph = GPIOE;
        break;
    case PORT_H:
        periph = GPIOH;
        break;
    default:
        return 0;
    }
    if (READFIELD(periph->IDR, 1UL, pin)) {
        return GPIO_HIGH;
    } else {
        return GPIO_LOW;
    }
}

/**
 * Enable interrupts on a GPIO pin
 * @param pin: pin to enable interrupts on
 * @param trigger: either GPIO_trig_rising, GPIO_trig_falling, or GPIO_trig_both
 * @param callback: callback to run. This function will be called from an
 * interrupt context.
 * @return SYS_OK on success, or ERR_INUSE if another GPIO pin is using the
 * interrupt line (GPIO pins are multipled accross 16 lines)
 */
syserr_t GPIO_interrupt_enable(GPIO_pin_t pin, GPIO_trigger_t trigger,
                               void (*callback)(void)) {
    /**
     * The pin number is all that really matters for the EXTI controller,
     * as gpio pins are multiplexed by pin number (PA1, PB1, PC1, PD1, and PE1
     * are grouped for example)
     */
    uint32_t port = pin & PORTMASK;
    uint32_t pin_value = pin & PINMASK;
    uint32_t interrupt_vect;
    uint32_t mask, regidx, value;
    switch (pin_value) {
    case PIN_0:
        mask = SYSCFG_EXTICR1_EXTI0_Msk;
        interrupt_vect = EXTI0_IRQn;
        regidx = 0;
        switch (port) {
        case PORT_A:
            value = SYSCFG_EXTICR1_EXTI0_PA;
            break;
        case PORT_B:
            value = SYSCFG_EXTICR1_EXTI0_PB;
            break;
        case PORT_C:
            value = SYSCFG_EXTICR1_EXTI0_PC;
            break;
        case PORT_D:
            value = SYSCFG_EXTICR1_EXTI0_PD;
            break;
        case PORT_E:
            value = SYSCFG_EXTICR1_EXTI0_PE;
            break;
        case PORT_H:
            value = SYSCFG_EXTICR1_EXTI0_PH;
            break;
        }
        break;
    case PIN_1:
        mask = SYSCFG_EXTICR1_EXTI1_Msk;
        interrupt_vect = EXTI1_IRQn;
        regidx = 0;
        switch (port) {
        case PORT_A:
            value = SYSCFG_EXTICR1_EXTI1_PA;
            break;
        case PORT_B:
            value = SYSCFG_EXTICR1_EXTI1_PB;
            break;
        case PORT_C:
            value = SYSCFG_EXTICR1_EXTI1_PC;
            break;
        case PORT_D:
            value = SYSCFG_EXTICR1_EXTI1_PD;
            break;
        case PORT_E:
            value = SYSCFG_EXTICR1_EXTI1_PE;
            break;
        case PORT_H:
            value = SYSCFG_EXTICR1_EXTI1_PH;
            break;
        }
        break;
    case PIN_2:
        mask = SYSCFG_EXTICR1_EXTI2_Msk;
        interrupt_vect = EXTI2_IRQn;
        regidx = 0;
        switch (port) {
        case PORT_A:
            value = SYSCFG_EXTICR1_EXTI2_PA;
            break;
        case PORT_B:
            value = SYSCFG_EXTICR1_EXTI2_PB;
            break;
        case PORT_C:
            value = SYSCFG_EXTICR1_EXTI2_PC;
            break;
        case PORT_D:
            value = SYSCFG_EXTICR1_EXTI2_PD;
            break;
        case PORT_E:
            value = SYSCFG_EXTICR1_EXTI2_PE;
            break;
        }
        break;
    case PIN_3:
        mask = SYSCFG_EXTICR1_EXTI3_Msk;
        interrupt_vect = EXTI3_IRQn;
        regidx = 0;
        switch (port) {
        case PORT_A:
            value = SYSCFG_EXTICR1_EXTI3_PA;
            break;
        case PORT_B:
            value = SYSCFG_EXTICR1_EXTI3_PB;
            break;
        case PORT_C:
            value = SYSCFG_EXTICR1_EXTI3_PC;
            break;
        case PORT_D:
            value = SYSCFG_EXTICR1_EXTI3_PD;
            break;
        case PORT_E:
            value = SYSCFG_EXTICR1_EXTI3_PE;
            break;
        }
        break;
    case PIN_4:
        mask = SYSCFG_EXTICR2_EXTI4_Msk;
        interrupt_vect = EXTI4_IRQn;
        regidx = 1;
        switch (port) {
        case PORT_A:
            value = SYSCFG_EXTICR2_EXTI4_PA;
            break;
        case PORT_B:
            value = SYSCFG_EXTICR2_EXTI4_PB;
            break;
        case PORT_C:
            value = SYSCFG_EXTICR2_EXTI4_PC;
            break;
        case PORT_D:
            value = SYSCFG_EXTICR2_EXTI4_PD;
            break;
        case PORT_E:
            value = SYSCFG_EXTICR2_EXTI4_PE;
            break;
        }
        break;
    case PIN_5:
        mask = SYSCFG_EXTICR2_EXTI5_Msk;
        interrupt_vect = EXTI9_5_IRQn;
        regidx = 1;
        switch (port) {
        case PORT_A:
            value = SYSCFG_EXTICR2_EXTI5_PA;
            break;
        case PORT_B:
            value = SYSCFG_EXTICR2_EXTI5_PB;
            break;
        case PORT_C:
            value = SYSCFG_EXTICR2_EXTI5_PC;
            break;
        case PORT_D:
            value = SYSCFG_EXTICR2_EXTI5_PD;
            break;
        case PORT_E:
            value = SYSCFG_EXTICR2_EXTI5_PE;
            break;
        }
        break;
    case PIN_6:
        mask = SYSCFG_EXTICR2_EXTI6_Msk;
        interrupt_vect = EXTI9_5_IRQn;
        regidx = 1;
        switch (port) {
        case PORT_A:
            value = SYSCFG_EXTICR2_EXTI6_PA;
            break;
        case PORT_B:
            value = SYSCFG_EXTICR2_EXTI6_PB;
            break;
        case PORT_C:
            value = SYSCFG_EXTICR2_EXTI6_PC;
            break;
        case PORT_D:
            value = SYSCFG_EXTICR2_EXTI6_PD;
            break;
        case PORT_E:
            value = SYSCFG_EXTICR2_EXTI6_PE;
            break;
        }
        break;
    case PIN_7:
        mask = SYSCFG_EXTICR2_EXTI7_Msk;
        interrupt_vect = EXTI9_5_IRQn;
        regidx = 1;
        switch (port) {
        case PORT_A:
            value = SYSCFG_EXTICR2_EXTI7_PA;
            break;
        case PORT_B:
            value = SYSCFG_EXTICR2_EXTI7_PB;
            break;
        case PORT_C:
            value = SYSCFG_EXTICR2_EXTI7_PC;
            break;
        case PORT_D:
            value = SYSCFG_EXTICR2_EXTI7_PD;
            break;
        case PORT_E:
            value = SYSCFG_EXTICR2_EXTI7_PE;
            break;
        }
        break;
    case PIN_8:
        mask = SYSCFG_EXTICR3_EXTI8_Msk;
        interrupt_vect = EXTI9_5_IRQn;
        regidx = 2;
        switch (port) {
        case PORT_A:
            value = SYSCFG_EXTICR3_EXTI8_PA;
            break;
        case PORT_B:
            value = SYSCFG_EXTICR3_EXTI8_PB;
            break;
        case PORT_C:
            value = SYSCFG_EXTICR3_EXTI8_PC;
            break;
        case PORT_D:
            value = SYSCFG_EXTICR3_EXTI8_PD;
            break;
        case PORT_E:
            value = SYSCFG_EXTICR3_EXTI8_PE;
            break;
        }
        break;
    case PIN_9:
        mask = SYSCFG_EXTICR3_EXTI9_Msk;
        interrupt_vect = EXTI9_5_IRQn;
        regidx = 2;
        switch (port) {
        case PORT_A:
            value = SYSCFG_EXTICR3_EXTI9_PA;
            break;
        case PORT_B:
            value = SYSCFG_EXTICR3_EXTI9_PB;
            break;
        case PORT_C:
            value = SYSCFG_EXTICR3_EXTI9_PC;
            break;
        case PORT_D:
            value = SYSCFG_EXTICR3_EXTI9_PD;
            break;
        case PORT_E:
            value = SYSCFG_EXTICR3_EXTI9_PE;
            break;
        }
        break;
    case PIN_10:
        mask = SYSCFG_EXTICR3_EXTI10_Msk;
        interrupt_vect = EXTI15_10_IRQn;
        regidx = 2;
        switch (port) {
        case PORT_A:
            value = SYSCFG_EXTICR3_EXTI10_PA;
            break;
        case PORT_B:
            value = SYSCFG_EXTICR3_EXTI10_PB;
            break;
        case PORT_C:
            value = SYSCFG_EXTICR3_EXTI10_PC;
            break;
        case PORT_D:
            value = SYSCFG_EXTICR3_EXTI10_PD;
            break;
        case PORT_E:
            value = SYSCFG_EXTICR3_EXTI10_PE;
            break;
        }
        break;
    case PIN_11:
        mask = SYSCFG_EXTICR3_EXTI11_Msk;
        interrupt_vect = EXTI15_10_IRQn;
        regidx = 2;
        switch (port) {
        case PORT_A:
            value = SYSCFG_EXTICR3_EXTI11_PA;
            break;
        case PORT_B:
            value = SYSCFG_EXTICR3_EXTI11_PB;
            break;
        case PORT_C:
            value = SYSCFG_EXTICR3_EXTI11_PC;
            break;
        case PORT_D:
            value = SYSCFG_EXTICR3_EXTI11_PD;
            break;
        case PORT_E:
            value = SYSCFG_EXTICR3_EXTI11_PE;
            break;
        }
        break;
    case PIN_12:
        mask = SYSCFG_EXTICR4_EXTI12_Msk;
        interrupt_vect = EXTI15_10_IRQn;
        regidx = 3;
        switch (port) {
        case PORT_A:
            value = SYSCFG_EXTICR4_EXTI12_PA;
            break;
        case PORT_B:
            value = SYSCFG_EXTICR4_EXTI12_PB;
            break;
        case PORT_C:
            value = SYSCFG_EXTICR4_EXTI12_PC;
            break;
        case PORT_D:
            value = SYSCFG_EXTICR4_EXTI12_PD;
            break;
        case PORT_E:
            value = SYSCFG_EXTICR4_EXTI12_PE;
            break;
        }
        break;
    case PIN_13:
        mask = SYSCFG_EXTICR4_EXTI13_Msk;
        interrupt_vect = EXTI15_10_IRQn;
        regidx = 3;
        switch (port) {
        case PORT_A:
            value = SYSCFG_EXTICR4_EXTI13_PA;
            break;
        case PORT_B:
            value = SYSCFG_EXTICR4_EXTI13_PB;
            break;
        case PORT_C:
            value = SYSCFG_EXTICR4_EXTI13_PC;
            break;
        case PORT_D:
            value = SYSCFG_EXTICR4_EXTI13_PD;
            break;
        case PORT_E:
            value = SYSCFG_EXTICR4_EXTI13_PE;
            break;
        }
        break;
    case PIN_14:
        mask = SYSCFG_EXTICR4_EXTI14_Msk;
        interrupt_vect = EXTI15_10_IRQn;
        regidx = 3;
        switch (port) {
        case PORT_A:
            value = SYSCFG_EXTICR4_EXTI14_PA;
            break;
        case PORT_B:
            value = SYSCFG_EXTICR4_EXTI14_PB;
            break;
        case PORT_C:
            value = SYSCFG_EXTICR4_EXTI14_PC;
            break;
        case PORT_D:
            value = SYSCFG_EXTICR4_EXTI14_PD;
            break;
        case PORT_E:
            value = SYSCFG_EXTICR4_EXTI14_PE;
            break;
        }
        break;
    case PIN_15:
        mask = SYSCFG_EXTICR4_EXTI15_Msk;
        interrupt_vect = EXTI15_10_IRQn;
        regidx = 3;
        switch (port) {
        case PORT_A:
            value = SYSCFG_EXTICR4_EXTI15_PA;
            break;
        case PORT_B:
            value = SYSCFG_EXTICR4_EXTI15_PB;
            break;
        case PORT_C:
            value = SYSCFG_EXTICR4_EXTI15_PC;
            break;
        case PORT_D:
            value = SYSCFG_EXTICR4_EXTI15_PD;
            break;
        case PORT_E:
            value = SYSCFG_EXTICR4_EXTI15_PE;
            break;
        }
        break;
    }
    if (READBITS(SYSCFG->EXTICR[regidx], mask) != 0) {
        // EXTI in use
        return ERR_INUSE;
    } else {
        // Enable SYSCFG clock
        SETBITS(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN);
        // Enable EXTI
        MODIFY_REG(SYSCFG->EXTICR[regidx], mask, value);
        // Power down SYSCFG
        CLEARBITS(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN);
        // EXTI line number is same as pin. Unmask line interrupt.
        SETBITS(EXTI->IMR1, (0x1 << pin_value));
        if (trigger == GPIO_trig_both) {
            // Enable both rising and falling registers
            SETBITS(EXTI->RTSR1, (0x1 << pin_value));
            SETBITS(EXTI->FTSR1, (0x1 << pin_value));
        } else if (trigger == GPIO_trig_falling) {
            SETBITS(EXTI->FTSR1, (0x1 << pin_value));
        } else if (trigger == GPIO_trig_rising) {
            SETBITS(EXTI->RTSR1, (0x1 << pin_value));
        }
        // Save pointer to callback
        gpio_interrupt_handlers[pin_value] = callback;
        // Enable interrupt
        enable_irq(interrupt_vect, GPIO_isr);
    }
    return SYS_OK;
}

/**
 * GPIO ISR. Handles EXTI interrupts for lines 0-15
 */
static void GPIO_isr(void) {
    // Check the EXTI pending register to dermine which line(s) have interrupts
    uint32_t pending = READBITS(EXTI->PR1, 0xFFFF); // Only need lower 16 bits
    int i;
    for (i = 0; i < 16; i++) {
        if (pending & (0x1 << i)) {
            // Call interrupt handler
            gpio_interrupt_handlers[i]();
        }
    }
    // A write of 1 to an EXTI_PR bit clears it. Just write the 'pending' mask
    SETBITS(EXTI->PR1, pending);
}