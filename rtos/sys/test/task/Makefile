
# Toolchain root
TOOLCHAIN_ROOT=/usr

# Debugger command
OPENOCD=openocd -f /usr/share/openocd/scripts/board/stm32l4discovery.cfg

# Drivers directory
RTOS=$(subst /sys/test/task,, $(PWD))

# Program name
PROG=task-test

# Include drivers makefile
include $(RTOS)/rtos.mk
