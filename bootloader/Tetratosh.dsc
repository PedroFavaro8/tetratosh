#
# TETRATOSH Bootloader DSC
# Automated Hackintosh Bootloader
#

[Defines]
  PLATFORM_NAME                  = Tetratosh
  PLATFORM_GUID                  = 12345678-1234-1234-1234-123456789ABC
  PLATFORM_VERSION                = 1.0
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/Tetratosh
  SUPPORTED_ARCHITECTURES        = X64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT

[LibraryClasses]
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  BaseDebugLibNull|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  BasePcdLibNull|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  UefiDevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  BasePrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  BaseStringLib|MdePkg/Library/BaseStringLib/BaseStringLib.inf

[LibraryClasses.common.UEFI_DRIVER]
  MemoryInitLib|MdeModulePkg/Library/MemoryInitLib/MemoryInitLib.inf

[Components]
  # Main Bootloader Driver
  /workspaces/tetratosh/bootloader/TetratoshDriver.inf
