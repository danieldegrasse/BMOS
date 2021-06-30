
# Toolchain root
TOOLCHAIN_ROOT=/usr

# Debugger command
OPENOCD=openocd -f /usr/share/openocd/scripts/board/stm32l4discovery.cfg

# Drivers directory
DRIVERS=drivers

# Program name
PROG=uart-echo

# Include drivers makefile
include $(DRIVERS)/drivers.mk