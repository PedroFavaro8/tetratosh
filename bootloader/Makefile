# TETRATOSH Bootloader Makefile
# Compile with EDK2 toolchain

# EDK2 paths (customize for your system)
EDK2_PATH ?= /usr/local/EDK2
EDK_TOOLS_PATH ?= $(EDK2_PATH)/BaseTools
GCC5_PREFIX ?= x86_64-w64-mingw32-

# Build target
TARGET = X64
ARCH = X64
TOOL_CHAIN = GCC5
TOOL_CHAIN_TAG = GCC5

# Paths
<<<<<<< HEAD
BOOTLOADER_DIR = .
OUTPUT_DIR = $(BOOTLOADER_DIR)/build
=======
BOOTLOADER_DIR = $(CURDIR)
OUTPUT_DIR = $(CURDIR)/build
>>>>>>> 6afc6e3 (updated)
EFI_DIR = $(OUTPUT_DIR)/efi

# Source files
SOURCES = src/main.c
SOURCES += src/cpu_detect.c
<<<<<<< HEAD
SOURCES += src/pci_scan.c
SOURCES += src/acpi_parse.c
SOURCES += src/kext_selector.c
SOURCES += src/config_gen.c
SOURCES += src/efi_builder.c
SOURCES += src/opencore.c
SOURCES += src/ui.c
SOURCES += src/logger.c
=======
SOURCES += src/kext_selector.c
SOURCES += src/boot_menu.c
>>>>>>> 6afc6e3 (updated)

# Include paths
INCLUDES = -I$(BOOTLOADER_DIR)/include
INCLUDES += -I$(EDK2_PATH)/MdePkg/Include
INCLUDES += -I$(EDK2_PATH)/MdePkg/Include/X64
INCLUDES += -I$(EDK2_PATH)/MdeModulePkg/Include
INCLUDES += -I$(EDK2_PATH)/BaseTools/Source/C/include

# Compiler flags
CFLAGS = -g -fno-stack-protector -fno-pie -fno-omit-frame-pointer
CFLAGS += -Wno-error -Wall -Wextra
CFLAGS += -Wno-unused-parameter
CFLAGS += -DSTRING_ARRAY_TETRATOSH
CFLAGS += -UEFI_STUB

# Target arch flags
CFLAGS += -m64 -mno-red-zone
CFLAGS += -fshort-wchar -fno-builtin
CFLAGS += -ffunction-sections -fdata-sections

# Linker flags
LDFLAGS = -nostdlib -Wl,-n,-q,--gc-sections
LDFLAGS += -Wl,--entry,UefiMain
LDFLAGS += -u UefiMain
LDFLAGS += -Wl,-e,_UefiMain

# Output
TARGET_NAME = Tetratosh.efi
TARGET_DSC = $(BOOTLOADER_DIR)/Tetratosh.dsc

.PHONY: all clean info help build-efi

all: info build-efi

info:
	@echo "TETRATOSH Bootloader Build System"
	@echo "==================================="
	@echo "EDK2 Path: $(EDK2_PATH)"
	@echo "Output: $(OUTPUT_DIR)"
	@echo "Target: $(TARGET)"
	@echo ""

help:
	@echo "Targets:"
	@echo "  build-efi    - Build UEFI bootloader"
	@echo "  build-all    - Build everything"
	@echo "  clean        - Clean build artifacts"
	@echo "  info         - Show build info"
	@echo ""
	@echo "Options:"
	@echo "  EDK2_PATH=/path/to/edk2 - Set EDK2 path"

build-efi:
	@echo "Building TETRATOSH bootloader..."
	@mkdir -p $(OUTPUT_DIR)
	@mkdir -p $(EFI_DIR)
	
# Check if EDK2 is available
ifneq ($(wildcard $(EDK2_PATH)/BaseTools/Source/C/Makefiles),)
	@echo "Using EDK2 build system..."
	cd $(EDK2_PATH) && $(EDK_TOOLS_PATH)/Build -p $(TARGET_DSC) -a $(ARCH) -b DEBUG -t $(TOOL_CHAIN_TAG) -n 3
else
	@echo "WARNING: EDK2 not found at $(EDK2_PATH)"
	@echo "Please install EDK2 or set EDK2_PATH"
	@echo ""
	@echo "Building with GCC (standalone mode)..."
	$(GCC5_PREFIX)gcc $(CFLAGS) $(INCLUDES) $(SOURCES) -o $(OUTPUT_DIR)/$(TARGET_NAME) $(LDFLAGS)
endif

build-all: build-efi
	@echo "Running download scripts..."
	@python3 ../scripts/download_all_keysts.py -v || true
	@python3 ../scripts/download_acpi.py -v || true

clean:
	@echo "Cleaning..."
	@rm -rf $(OUTPUT_DIR)
	@rm -rf $(EFI_DIR)
	@echo "Clean complete!"

# Legacy build (for testing without EDK2)
legacy-build:
	@echo "Building legacy (no EDK2)..."
	@mkdir -p $(OUTPUT_DIR)
	gcc -c $(CFLAGS) $(INCLUDES) $(SOURCES) -o $(OUTPUT_DIR)/main.o
	@echo "Note: This is a stub build only!"
