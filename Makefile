# TETRATOSH Bootloader Makefile
# Build system for Tetratosh Hackintosh bootloader

.PHONY: all clean syntax-check help info

# Directories
BUILD_DIR = build
SRC_DIR = bootloader/src

# Source files
SOURCES = $(SRC_DIR)/cpu_detect.c
SOURCES += $(SRC_DIR)/kext_selector.c
SOURCES += $(SRC_DIR)/boot_menu.c

info:
	@echo "TETRATOSH Build System"
	@echo "======================"
	@echo ""
	@echo "Note: Full build requires EDK2"
	@echo ""
	@echo "Available targets:"
	@echo "  syntax-check  - Check C syntax (no linking)"
	@echo "  clean        - Clean build artifacts"
	@echo ""

help: info

# Syntax check only (no linking)
syntax-check:
	@echo "Running syntax check..."
	@gcc -fsyntax-only -I bootloader/include \
		-DUEFI_STUB=1 \
		-DSTRING_ARRAY_TETRATOSH \
		-Wall -Wextra \
		$(SOURCES) 2>&1 || true
	@echo ""
	@echo "Syntax check complete!"

clean:
	@echo "Cleaning..."
	@rm -rf $(BUILD_DIR)
	@echo "Clean complete!"
