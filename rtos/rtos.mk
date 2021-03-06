## PROG=example # Must be set by user

# Toolchain root
## TOOLCHAIN_ROOT=/usr # Must be set by user

# Toolchain tools
CC=$(TOOLCHAIN_ROOT)/bin/arm-none-eabi-gcc
LD=$(TOOLCHAIN_ROOT)/bin/arm-none-eabi-ld
OBJCOPY=$(TOOLCHAIN_ROOT)/bin/arm-none-eabi-objcopy
GDB=$(TOOLCHAIN_ROOT)/bin/arm-none-eabi-gdb
SIZE=$(TOOLCHAIN_ROOT)/bin/arm-none-eabi-size

# Debugger command (Must be set by user)
## OPENOCD=openocd -f /usr/share/openocd/scripts/board/stm32l4discovery.cfg

# RTOS directory
##  RTOS=rtos # Must be set by user


# Note that mthumb is required. Cortex M executes in T32 mode.
local_CFLAGS += -mcpu=cortex-m4 \
	-mthumb \
	-Wall \
	-Werror \
	-ffreestanding \
	-isystem $(RTOS) \
	-nostartfiles\
	--specs=nano.specs \
	$(CFLAGS)
local_LDFLAGS += -Wl,-T $(RTOS)/linker_script.ld \
	-Wl,-Map=$(BUILDDIR)/$(PROG).map \
	$(LDFLAGS)

# Build output directory
BUILDDIR=build

# Excluded build paths. Should not have a trailing slash.
# Any files in these directories will not be built
EXCLUDED_DIRS=$(RTOS)/drivers/test $(RTOS)/util/test $(RTOS)/sys/test

###### recursive wildcard function #######
rwildcard=$(wildcard $1$2) $(foreach d, \
	$(filter-out $(EXCLUDED_DIRS), $(wildcard $1*)),\
	$(call rwildcard,$d/,$2))

## All directories to search for source files
DIRS=$(RTOS)

# Add all .c files in current directory to compilation
SRCS=$(wildcard *.c)

# Recursively find all .c files in DIRS
SRCS+= $(foreach d, $(DIRS),$(call rwildcard,$(d),*.c))   


# Object files (autogenerated from sources)
OBJ=$(SRCS:%.c=%.o)
OBJDIR=$(BUILDDIR)/obj
_OBJ=$(patsubst %,$(OBJDIR)/%,$(OBJ))

# Enable debugging symbols on default build
all: local_CFLAGS+=-g
all: $(BUILDDIR)/$(PROG).bin

# Disable system logging and optimize code for release build
release: local_CFLAGS+=-O2 -DSYSLOG=3
release: $(BUILDDIR)/$(PROG).bin
	@echo "Release build"

# Output compiled object files into BUILDDIR
$(OBJDIR)/%.o: %.c 
	@ [ -d $(dir $@) ] || mkdir -p $(dir $@)
	@ echo "[CC] $<"
	@ $(CC) $(local_CFLAGS) -c -o $@ $< 

$(BUILDDIR)/$(PROG).bin: $(BUILDDIR)/$(PROG).elf
	@ echo "Creating $@"
	@ $(OBJCOPY) -O binary $^ $@

$(BUILDDIR)/$(PROG).elf: $(_OBJ)
	@ echo "Linking $@"
	@ $(CC) -o $@ $^ $(local_CFLAGS) $(local_LDFLAGS)
	@ echo "Code sizes for $@:"
	@ $(SIZE) -G $@

##### Flash code to board using OpenOCD (0x08000000 is start of flash bank)
flash: $(BUILDDIR)/$(PROG).bin
	$(OPENOCD) -c "program $^ 0x08000000 reset exit"

## Start debugserver, which flashes the program at boot
debugserver: all
	$(OPENOCD) -c "program $(BUILDDIR)/$(PROG).bin 0x08000000 reset verify; \
	reset init; gdb_breakpoint_override hard"

## Start debugger and connect to debugserver
debug: all
	$(GDB) -ex 'target extended-remote localhost:3333' \
	$(BUILDDIR)/$(PROG).elf


.PHONY: clean erase

clean:
	@ if [ -d $(BUILDDIR) ]; then \
		echo "rm -r $(BUILDDIR)"; \
		rm -r $(BUILDDIR); \
	fi

##### Erase all flash memory from board using OpenOCD #####
erase:
	$(OPENOCD) -c "init; reset halt; stm32l4x mass_erase 0; exit"


