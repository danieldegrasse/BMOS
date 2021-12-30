# BMOS
A basic RTOS kernel offering preemptive task switching, distinct task priority, and semaphores for synchronization. This project also includes STM324L433RC specific drivers designed for use with the RTOS kernel.
## Project Components
This project includes 
- a kernel core in `rtos/sys`, which offers tasks, as well as semaphores
- device-agnostic utility libraries, including a logging subsystem, list management, and a ring buffer in `rtos/util`
- STM324L433RC specific drivers for UART peripherals and GPIO outputs, in `rtos/drivers`
- Limited newlib support (`write` and `sbrk` implemented) in `rtos/sys/syscalls.c`

## RTOS Component
This RTOS is designed for the Cortex-M series of ARM MCUs. It implements cooperative multitasking (with optional priority preemption), as well as task stack protection and semaphores for synchronization.

### Scheduling
The scheduler uses task priorities to determine which task will be selected, and a running task must explicitly yield. If preemption is enabled, higher priority tasks that become ready to run will preempt lower priority ones. Otherwise multitasking is entirely cooperative.

### Synchronization
The RTOS supports semaphores for synchronization. Semaphores may be counting or binary, and p() operations may supply a timeout parameter.

### Additional Features
Statically allocated task stacks are supported, as well as dynamic ones. Task stack protection is implemented via a padded section at the end of stack of configurable size, and task overflow checking in the idle task

## Driver Component
Drivers are implemented for the STM32L433RC within the `drivers` directory, and can run without the RTOS being started (but will use synchronization methods such as semaphores when it is). A UART driver, device agnostic semihosting/SWO driver, clock driver, and GPIO driver are implemented.
### UART Driver
The UART driver supports all world lengths supported by the STM32L433RC UART devices, as well as several advanced features including swapping the TX/RX pins and enabling hardware flow control. It implements an optional 'echo mode', that will echo data back to the UART device (for a console), as well as automatic replacement of newlines with CRLF for console usage. Regardless of the status of the RTOS, the UART driver is entirely interrupt driven
### GPIO driver
The GPIO driver supports analog digital reads and writes, as well as enabling interrupts on any GPIO pin via the EXTI interrupt controller

### SWO/Semihost Drivers
Both of these drivers implement logging facilities, for SWO and Semihosting respectively. The logging subsystem the RTOS uses can be configured in `config.h`. The SWO driver logs at 2MHz.

### Clock driver
The clock driver is STM324L433RC specific, and supports setting the system clock to use the MSI, PLL, or HSI16 oscillator. The default configuration is to use the PLL with an 80MHz cpu clock and peripheral clock, but this can be configured to a variety of frequencies by setting the PLL divider, or the MSI can be used across its range of supported frequencies.

## Utilities Component
The system utilities include a simple statically allocated ring buffer, as well as a list implementation, also avoiding dynamic allocation. Finally, a logging subsystem is implemented to simplify debugging

# Building and Running
The project is designed to run on an STM32L433 Nucleo64 board (hence the name), although the kernel and SWO/Semihost drivers (and all utilities) should be able to run on any Cortex M4 core. To build the demo application, ensure you have the following dependencies installed:
- `arm-none-eabi-gcc`
- `arm-none-ebai-newlib`
- `openocd`

With these programs installed, simply edit the file `demo/Makefile` to reflect the root of your toolchain, and the path to your openocd binary (as well as to the board script file). The program can then be built and flashed by changing to the `demo` directory and running `make flash`. A release build (with logging disabled) can be created with `make release`. Build files are output to the `build` directory.

## Viewing Logs
Logs can viewed using SWO, or using semihosting (configurable by editing `config.h`). SWO can be configured by any debugging utility preferred, or the logging system can be switched to semihosting. Logging via the LPUART1 device (exposed via a UART to usb converter) can be enabled, but in the demo application the LPUART1 device is used by the application itself.
