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
    UART_config_t cfg;   /*!< User configuration for UART */
    USART_TypeDef *regs; /*!< Register access for this UART */
    UART_state_t state;  /*!< UART state (open or closed) */
    bool tx_active;      /*!< Is UART transmission active */
    RingBuf_t write_buf; /*!< UART write ring buffer (outgoing data)*/
    RingBuf_t read_buf;  /*!< UART read ring buffer (incoming data)*/
} UART_periph_status_t;

#define UART_RINGBUF_SIZE 80

static UART_periph_status_t UARTS[NUM_UARTS] = {0};
static uint8_t UART_RBUFFS[NUM_UARTS][UART_RINGBUF_SIZE];
static uint8_t UART_WBUFFS[NUM_UARTS][UART_RINGBUF_SIZE];

static void UART_interrupt(UART_periph_t source);
static void UART_transmit(UART_periph_status_t *handle);
static int UART_bufwrite(UART_periph_status_t *uart, uint8_t *buf, int len);
/**
 * Opens a UART or LPUART device for read/write access
 * @param periph: Identifier of UART to open
 * @param config: UART configuration structure
 * @param err: Set on function error
 * @return NULL on error, or a UART handle to the open peripheral
 */
UART_handle_t UART_open(UART_periph_t periph, UART_config_t *config,
                        syserr_t *err) {
    UART_periph_status_t *handle;
    *err = SYS_OK; // Set no error until one occurs
    /**
     * Check parameters. Note that due to limitations on the range of the
     * LPUART1_BRR register, LPUART1 cannot support low baud rates
     * without switching its clock source to LSE or HSI16
     */
    if (periph == LPUART_1 && config->UART_baud_rate < UART_baud_38400) {
        *err = ERR_NOSUPPORT;
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
    switch (periph) {
    case LPUART_1:
        SETBITS(RCC->APB1ENR2, RCC_APB1ENR2_LPUART1EN);
        enable_irq(LPUART1_IRQn);
        handle->regs = LPUART1;
        break;
    case USART_1:
        SETBITS(RCC->APB2ENR, RCC_APB2ENR_USART1EN);
        enable_irq(USART1_IRQn);
        handle->regs = USART1;
        break;
    case USART_2:
        SETBITS(RCC->APB1ENR1, RCC_APB1ENR1_USART2EN);
        enable_irq(USART2_IRQn);
        handle->regs = USART2;
        break;
    case USART_3:
        SETBITS(RCC->APB1ENR1, RCC_APB1ENR1_USART3EN);
        enable_irq(USART3_IRQn);
        handle->regs = USART3;
        break;
    default:
        *err = ERR_BADPARAM;
        return NULL;
        break;
    }
    /**
     * Configure the UART module according to the UART config provided
     * Register description can be found at p.1238 of datasheet
     */
    /* Configure the number of bits */
    CLEARBITS(handle->regs->CR1, USART_CR1_M_Msk);
    switch (handle->cfg.UART_wordlen) {
    case UART_word_7n1:
        SETBITS(handle->regs->CR1, USART_CR1_M1);
        break;
    case UART_word_8n1:
        // Bit M0 and M1 should be zero, and they have been cleared
        break;
    case UART_word_9n1:
        SETBITS(handle->regs->CR1, USART_CR1_M0);
        break;
    default:
        *err = ERR_BADPARAM;
        return NULL;
        break;
    }
    /* Configure the number of stop bits */
    CLEARBITS(handle->regs->CR2, USART_CR2_STOP_Msk);
    switch (handle->cfg.UART_stopbit) {
    case UART_onestop:
        // Bitfield of 0b00 sets one stop bit
        break;
    case UART_twostop:
        SETBITS(handle->regs->CR2, USART_CR2_STOP_1);
        break;
    default:
        *err = ERR_BADPARAM;
        return NULL;
        break;
    }
    /* Configure the parity setting */
    switch (handle->cfg.UART_parity) {
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
        *err = ERR_BADPARAM;
        return NULL;
        break;
    }
    /* Configure pinswap (swaps TX and RX pins) */
    switch (handle->cfg.UART_pin_swap) {
    case UART_pin_normal:
        CLEARBITS(handle->regs->CR2, USART_CR2_SWAP);
        break;
    case UART_pin_swapped:
        SETBITS(handle->regs->CR2, USART_CR2_SWAP);
        break;
    default:
        *err = ERR_BADPARAM;
        return NULL;
        break;
    }
    /* Select between UART MSB and LSB */
    switch (handle->cfg.UART_bit_order) {
    case UART_lsb_first:
        CLEARBITS(handle->regs->CR2, USART_CR2_MSBFIRST);
        break;
    case UART_msb_first:
        SETBITS(handle->regs->CR2, USART_CR2_MSBFIRST);
        break;
    default:
        *err = ERR_BADPARAM;
        return NULL;
        break;
    }
    /* Configure UART flow control */
    switch (handle->cfg.UART_flowcontrol) {
    case UART_no_flow:
        // Just need to disable the relevant bit fields
        CLEARBITS(handle->regs->CR3, USART_CR3_CTSE | USART_CR3_RTSE);
        break;
    case UART_flow_control:
        // Enable the UART flow control bits
        SETBITS(handle->regs->CR3, USART_CR3_CTSE | USART_CR3_RTSE);
        break;
    default:
        *err = ERR_BADPARAM;
        return NULL;
        break;
    }
    /**
     * Baud rate configuration. See p.1210 for baud rate formula. For
     * 16x oversampling (default) LPUART1 will use 256*fck/LPUARTDIV.
     * USARTx will use fck/USARTDIV. Default clock source for UART devices is
     * PCLK, running at 80MHz, but can be changed using the RCC_CCIPR register
     *
     * Values used below are taken from datasheet pg.1274
     */
    if (periph == LPUART_1) {
        switch (handle->cfg.UART_baud_rate) {
        case UART_baud_38400:
            handle->regs->BRR = 256U * pclk1_freq() / 38400;
            break;
        case UART_baud_57600:
            handle->regs->BRR = 256U * pclk1_freq() / 57600;
            break;
        case UART_baud_115200:
            handle->regs->BRR = 256U * pclk1_freq() / 115200;
            break;
        default:
            *err = ERR_BADPARAM;
            return NULL;
            break;
        }
    } else {
        // For all other UARTS, use the standard baud rate equation
        switch (handle->cfg.UART_baud_rate) {
        case UART_baud_auto:
            /* Special case. Write a starter value to the BRR register */
            handle->regs->BRR = 0x2B6; // 115200 baud
            break;
        case UART_baud_1200:
            handle->regs->BRR = 0x1046B;
            break;
        case UART_baud_2400:
            handle->regs->BRR = 0x8236;
            break;
        case UART_baud_4800:
            handle->regs->BRR = 0x411B;
            break;
        case UART_baud_9600:
            handle->regs->BRR = 0x208E;
            break;
        case UART_baud_19200:
            handle->regs->BRR = 0x1047;
            break;
        case UART_baud_38400:
            handle->regs->BRR = 0x824;
            break;
        case UART_baud_57600:
            handle->regs->BRR = 0x56D;
            break;
        case UART_baud_115200:
            handle->regs->BRR = 0x2B6;
            break;
        }
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
    /**
     * First, we must disable interrupts. If we don't do so, then we may get
     * inconsistent data from the ring buffer
     */
    mask_irq();
    // Now, attempt to read data from the input ring buffer
    num_read =
        buf_readblock(&(((UART_periph_status_t *)handle)->read_buf), buf, len);
    // reenable interrupts
    unmask_irq();
    timeout = (int)((UART_periph_status_t *)handle)->cfg.UART_timeout;
    while (num_read < len && timeout != UART_TIMEOUT_NONE) {
        /**
         * Wait for data to be available. For now, we will simply poll the
         * ringbuffer's size
         */
        while (buf_getsize(&(((UART_periph_status_t *)handle)->read_buf)) ==
                   0 &&
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
        num_read += buf_readblock(&(((UART_periph_status_t *)handle)->read_buf),
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
    int num_written, timeout;
    bool tx_inactive;
    UART_periph_status_t *uart = (UART_periph_status_t *)handle;
    if (len == 0) {
        return len;
    }
    /**
     * First, disable interrupts so we do not place bad data into the output
     * ring buffer
     */
    mask_irq();
    tx_inactive = !(uart->tx_active);
    // Now write data to the ring buffer
    num_written = UART_bufwrite(uart, buf, len);
    unmask_irq();
    // Advance the offset of buffer, and decrease len
    buf += num_written;
    len -= num_written;
    if (tx_inactive) {
        // We need to write data to the UART to start tranmission
        UART_transmit((UART_periph_status_t *)handle);
    }
    while (num_written < len && timeout != UART_TIMEOUT_NONE) {
        // Wait for there to be space in the ringbuffer
        while (buf_getsize(&(uart->write_buf)) == UART_RINGBUF_SIZE &&
               timeout != UART_TIMEOUT_NONE) {
            // If the timeout is set to infinity, we should just spin here
            if (timeout != UART_TIMEOUT_INF) {
                // Decrement the timeout
                delay_ms(200);
                if (timeout - 200 < UART_TIMEOUT_NONE) {
                    // Just set timeout to NONE (defined to be zero)
                    timeout = UART_TIMEOUT_NONE;
                } else {
                    timeout -= 200;
                }
            }
        }
        // There is space to write data. Write it.
        mask_irq();
        num_written += UART_bufwrite(uart, buf, len);
        unmask_irq();
    }
    // Now wait for all data to be sent
    while (timeout != UART_TIMEOUT_NONE &&
           buf_getsize(&(uart->write_buf)) > 0) {
        mask_irq();
        unmask_irq();
    }
    return num_written;
}

/**
 * Transmits data on the UART device provided.
 * Can be used to transmit data if none is actively being sent.
 */
static void UART_transmit(UART_periph_status_t *handle) {
    char data;
    if (buf_getsize(&(handle->write_buf)) != 0) {
        // Read a byte from the ring buffer and send it
        buf_read(&(handle->write_buf), &data);
        // Send by writing to the TDR register
        handle->regs->TDR = USART_TDR_TDR & data;
        if (handle->tx_active == false) {
            // Enable interrupts so that we can send next character
            handle->tx_active = true;
            SETBITS(handle->regs->CR1, USART_CR1_TE);
            SETBITS(handle->regs->CR1, USART_CR1_TXEIE);
        }
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
    UART_periph_status_t *handle = &UARTS[source];
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
    if (READBITS(handle->regs->ISR, USART_ISR_TXE) && handle->tx_active) {
        /**
         * Transmit data register is empty and ready for a new char.
         */
        UART_transmit(handle);
    }
    if (READBITS(handle->regs->ISR, USART_ISR_TC)) {
        // Transmission is complete. If buffer is empty, TX is no longer active
        if (buf_getsize(&(handle->write_buf)) == 0) {
            /**
             * No data is present. Wait for the TC bit to be set, then clear it
             * By waiting here, we ensure the UART is done transmitting data.
             */
            while (READBITS(handle->regs->ISR, USART_ISR_TC) == 0) {
            }
            handle->tx_active = false;
            CLEARBITS(handle->regs->CR1, USART_CR1_TXEIE);
            SETBITS(handle->regs->ICR, USART_ICR_TCCF);
        }
    }
}

/**
 * Writes data to a UART's output buffer, until the buffer is full or the
 * provided buffer is entirely written. returns the number of bytes written.
 * @param uart: UART handle to write to the output buffer of
 * @param buf: data to write to output buffer.
 * @param len: length of data to write
 */
static int UART_bufwrite(UART_periph_status_t *uart, uint8_t *buf, int len) {
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
                buf_writeblock(&(uart->write_buf), (uint8_t*)"\r\n", 2);
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