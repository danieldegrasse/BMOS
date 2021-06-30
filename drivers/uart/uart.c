/**
 * @file uart.c
 * Implements UART and LPUART support for STM32L4xxxx
 */
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <device/device.h>
#include <sys/clock.h>
#include <sys/err.h>
#include <sys/isr.h>
#include <util/logging.h>
#include <util/ringbuf.h>
#include <util/util.h>

#include "uart.h"

/**
 * UART device state
 */
typedef enum {
    UART_dev_closed = 0,
    UART_dev_open = 1,
} UART_state_t;

/**
 * Configuration structure for UART devices
 */
typedef struct {
    UART_config_t cfg;       /*!< User configuration for UART */
    USART_TypeDef *regs;     /*!< Register access for this UART */
    UART_state_t state;      /*!< UART state (open or closed) */
    bool tx_active;          /*!< Is UART transmission active */
    RingBuf_t write_buf;     /*!< UART write ring buffer (outgoing data)*/
    RingBuf_t read_buf;      /*!< UART read ring buffer (incoming data)*/
    UART_periph_t periph_id; /*!< Identifies the peripheral handle references */
} UART_status_t;

#define UART_RINGBUF_SIZE 80

static UART_status_t UARTS[NUM_UARTS] = {0};
static uint8_t UART_RBUFFS[NUM_UARTS][UART_RINGBUF_SIZE];
static uint8_t UART_WBUFFS[NUM_UARTS][UART_RINGBUF_SIZE];

static void UART_interrupt(UART_periph_t source);
static void UART_transmit(UART_status_t *handle);
static int UART_bufwrite(UART_status_t *uart, uint8_t *buf, int len);
static syserr_t UART_set_wordlen(UART_status_t *handle, UART_wordlen_t wlen);
static syserr_t UART_set_stopbits(UART_status_t *handle, UART_stopbit_t sbit);
static syserr_t UART_set_parity(UART_status_t *handle, UART_parity_t parity);
static syserr_t UART_enable_periph(UART_status_t *handle, UART_periph_t periph);
static syserr_t UART_set_pinswap(UART_status_t *handle, UART_pinswap_t swap);
static syserr_t UART_set_msb(UART_status_t *handle, UART_bitorder_t bitorder);
static syserr_t UART_set_flowcontrol(UART_status_t *dev, UART_flow_control_t f);
static syserr_t UART_set_baudrate(UART_status_t *handle, UART_baud_rate_t baud);
static syserr_t UART_start_tx(UART_status_t *handle);

/**
 * Opens a UART or LPUART device for read/write access
 * @param periph: Identifier of UART to open
 * @param config: UART configuration structure
 * @param err: Set on function error
 * @return NULL on error, or a UART handle to the open peripheral
 */
