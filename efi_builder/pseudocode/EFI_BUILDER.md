# Módulo: EFI Builder - Pseudocódigo

## 1. Interface do Módulo

```c
// efi_builder.h

#ifndef EFI_BUILDER_H
#define EFI_BUILDER_H

#include "hardware_detector.h"
#include "kext_selector.h"
#include "config_generator.h"

#define EFI_PARTITION_PATH L"EFI"
#define OC_FOLDER_PATH L"EFI\\OC"
#define KEKTS_FOLDER_PATH L"EFI\\OC\\Kexts"
#define DRIVERS_FOLDER_PATH L"EFI\\OC\\Drivers"
#define ACPI_FOLDER_PATH L"EFI\\OC\\ACpi"

EFI_STATUS BuildEFI(
    SystemInfo *SystemInfo,
    ListKexts *Kexts,
    ConfigPlist *Config,
    CHAR16 *TargetPath
);

EFI_STATUS CopyKexts(ListKexts *Kexts, CHAR16 *TargetPath);
EFI_STATUS CopyOpenCoreDrivers(CHAR16 *TargetPath);
EFI_STATUS CopyACPI(CHAR16 *TargetPath);
EFI_STATUS GenerateConfigPlistFile(ConfigPlist *Config, CHAR16 *TargetPath);

#endif
```

## 2. Implementação Principal

```c
// efi_builder.c

EFI_STATUS BuildEFI(
    SystemInfo *SystemInfo,
    ListKexts *Kexts,
    ConfigPlist *Config,
    CHAR16 *TargetPath
) {
    EFI_STATUS Status;
    
    Log("Starting EFI build...");
    
    // 1. Criar estrutura de diretórios
    Log("Creating directory structure...");
    Status = CreateEFIDirectories(TargetPath);
    SE Status != EFI_SUCCESS:
        LogError("Failed to create directories");
        RETORNAR Status;
    FIM SE
    
    // 2. Copiar OpenCore
    Log("Copying OpenCore files...");
    Status = CopyOpenCore(TargetPath);
    SE Status != EFI_SUCCESS:
        LogError("Failed to copy OpenCore");
        RETORNAR Status;
    FIM SE
    
    // 3. Copiar Drivers
    Log("Copying UEFI drivers...");
    Status = CopyOpenCoreDrivers(TargetPath);
    SE Status != EFI_SUCCESS:
        LogWarn("Some drivers may be missing");
    FIM SE
    
    // 4. Copiar Kexts
    Log("Copying kexts...");
    Status = CopyKexts(Kexts, TargetPath);
    SE Status != EFI_SUCCESS:
        LogError("Failed to copy kexts");
        RETORNAR Status;
    FIM SE
    
    // 5. Copiar ACPI
    Log("Copying ACPI tables...");
    Status = CopyACPI(TargetPath);
    SE Status != EFI_SUCCESS:
        LogWarn("ACPI tables may need manual attention");
    FIM SE
    
    // 6. Gerar config.plist
    Log("Generating config.plist...");
    Status = GenerateConfigPlistFile(Config, TargetPath);
    SE Status != EFI_SUCCESS:
        LogError("Failed to generate config.plist");
        RETORNAR Status;
    FIM SE
    
    // 7. Validar estrutura
    Log("Validating EFI structure...");
    Status = ValidateEFIStructure(TargetPath);
    SE Status != EFI_SUCCESS:
        LogWarn("EFI validation had warnings");
    FIM SE
    
    Log("EFI build complete!");
    
    RETORNAR EFI_SUCCESS;
}
```

## 3. Estrutura de Diretórios

