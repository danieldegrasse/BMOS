# Toolchain root
TOOLCHAIN_ROOT=/usr

# Debugger command
OPENOCD=openocd -f /usr/share/openocd/scripts/board/stm32l4discovery.cfg

# RTOS directory
RTOS=$(subst /drivers/test/clock,, $(PWD))

# Program name
PROG=clock-test

# Include drivers makefile
include $(RTOS)/rtos.mk