UART_handle_t UART_open(UART_periph_t periph, UART_config_t *config,
                        syserr_t *err) {
    UART_status_t *handle;
    *err = SYS_OK; // Set no error until one occurs
    /**
     * Check parameters.
     */
    if (periph > USART_3) {
        *err = ERR_BADPARAM;
        return NULL;
    }
    handle = &UARTS[periph];
    if (handle->state == UART_dev_open) {
        *err = ERR_INUSE;
        return NULL;
    }
    // Set handle state to open
    handle->state = UART_dev_open;
    handle->tx_active = false;
    memcpy(&handle->cfg, config, sizeof(UART_config_t));
    // Setup read and write buffers
    buf_init(&handle->read_buf, UART_RBUFFS[periph], UART_RINGBUF_SIZE);
    buf_init(&handle->write_buf, UART_WBUFFS[periph], UART_RINGBUF_SIZE);
    /**
     * Record the UART peripheral address into the config structure
     * Here we also enable the clock for the relevant UART device,
     * as well as the interrupt
     */
    *err = UART_enable_periph(handle, periph);
    if (*err != SYS_OK) {
        UART_close(handle);
        return NULL;
    }
    /**
     * Configure the UART module according to the UART config provided
     * Register description can be found at p.1238 of datasheet
     */
    /* Configure the number of bits (wordlen) */
    *err = UART_set_wordlen(handle, config->UART_wordlen);
    if (*err != SYS_OK) {
        UART_close(handle);
        return NULL;
    }
    /* Configure the number of stop bits */
    *err = UART_set_stopbits(handle, config->UART_stopbit);
    if (*err != SYS_OK) {
        UART_close(handle);
        return NULL;
    }
    /* Configure the parity setting */
    *err = UART_set_parity(handle, config->UART_parity);
    if (*err != SYS_OK) {
        UART_close(handle);
        return NULL;
    }
    /* Configure pinswap (swaps TX and RX pins) */
    *err = UART_set_pinswap(handle, config->UART_pin_swap);
    if (*err != SYS_OK) {
        UART_close(handle);
        return NULL;
    }
    /* Select between UART MSB and LSB */
    *err = UART_set_msb(handle, config->UART_bit_order);
    if (*err != SYS_OK) {
        UART_close(handle);
        return NULL;
    }
    /* Configure UART flow control */
    *err = UART_set_flowcontrol(handle, config->UART_flowcontrol);
    if (*err != SYS_OK) {
        UART_close(handle);
        return NULL;
    }
    /* Configure UART baud rate */
    *err = UART_set_baudrate(handle, config->UART_baud_rate);
    if (*err != SYS_OK) {
        UART_close(handle);
        return NULL;
    }

    // Now, enable the UART
    SETBITS(handle->regs->CR1, USART_CR1_UE);
    // If auto bauding is enabled, here we need to request it
    if (handle->cfg.UART_baud_rate == UART_baud_auto) {
        SETBITS(handle->regs->CR2, USART_CR2_ABREN);
    }
    // Enable the transmitter and receiver
    SETBITS(handle->regs->CR1, USART_CR1_RE);
    // Register interrupt handler
    set_UART_isr(UART_interrupt);
    // Enable transmit complete and receive interrupts
    SETBITS(handle->regs->CR1, USART_CR1_RXNEIE);
    SETBITS(handle->regs->CR1, USART_CR1_TCIE);
    return handle;
}

/**
 * Reads data from a UART or LPUART device
 * @param handle: UART handle to access
 * @param buf: Buffer to read data into
 * @param len: buffer length
 * @param err: Set on error
 * @return number of bytes read, or -1 on error
 */
int UART_read(UART_handle_t handle, uint8_t *buf, uint32_t len, syserr_t *err) {
    int num_read, timeout;
    // Verify inputs
    if (handle == NULL || buf == NULL) {
        *err = ERR_BADPARAM;
        return -1;
    }
    /**
     * First, we must disable interrupts. If we don't do so, then we may get
     * inconsistent data from the ring buffer
     */
    mask_irq();
    // Now, attempt to read data from the input ring buffer
    num_read = buf_readblock(&(((UART_status_t *)handle)->read_buf), buf, len);
    // reenable interrupts
    unmask_irq();
    timeout = (int)((UART_status_t *)handle)->cfg.UART_read_timeout;
    while (num_read < len && timeout != UART_TIMEOUT_NONE) {
        /**
         * Wait for data to be available. For now, we will simply poll the
         * ringbuffer's size
         */
        while (buf_getsize(&(((UART_status_t *)handle)->read_buf)) == 0 &&
               timeout != UART_TIMEOUT_NONE) {
            // If the timeout is infinite, spin here until there is data to read
            if (timeout != UART_TIMEOUT_INF) {
                // Decrement timeout
                delay_ms(200);
                timeout -= 200;
                if (timeout - 200 < UART_TIMEOUT_NONE) {
                    // Just set timeout to NONE (defined to be zero)
                    timeout = UART_TIMEOUT_NONE;
                } else {
                    timeout -= 200;
                }
            }
        }
        mask_irq();
        // Now, there is data available in the buffer. Read it.
        num_read += buf_readblock(&(((UART_status_t *)handle)->read_buf),
                                  buf + num_read, len - num_read);
        unmask_irq();
    }
    return num_read;
}

/**
 * Writes data to a UART or LPUART device
 * @param handle: UART handle to access
 * @param buf: buffer to write data from
 * @param len: buffer length
 * @param err: set on error
 * @return number of bytes written, or -1 on error
 */