```c
// directory_builder.c

EFI_STATUS CreateEFIDirectories(CHAR16 *BasePath) {
    EFI_STATUS Status;
    CHAR16 Path[MAX_PATH];
    
    // Lista de diretórios a criar
    CHAR16* Directories[] = {
        L"EFI",
        L"EFI\\BOOT",
        L"EFI\\OC",
        L"EFI\\OC\\ACPI",
        L"EFI\\OC\\Drivers",
        L"EFI\\OC\\Kexts",
        L"EFI\\OC\\Resources",
        L"EFI\\OC\\Resources\\Audio",
        L"EFI\\OC\\Resources\\Font",
        L"EFI\\OC\\Resources\\Image",
        L"EFI\\OC\\Tools",
    };
    
    PARA i = 0 ATÉ ARRAY_SIZE(Directories):
        // Construir caminho completo
        SwPrint(Path, MAX_PATH, L"%s\\%s", BasePath, Directories[i]);
        
        // Criar diretório
        Status = CreateDirectoryRecursively(Path);
        SE Status != EFI_SUCCESS:
            LogError("Failed to create: %s", Path);
            RETORNAR Status;
        FIM SE
        
        Log("Created: %s", Path);
    FIM PARA
    
    RETORNAR EFI_SUCCESS;
}
```

## 4. Copiar Kexts

```c
// kext_copier.c

EFI_STATUS CopyKexts(ListKexts *Kexts, CHAR16 *TargetPath) {
    EFI_STATUS Status;
    CHAR16 SourcePath[MAX_PATH];
    CHAR16 DestPath[MAX_PATH];
    UINTN i;
    
    PARA i = 0 ATÉ Kexts->Count:
        KextInfo *Kext = &Kexts->Items[i];
        
        // Procurar kext no sistema de arquivos
        Status = FindKextFile(Kext->Name, SourcePath);
        
        SE Status == EFI_SUCCESS:
            // Construir destino
            SwPrint(DestPath, MAX_PATH, 
                L"%s\\EFI\\OC\\Kexts\\%s", 
                TargetPath, Kext->Name);
            
            // Copiar
            Status = CopyFileTree(SourcePath, DestPath);
            
            SE Status == EFI_SUCCESS:
                Log("Copied: %s", Kext->Name);
            SENÃO:
                LogError("Failed to copy: %s", Kext->Name);
            FIM SE
        SENÃO:
            LogWarn("Kext not found: %s", Kext->Name);
        FIM SE
    FIM PARA
    
    RETORNAR EFI_SUCCESS;
}
```

## 5. Gerar config.plist

```c
// config_writer.c

EFI_STATUS GenerateConfigPlistFile(ConfigPlist *Config, CHAR16 *TargetPath) {
    EFI_STATUS Status;
    CHAR16 FilePath[MAX_PATH];
    EFI_FILE_HANDLE File;
    UINT8 *Buffer;
    UINTN Size;
    
    // Construir caminho
    SwPrint(FilePath, MAX_PATH, 
        L"%s\\EFI\\OC\\config.plist", TargetPath);
    
    // Abrir arquivo
    Status = OpenFile(FilePath, &File, EFI_FILE_MODE_WRITE);
    SE Status != EFI_SUCCESS:
        RETORNAR Status;
    FIM SE
    
    // Gerar XML
    Buffer = GeneratePlistXML(Config, &Size);
    
    // Escrever
    Status = File->Write(File, &Size, Buffer);
    
    // Cleanup
    FreePool(Buffer);
    File->Close(File);
    
    Log("Generated: config.plist");
    
    RETORNAR Status;
}

UINT8* GeneratePlistXML(ConfigPlist *Config, UINTN *OutSize) {
    STRING_BUFFER *sb;
    UINT8 *Result;
    
    sb = CreateStringBuffer(8192);
    
    // XML Header
    AppendString(sb, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    AppendString(sb, "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n");
    AppendString(sb, "<plist version=\"1.0\">\n");
    AppendString(sb, "<dict>\n");
    
    // ACPI
    AppendString(sb, "  <key>ACPI</key>\n");
    AppendString(sb, "  <dict>\n");
    AppendACPI(sb, &Config->ACPI);
    AppendString(sb, "  </dict>\n");
    
    // Kernel
    AppendString(sb, "  <key>Kernel</key>\n");
    AppendString(sb, "  <dict>\n");
    AppendKernel(sb, &Config->Kernel);
    AppendString(sb, "  </dict>\n");
    
    // UEFI
    AppendString(sb, "  <key>UEFI</key>\n");
    AppendString(sb, "  <dict>\n");
    AppendUEFI(sb, &Config->UEFI);
    AppendString(sb, "  </dict>\n");
    
    // SMBIOS
    AppendString(sb, "  <key>SMBIOS</key>\n");
    AppendString(sb, "  <dict>\n");
    AppendSMBIOS(sb, &Config->SMBIOS);
    AppendString(sb, "  </dict>\n");
    
    // NVRAM
    AppendString(sb, "  <key>NVRAM</key>\n");
    AppendString(sb, "  <dict>\n");
    AppendNVRAM(sb, &Config->NVRAM);
    AppendString(sb, "  </dict>\n");
    
    AppendString(sb, "</dict>\n");
    AppendString(sb, "</plist>\n");
    
    Result = GetStringBufferContent(sb, OutSize);
    FreeStringBuffer(sb);
    
    RETORNAR Result;
}
```

