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
  DebugLib|MdePkg/Library/DebugLib/DebugLib.inf
  PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  MemoryAllocationLib|MdePkg/Library/DxeMemoryAllocationLib/DxeMemoryAllocationLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  FileSystemLib|MdePkg/Library/DxeFileSystemLib/DxeFileSystemLib.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  StringLib|MdePkg/Library/BaseStringLib/BaseStringLib.inf

[LibraryClasses.common.UEFI_DRIVER]
  MemoryInitLib|MdeModulePkg/Library/MemoryInitLib/MemoryInitLib.inf

[Components]
  # Main Bootloader Driver
  Tetratosh/TetratoshDriver.inf