int UART_write(UART_handle_t handle, uint8_t *buf, uint32_t len,
               syserr_t *err) {
    int num_written, write_len, timeout, remaining_writes;
    UART_status_t *uart = (UART_status_t *)handle;
    // Verify inputs
    if (handle == NULL || buf == NULL) {
        *err = ERR_BADPARAM;
        return -1;
    }
    if (len == 0) {
        return len;
    }
    *err = SYS_OK;
    remaining_writes = len;
    /**
     * First, disable interrupts so we do not place bad data into the output
     * ring buffer
     */
    mask_irq();
    // Now write data to the ring buffer
    num_written = UART_bufwrite(uart, buf, remaining_writes);
    unmask_irq();
    // Advance the offset of buffer, and decrease len
    buf += num_written;
    remaining_writes -= num_written;
    // Start transmission for the UART
    *err = UART_start_tx(uart);
    if (*err != SYS_OK) {
        // UART is likely already in use for transmission
        return -1;
    }
    timeout = uart->cfg.UART_write_timeout;
    while (num_written < len && timeout != UART_TIMEOUT_NONE) {
        // Wait for there to be space in the ringbuffer
        while (buf_getsize(&(uart->write_buf)) == UART_RINGBUF_SIZE &&
               timeout != UART_TIMEOUT_NONE) {
            // If the timeout is set to infinity, we should just spin here
            if (timeout != UART_TIMEOUT_INF) {
                // Decrement the timeout
                delay_ms(1);
                timeout--;
            }
        }
        // There is space to write data. Write it.
        mask_irq();
        write_len = UART_bufwrite(uart, buf, remaining_writes);
        unmask_irq();
        num_written += write_len;
        buf += write_len;
        remaining_writes -= write_len;
    }
    /**
     * Now wait for all data to be sent. TC interrupt will clear tx_active flag
     * when transmission is done.
     */
    while (timeout != UART_TIMEOUT_NONE && uart->tx_active) {
        if (timeout != UART_TIMEOUT_INF) {
            delay_ms(200);
            if (timeout - 200 < UART_TIMEOUT_NONE) {
                // Just set timeout to NONE (defined to be zero)
                timeout = UART_TIMEOUT_NONE;
            } else {
                timeout -= 200;
            }
        }
    }
    return num_written;
}

/**
 * Closes a UART or LPUART device
 * @param handle: Handle to open uart device
 * @return SYS_OK on success, or error value otherwise
 */
syserr_t UART_close(UART_handle_t handle) {
    UART_status_t *uart = (UART_status_t *)handle;
    if (uart->state != UART_dev_open) {
        return ERR_BADPARAM;
    }
    switch (uart->periph_id) {
    case LPUART_1:
        // Reset peripheral by toggling reset bit
        SETBITS(RCC->APB1RSTR2, RCC_APB1RSTR2_LPUART1RST);
        CLEARBITS(RCC->APB1RSTR2, RCC_APB1RSTR2_LPUART1RST);
        CLEARBITS(RCC->APB1ENR2, RCC_APB1ENR2_LPUART1EN); // Disable peripheral
        disable_irq(LPUART1_IRQn); // Disable interrupts for this device
        break;
    case USART_1:
        // Reset peripheral by toggling reset bit
        SETBITS(RCC->APB2RSTR, RCC_APB2RSTR_USART1RST);
        CLEARBITS(RCC->APB2RSTR, RCC_APB2RSTR_USART1RST);
        CLEARBITS(RCC->APB2ENR, RCC_APB2ENR_USART1EN); // Disable peripheral
        disable_irq(USART1_IRQn); // Disable interrupts for this device
        break;
    case USART_2:
        // Reset peripheral by toggling reset bit
        SETBITS(RCC->APB1RSTR1, RCC_APB1RSTR1_USART2RST);
        CLEARBITS(RCC->APB1RSTR1, RCC_APB1RSTR1_USART2RST);
        CLEARBITS(RCC->APB1ENR1, RCC_APB1ENR1_USART2EN); // Disable peripheral
        disable_irq(USART2_IRQn); // Disable interrupts for this device
        break;
    case USART_3:
        // Reset peripheral by toggling reset bit
        SETBITS(RCC->APB1RSTR1, RCC_APB1RSTR1_USART3RST);
        CLEARBITS(RCC->APB1RSTR1, RCC_APB1RSTR1_USART2RST);
        CLEARBITS(RCC->APB1ENR1, RCC_APB1ENR1_USART3EN); // Disable peripheral
        disable_irq(USART3_IRQn); // Disable interrupts for this device
        break;
    default:
        return ERR_BADPARAM;
        break;
    }
    // Close UART device
    uart->state = UART_dev_closed;
    return SYS_OK;
}

