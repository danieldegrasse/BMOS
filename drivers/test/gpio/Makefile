
# Toolchain root
TOOLCHAIN_ROOT=/usr

# Debugger command
OPENOCD=openocd -f /usr/share/openocd/scripts/board/stm32l4discovery.cfg

# Drivers directory
DRIVERS=$(subst /test/gpio,, $(PWD))

# Program name
PROG=gpio-test

# Include drivers makefile
include $(DRIVERS)/drivers.mk