---

# Módulo: OpenCore Launcher - Pseudocódigo

## 1. Interface do Módulo

```c
// opencore_launcher.h

#ifndef OPENCORE_LAUNCHER_H
#define OPENCORE_LAUNCHER_H

#include <Uefi.h>

EFI_STATUS LaunchOpenCore();
EFI_STATUS LocateOpenCore(CHAR16 **Path);
EFI_STATUS PrepareBootParameters(EFI_HANDLE ImageHandle);

#endif
```

## 2. Implementação do Chainload

```c
// opencore_launcher.c

EFI_STATUS LaunchOpenCore() {
    EFI_STATUS Status;
    CHAR16 *OpenCorePath;
    EFI_DEVICE_PATH *DevicePath;
    EFI_HANDLE OpenCoreHandle;
    UINTN ExitDataSize;
    CHAR16 *ExitData;
    
    Log("=== Launching OpenCore ===");
    
    // 1. Localizar OpenCore.efi
    Log("Locating OpenCore.efi...");
    Status = LocateOpenCore(&OpenCorePath);
    
    SE Status != EFI_SUCCESS:
        LogError("OpenCore not found!");
        RETORNAR Status;
    FIM SE
    
    Log("Found OpenCore at: %s", OpenCorePath);
    
    // 2. Converter caminho para DevicePath
    DevicePath = FileDevicePath(NULL, OpenCorePath);
    
    SE DevicePath == NULL:
        LogError("Failed to create device path");
        RETORNAR EFI_OUT_OF_RESOURCES;
    FIM SE
    
    // 3. Carregar OpenCore na memória
    Log("Loading OpenCore.efi...");
    Status = gBS->LoadImage(
        FALSE,                    // BootPolicy
        gImageHandle,             // ParentImageHandle
        DevicePath,               // FilePath
        NULL,                     // SourceBuffer
        0,                        // SourceSize
        &OpenCoreHandle           // ImageHandle
    );
    
    SE Status != EFI_SUCCESS:
        LogError("Failed to load OpenCore: %r", Status);
        RETORNAR Status;
    FIM SE
    
    Log("OpenCore loaded successfully");
    
    // 4. Preparar parâmetros de boot
    Log("Preparing boot parameters...");
    Status = PrepareBootParameters(OpenCoreHandle);
    
    SE Status != EFI_SUCCESS:
        LogWarn("Failed to prepare boot params, continuing anyway");
    FIM SE
    
    // 5. Iniciar OpenCore (chainload)
    Log("Starting OpenCore...");
    Status = gBS->StartImage(
        OpenCoreHandle,
        &ExitDataSize,
        &ExitData
    );
    
    // Se retornou, o OpenCore encerrou
    SE Status != EFI_SUCCESS:
        LogError("OpenCore returned error: %r", Status);
        
        // Exibir exit data se disponível
        SE ExitDataSize > 0:
            LogError("Exit data: %s", ExitData);
        FIM SE
    FIM SE
    
    RETORNAR Status;
}

EFI_STATUS LocateOpenCore(CHAR16 **Path) {
    EFI_STATUS Status;
    EFI_FILE_PROTOCOL *Root;
    EFI_FILE_PROTOCOL *File;
    CHAR16 *SearchPaths[] = {
        L"EFI\\OC\\OpenCore.efi",
        L"EFI\\BOOT\\OpenCore.efi",
        L"\\EFI\\OC\\OpenCore.efi",
    };
    
    PARA i = 0 ATÉ ARRAY_SIZE(SearchPaths):
        Status = OpenFile(SearchPaths[i], &File, EFI_FILE_MODE_READ);
        
        SE Status == EFI_SUCCESS:
            *Path = StrDuplicate(SearchPaths[i]);
            File->Close(File);
            RETORNAR EFI_SUCCESS;
        FIM SE
    FIM PARA
    
    RETORNAR EFI_NOT_FOUND;
}
```