/**
 * Starts UART transmission. This function should only be called once data
 * is stored in the UART write buffer, to avoid constantly getting a TX
 * interrupt. The TC interrupt handler will disable TX interrupts once the
 * write buffer is drained.
 *
 * @param handle: UART handle to enable tx on
 * @return SYS_OK on success, or error on failure
 */
static syserr_t UART_start_tx(UART_status_t *handle) {
    if (handle->tx_active &&
        handle->cfg.UART_write_timeout != UART_TIMEOUT_INF) {
        // Device is already transmitting. Block this attempt to transmit
        return ERR_INUSE;
    } else {
        // Busy wait for the uart to be available for transmission
        while (handle->tx_active)
            ;
    }
    // Enable interrupts for this UART device, and set TX as active
    handle->tx_active = true;
    SETBITS(handle->regs->CR1, USART_CR1_TE);
    SETBITS(handle->regs->CR1, USART_CR1_TXEIE);
    return SYS_OK;
}

/**
 * Transmits data on the UART device provided. Reads data from the device's
 * ring buffer
 * @param handle: UART device to send data from ringbuffer on
 */
static void UART_transmit(UART_status_t *handle) {
    char data;
    // Read a byte from the ring buffer and send it
    if (buf_read(&(handle->write_buf), &data) == SYS_OK) {
        // Send by writing to the TDR register
        handle->regs->TDR = USART_TDR_TDR & data;
    }
}

/**
 * Handles UART interrupts
 * @param source: UART device generating interrupt
 */
static void UART_interrupt(UART_periph_t source) {
    char data;
    /**
     * Use the source as an index into the handle structures,
     * to find the UART that generated the interrupt
     */
    UART_status_t *handle = &UARTS[source];
    /**
     * Now determine what flag caused the interrupt. We need to check for
     * the TXE and RXNE bits
     */
    if (READBITS(handle->regs->ISR, USART_ISR_RXNE)) {
        // We have data in the RX buffer. Read it to clear the flag.
        data = READBITS(handle->regs->RDR, USART_RDR_RDR);
        if (handle->cfg.UART_textmode == UART_txtmode_en && data == '\r') {
            // Transparently replace the \r with a \n
            data = '\n';
        }
        // Store the data
        if (buf_write(&(handle->read_buf), data) != SYS_OK) {
            LOG_D("Dropping character from UART");
            // Write 1 to RXFRQ to drop the data
            SETBITS(handle->regs->RQR, USART_RQR_RXFRQ);
        }
    }
    if (READBITS(handle->regs->ISR, USART_ISR_TC)) {
        // Transmission is complete. Check if write buffer is empty.
        if (buf_getsize(&(handle->write_buf)) == 0) {
            /**
             * Tranmission is no longer active. Wait for the TC bit to be set,
             * then clear it By waiting here, we ensure the UART is done
             * transmitting data.
             */
            while (READBITS(handle->regs->ISR, USART_ISR_TC) == 0) {
            }
            handle->tx_active = false;
            // Disable the transmitter
            CLEARBITS(handle->regs->CR1, USART_CR1_TXEIE);
            CLEARBITS(handle->regs->CR1, USART_CR1_TE);
            // Clear the TC interrupt
            SETBITS(handle->regs->ICR, USART_ICR_TCCF);
        }
    }
    if (READBITS(handle->regs->ISR, USART_ISR_TXE) && handle->tx_active) {
        /**
         * Transmit data register is empty and ready for a new char.
         */
        UART_transmit(handle);
    }
}

