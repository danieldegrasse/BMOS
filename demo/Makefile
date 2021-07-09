
# Toolchain root
TOOLCHAIN_ROOT=/usr

# Debugger command
OPENOCD=openocd -f /usr/share/openocd/scripts/board/stm32l4discovery.cfg

# RTOS directory
RTOS=$(subst /demo,/rtos, $(PWD))

# Program name
PROG=demo

# Include drivers makefile
include $(RTOS)/rtos.mk