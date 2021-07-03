
# Toolchain root
TOOLCHAIN_ROOT=/usr

# Debugger command
OPENOCD=openocd -f /usr/share/openocd/scripts/board/stm32l4discovery.cfg

# Drivers directory
RTOS=$(subst /drivers/test/logging,, $(PWD))

# Program name
PROG=logging-test

# Include drivers makefile
include $(RTOS)/rtos.mk