/**
 * Writes data to a UART's output buffer, until the buffer is full or the
 * provided buffer is entirely written. returns the number of bytes written.
 * @param uart: UART handle to write to the output buffer of
 * @param buf: data to write to output buffer.
 * @param len: length of data to write
 */
static int UART_bufwrite(UART_status_t *uart, uint8_t *buf, int len) {
    int rd_idx, num_written;
    syserr_t ret;
    num_written = 0;
    if (uart->cfg.UART_textmode == UART_txtmode_en) {
        /**
         * As we write data into the buffer, replace '\n' with "\r\n"
         * we have to iterate through each character to accomplish this
         */
        rd_idx = 0;
        while (rd_idx < len) {
            if (buf[rd_idx] == '\n') {
                // Ensure buffer has space for two characters
                if (buf_getspace(&(uart->write_buf)) < 2) {
                    // We cannot write /r/n, break out of write loop
                    break;
                }
                // The buffer has space. Write CR and LF.
                buf_writeblock(&(uart->write_buf), (uint8_t *)"\r\n", 2);
            } else {
                ret = buf_write(&(uart->write_buf), *(buf + rd_idx));
                if (ret != SYS_OK) {
                    // Buffer is full, break out of write loop
                    break;
                }
            }
            // Increment number of bytes written and read index
            num_written++;
            rd_idx++;
        }
    } else {
        num_written = buf_writeblock(&(uart->write_buf), buf, len);
    }
    return num_written;
}

/**
 * Selects a UART peripheral and enables it.
 * Also records the register value for the selected UART into the
 * "regs" field of handle
 * @param handle: UART handle to open peripheral with
 * @param periph: UART peripheral ID to enable
 */
static syserr_t UART_enable_periph(UART_status_t *handle,
                                   UART_periph_t periph) {
    switch (periph) {
    case LPUART_1:
        SETBITS(RCC->APB1ENR2, RCC_APB1ENR2_LPUART1EN); // Enable peripheral
        // Reset peripheral by toggling reset bit
        SETBITS(RCC->APB1RSTR2, RCC_APB1RSTR2_LPUART1RST);
        CLEARBITS(RCC->APB1RSTR2, RCC_APB1RSTR2_LPUART1RST);
        enable_irq(LPUART1_IRQn); // Enable interrupts for this device
        handle->regs = LPUART1;
        break;
    case USART_1:
        SETBITS(RCC->APB2ENR, RCC_APB2ENR_USART1EN); // Enable peripheral
        // Reset peripheral by toggling reset bit
        SETBITS(RCC->APB2RSTR, RCC_APB2RSTR_USART1RST);
        CLEARBITS(RCC->APB2RSTR, RCC_APB2RSTR_USART1RST);
        enable_irq(USART1_IRQn); // Enable interrupts for this device
        handle->regs = USART1;
        break;
    case USART_2:
        SETBITS(RCC->APB1ENR1, RCC_APB1ENR1_USART2EN); // Enable peripheral
        // Reset peripheral by toggling reset bit
        SETBITS(RCC->APB1RSTR1, RCC_APB1RSTR1_USART2RST);
        CLEARBITS(RCC->APB1RSTR1, RCC_APB1RSTR1_USART2RST);
        enable_irq(USART2_IRQn); // Enable interrupts for this device
        handle->regs = USART2;
        break;
    case USART_3:
        SETBITS(RCC->APB1ENR1, RCC_APB1ENR1_USART3EN); // Enable peripheral
        // Reset peripheral by toggling reset bit
        SETBITS(RCC->APB1RSTR1, RCC_APB1RSTR1_USART3RST);
        CLEARBITS(RCC->APB1RSTR1, RCC_APB1RSTR1_USART2RST);
        enable_irq(USART3_IRQn); // Enable interrupts for this device
        handle->regs = USART3;
        break;
    default:
        return ERR_BADPARAM;
        break;
    }
    // Record peripheral ID in handler
    handle->periph_id = periph;
    return SYS_OK;
}

