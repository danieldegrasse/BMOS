
# Toolchain root
TOOLCHAIN_ROOT=/usr

# Debugger command
OPENOCD=openocd -f /usr/share/openocd/scripts/board/stm32l4discovery.cfg

# RTOS directory
RTOS=$(subst /util/test/list,, $(PWD))

# Program name
PROG=list-test

# Include drivers makefile
include $(RTOS)/rtos.mk