## 3. Parâmetros de Boot

```c
// boot_params.c

EFI_STATUS PrepareBootParameters(EFI_HANDLE ImageHandle) {
    EFI_STATUS Status;
    EFI_LOADED_IMAGE_PROTOCOL *LoadedImage;
    
    // Obter LoadedImage protocol
    Status = gBS->HandleProtocol(
        ImageHandle,
        &gEfiLoadedImageProtocolGuid,
        (VOID**)&LoadedImage
    );
    
    SE Status != EFI_SUCCESS:
        RETORNAR Status;
    FIM SE
    
    // Configurar DevicePath
    LoadedImage->DeviceHandle = gImageHandle;
    
    // Configurar LoadOptions com variáveis de boot
    // Isso permite passar argumentos para o OpenCore
    
    // Por exemplo: -v para verbose
    // LoadedImage->LoadOptions = L"-v";
    // LoadedImage->LoadOptionsSize = StrLen(LoadedImage->LoadOptions) * 2;
    
    RETORNAR EFI_SUCCESS;
}
```

## 4. Tratamento de Erros

```c
// launcher_errors.c

VOID HandleOpenCoreError(EFI_STATUS Status) {
    LogError("=== OpenCore Launch Failed ===");
    LogError("Error code: 0x%X", Status);
    
    // Mapear códigos de erro comuns
    SWITCH Status:
        CASO EFI_NOT_FOUND:
            LogError("OpenCore.efi was not found");
            LogError("Please ensure EFI structure is correct");
            BREAK;
            
        CASO EFI_LOAD_ERROR:
            LogError("Failed to load OpenCore image");
            LogError("EFI file may be corrupted");
            BREAK;
            
        CASO EFI_INVALID_PARAMETER:
            LogError("Invalid parameters passed to OpenCore");
            LogError("Check config.plist");
            BREAK;
            
        CASO EFI_OUT_OF_RESOURCES:
            LogError("Out of memory");
            BREAK;
            
        CASO_DEFAULT:
            LogError("Unknown error");
            BREAK;
    FIM SWITCH
    
    // Oferecer opções de recovery
    ShowRecoveryOptions();
}

VOID ShowRecoveryOptions() {
    Log("");
    Log("Recovery Options:");
    Log("1. Reboot system");
    Log("2. Enter UEFI Shell");
    Log("3. Retry");
    
    EFI_INPUT_KEY Key;
    gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
    
    SWITCH Key.UnicodeChar:
        CASO '1':
            gRT->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
            BREAK;
            
        CASO '2':
            LaunchUEFIShell();
            BREAK;
            
        CASO '3':
            LaunchOpenCore();
            BREAK;
    FIM SWITCH
}
```