/**
 * Configures UART world length
 * @param handle: UART device handle to configure
 * @param config: UART config structure
 * @return SYS_OK on success, or error value on failuer
 */
static syserr_t UART_set_wordlen(UART_status_t *handle, UART_wordlen_t wlen) {
    switch (wlen) {
    case UART_word_7n1:
        MODIFY_REG(handle->regs->CR1, USART_CR1_M, USART_CR1_M1);
        break;
    case UART_word_8n1:
        // Bits M0 and M1 should both be zero
        CLEARBITS(handle->regs->CR1, USART_CR1_M_Msk);
        break;
    case UART_word_9n1:
        MODIFY_REG(handle->regs->CR1, USART_CR1_M, USART_CR1_M0);
        break;
    default:
        return ERR_BADPARAM;
        break;
    }
    return SYS_OK;
}

/**
 * Sets the number of stop bits for a UART device
 * @param handle: UART handle
 * @param sbit: number of stop bits
 * @return SYS_OK on success, or error value on error
 */
static syserr_t UART_set_stopbits(UART_status_t *handle, UART_stopbit_t sbit) {
    switch (sbit) {
    case UART_onestop:
        // Bitfield of 0b00 sets one stop bit
        CLEARBITS(handle->regs->CR2, USART_CR2_STOP_Msk);
        break;
    case UART_twostop:
        MODIFY_REG(handle->regs->CR2, USART_CR2_STOP, USART_CR2_STOP_1);
        break;
    default:
        return ERR_BADPARAM;
        break;
    }
    return SYS_OK;
}

/**
 * Sets UART device's parity bit count
 * @param handle: UART handle
 * @param parity: number of parity bits
 * @return SYS_OK on success, or error value on error
 */
static syserr_t UART_set_parity(UART_status_t *handle, UART_parity_t parity) {
    switch (parity) {
    case UART_parity_disabled:
        // Ensure PCE bit is cleared
        CLEARBITS(handle->regs->CR1, USART_CR1_PCE);
        break;
    case UART_parity_even:
        // Set PCE bit and clear PS bit
        SETBITS(handle->regs->CR1, USART_CR1_PCE);
        CLEARBITS(handle->regs->CR1, USART_CR1_PS);
        break;
    case UART_parity_odd:
        // Set PCE and PS bits
        SETBITS(handle->regs->CR1, USART_CR1_PS | USART_CR1_PCE);
        break;
    default:
        return ERR_BADPARAM;
        break;
    }
    return SYS_OK;
}

/**
 * Enables or disables pinswap for a UART device
 * @param handle: UART device to manipulate
 * @param swap: should swap be enabled or disabled
 * @return SYS_OK on success, or error on error
 */
static syserr_t UART_set_pinswap(UART_status_t *handle, UART_pinswap_t swap) {
    switch (swap) {
    case UART_pin_normal:
        CLEARBITS(handle->regs->CR2, USART_CR2_SWAP);
        break;
    case UART_pin_swapped:
        SETBITS(handle->regs->CR2, USART_CR2_SWAP);
        break;
    default:
        return ERR_BADPARAM;
        break;
    }
    return SYS_OK;
}

/**
 * Sets UART bit order
 * @param handle: UART device to manipulate
 * @param bitorder: sets bit order to msb first or lsb first
 * @return SYS_OK on success, or error on error
 */
static syserr_t UART_set_msb(UART_status_t *handle, UART_bitorder_t bitorder) {
    switch (bitorder) {
    case UART_lsb_first:
        CLEARBITS(handle->regs->CR2, USART_CR2_MSBFIRST);
        break;
    case UART_msb_first:
        SETBITS(handle->regs->CR2, USART_CR2_MSBFIRST);
        break;
    default:
        return ERR_BADPARAM;
        break;
    }
    return SYS_OK;
}

/**
 * Sets UART flow control status (RTS/CTS pins)
 * @param dev: UART device to manipulate
 * @param f: should flow control be enabled
 * @return SYS_OK on success, or error on error
 */
