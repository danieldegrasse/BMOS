
# Toolchain root
TOOLCHAIN_ROOT=/usr

# Debugger command
OPENOCD=openocd -f /usr/share/openocd/scripts/board/stm32l4discovery.cfg

# Drivers directory
RTOS=rtos

# Program name
PROG=main

# Include drivers makefile
include $(RTOS)/rtos.mk