static syserr_t UART_set_flowcontrol(UART_status_t *dev,
                                     UART_flow_control_t f) {
    switch (f) {
    case UART_no_flow:
        // Just need to disable the relevant bit fields
        CLEARBITS(dev->regs->CR3, USART_CR3_CTSE | USART_CR3_RTSE);
        break;
    case UART_flow_control:
        // Enable the UART flow control bits
        SETBITS(dev->regs->CR3, USART_CR3_CTSE | USART_CR3_RTSE);
        break;
    default:
        return ERR_BADPARAM;
        break;
    }
    return SYS_OK;
}

/**
 * Sets UART baud rate
 * @param handle: UART device to manipulate
 * @param baud: baud rate to set
 * @return SYS_OK on success, or error on error
 */
static syserr_t UART_set_baudrate(UART_status_t *handle,
                                  UART_baud_rate_t baud) {
    uint32_t brr_val;
    uint64_t clk_freq;
    /**
     * Baud rate configuration. See p.1210 for baud rate formula. For
     * 16x oversampling (default) LPUART1 will use 256*fck/LPUARTDIV.
     * USARTx will use fck/USARTDIV. Default clock source for UART devices is
     * PCLK but can be changed using the RCC_CCIPR register
     *
     * Values used below are taken from datasheet pg.1274
     */
    if (handle->periph_id == LPUART_1) {
        clk_freq = pclk1_freq();
        switch (baud) {
        case UART_baud_1200:
            brr_val = (256UL * clk_freq) / 1200;
            break;
        case UART_baud_2400:
            brr_val = (256UL * clk_freq) / 2400;
            break;
        case UART_baud_4800:
            brr_val = (256UL * clk_freq) / 4800;
            break;
        case UART_baud_9600:
            brr_val = (256UL * clk_freq) / 9600;
            break;
        case UART_baud_19200:
            brr_val = (256UL * clk_freq) / 19200;
            break;
        case UART_baud_38400:
            brr_val = (256UL * clk_freq) / 38400;
            break;
        case UART_baud_57600:
            brr_val = (256UL * clk_freq) / 57600;
            break;
        case UART_baud_115200:
            brr_val = (256UL * clk_freq) / 115200;
            break;
        default:
            return ERR_BADPARAM;
            break;
        }
        /**
         * Verify that the brr value results in a baud rate
         * between fck/3 and fck/4096, and that it is greater than 0x300
         * These requirements are only enforced for the LPUART BRR register,
         * not for the rest of the USARTS
         */
        if (brr_val < 0x300 || baud < (clk_freq >> 12) ||
            baud > (clk_freq / 3)) {
            return ERR_BADPARAM;
        }
    } else {
        // Switch on the UART device to determine the clock source
        switch (handle->periph_id) {
        case USART_1:
            clk_freq = pclk2_freq(); // Sources from APB2
            break;
        case USART_2:
            clk_freq = pclk1_freq(); // Sources from APB1
            break;
        case USART_3:
            clk_freq = pclk1_freq(); // Sources form APB1
            break;
        default:
            return ERR_BADPARAM;
            break;
        }
        // For all other UARTS, use the standard baud rate equation
        switch (baud) {
        case UART_baud_auto:
            /* Special case. Write a starter value to the BRR register */
            brr_val = clk_freq / 115200; // 115200 baud
            break;
        case UART_baud_1200:
            brr_val = clk_freq / 1200;
            break;
        case UART_baud_2400:
            brr_val = clk_freq / 2400;
            break;
        case UART_baud_4800:
            brr_val = clk_freq / 4800;
            break;
        case UART_baud_9600:
            brr_val = clk_freq / 9600;
            break;
        case UART_baud_19200:
            brr_val = clk_freq / 19200;
            break;
        case UART_baud_38400:
            brr_val = clk_freq / 38400;
            break;
        case UART_baud_57600:
            brr_val = clk_freq / 57600;
            break;
        case UART_baud_115200:
            brr_val = clk_freq / 115200;
            break;
        default:
            return ERR_BADPARAM;
            break;
        }
    }
    // Set UART BRR value
    handle->regs->BRR = brr_val;
    return SYS_OK;
}