# Bootloader Module - Pseudocódigo

## 1. Estrutura Principal

```c
// boot/main.c - Entry point do bootloader

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>

// Headers do Tetratosh
#include "hardware_detector.h"
#include "kext_selector.h"
#include "config_generator.h"
#include "efi_builder.h"
#include "opencore_launcher.h"
#include "logging.h"

// Variáveis globais
EFI_HANDLE gImageHandle;
EFI_SYSTEM_TABLE *gST;

// Função principal
EFI_STATUS EFIAPI UefiMain(
    EFI_HANDLE ImageHandle,
    EFI_SYSTEM_TABLE *SystemTable
) {
    EFI_STATUS Status;
    
    // Inicializar variáveis globais
    gImageHandle = ImageHandle;
    gST = SystemTable;
    
    // Inicializar sistema de logging
    Log_Init();
    
    Log("===========================================");
    Log("  TETRATOSH - Automated Hackintosh Boot");
    Log("===========================================");
    
    // Loop principal
    Status = TetratoshMain();
    
    SE Status != EFI_SUCCESS:
        LogError("Fatal error: 0x%X", Status);
        EnterRecoveryMode();
    FIM SE
    
    RETORNAR Status;
}

EFI_STATUS TetratoshMain() {
    EFI_STATUS Status;
    SystemInfo sys_info;
    ListaKexts kexts_selecionados;
    ConfigPlist config;
    
    // Fase 1: Detecção de Hardware
    Log("Phase 1: Hardware Detection");
    Status = DetectarHardware(&sys_info);
    SE Status != EFI_SUCCESS:
        LogError("Hardware detection failed");
        RETORNAR Status;
    FIM SE
    
    ExibirInformacoesHardware(sys_info);
    
    // Fase 2: Seleção de Kexts
    Log("Phase 2: Kext Selection");
    Status = SelecionarKexts(sys_info, &kexts_selecionados);
    SE Status != EFI_SUCCESS:
        LogError("Kext selection failed");
        RETORNAR Status;
    FIM SE
    
    ExibirKextsSelecionados(kexts_selecionados);
    
    // Fase 3: Geração de Config
    Log("Phase 3: Config Generation");
    Status = GerarConfigPlist(sys_info, kexts_selecionados, &config);
    SE Status != EFI_SUCCESS:
        LogError("Config generation failed");
        RETORNAR Status;
    FIM SE
    
    // Fase 4: Construção da EFI
    Log("Phase 4: EFI Building");
    Status = ConstruirEFI(sys_info, kexts_selecionados, config);
    SE Status != EFI_SUCCESS:
        LogError("EFI building failed");
        RETORNAR Status;
    FIM SE
    
    // Fase 5: Chainload do OpenCore
    Log("Phase 5: Launching OpenCore");
    Status = IniciarOpenCore();
    SE Status != EFI_SUCCESS:
        LogError("OpenCore launch failed");
        RETORNAR Status;
    FIM SE
    
    Log("Tetratosh execution complete");
    RETORNAR EFI_SUCCESS;
}
```

## 2. Sistema de Logging

```c
// logging.h

#define LOG_LEVEL_DEBUG 0
#define LOG_LEVEL_INFO  1
#define LOG_LEVEL_WARN  2
#define LOG_LEVEL_ERROR 3

typedef struct {
    UINTN Level;
    BOOLEAN EnableUI;
    CHAR16 *LogBuffer;
    UINTN BufferSize;
    UINTN CurrentPos;
} LogSystem;

VOID Log_Init();
VOID Log(CHAR8 *Format, ...);
VOID LogError(CHAR8 *Format, ...);
VOID LogWarn(CHAR8 *Format, ...);
VOID LogDebug(CHAR8 *Format, ...);

// logging.c

EFI_STATUS Log_Init() {
    gLogSystem.Level = LOG_LEVEL_INFO;
    gLogSystem.EnableUI = TRUE;
    gLogSystem.BufferSize = 64 * 1024; // 64KB
    gLogSystem.CurrentPos = 0;
    
    // Alocar buffer
    gLogSystem.LogBuffer = AllocateZeroPool(gLogSystem.BufferSize);
    
    SE gLogSystem.LogBuffer == NULL:
        RETORNAR EFI_OUT_OF_RESOURCES;
    FIM SE
    
    // Limpar tela
    gST->ConOut->ClearScreen(gST->ConOut);
    
    RETORNAR EFI_SUCCESS;
}

VOID Log(CHAR8 *Format, ...) {
    VA_LIST Args;
    CHAR8 Buffer[512];
    
    // Formatar string
    VA_START(Args, Format);
    AsciiVSPrint(Buffer, sizeof(Buffer), Format, Args);
    VA_END(Args);
    
    // Imprimir na tela
    gST->ConOut->OutputString(gST->ConOut, Buffer);
    gST->ConOut->OutputString(gST->ConOut, L"\r\n");
    
    // Salvar no buffer
    SE gLogSystem.LogBuffer != NULL:
        AsciiSPrint(
            gLogSystem.LogBuffer + gLogSystem.CurrentPos,
            gLogSystem.BufferSize - gLogSystem.CurrentPos,
            "%a\n",
            Buffer
        );
        gLogSystem.CurrentPos += AsciiStrLen(Buffer) + 1;
    FIM SE
}
```

## 3. Interface Gráfica

```c
// ui.h

#define MAX_ROWS 25
#define MAX_COLS 80

typedef struct {
    UINTN Row;
    UINTN Col;
    UINTN Width;
    UINTN Height;
    CHAR16 *Title;
    BOOLEAN Visible;
} UIPanel;

VOID UI_Init();
VOID UI_Clear();
VOID UI_DrawBox(UINTN X, UINTN Y, UINTN W, UINTN H, CHAR16 *Title);
VOID UI_ProgressBar(UINTN X, UINTN Y, UINTN W, FLOAT64 Progress);
VOID UI_SetCursor(UINTN Row, UINTN Col);
VOID UI_PrintAt(UINTN Row, UINTN Col, CHAR16 *Format, ...);

// ui.c - Tela Principal

VOID UI_ShowMainScreen() {
    UI_Clear();
    
    // Título
    UI_DrawBox(0, 0, 80, 25, L" TETRATOSH ");
    
    // Logotipo ASCII
    UI_PrintAt(3, 20, L"  #######  #######  #######  ");
    UI_PrintAt(4, 20, L"  #     #  #     #  #       ");
    UI_PrintAt(5, 20, L"  #     #  #     #  #       ");
    UI_PrintAt(6, 20, L"  #     #  #######  ######  ");
    UI_PrintAt(7, 20, L"  #     #  #     #  #       ");
    UI_PrintAt(8, 20, L"  #######  #     #  ####### ");
    
    UI_PrintAt(10, 20, L"Automated Hackintosh Bootloader");
    
    // Barra de status
    UI_PrintAt(23, 2, L"Detecting Hardware...");
}

VOID UI_UpdateStatus(CHAR16 *Status) {
    UI_PrintAt(23, 2, L"                                    ");
    UI_PrintAt(23, 2, L"%s", Status);
}

VOID UI_ShowKextList(ListaKexts kexts) {
    UINTN i;
    CHAR16 Buffer[128];
    
    // Panel de kexts
    UI_DrawBox(45, 10, 34, 12, L"Selected Kexts");
    
    PARA i = 0 ATÉ kexts.Contar E i < 10:
        SE kexts[i].Selecionado:
            SwPrint(Buffer, L"  + %s", kexts[i].Nome);
            UI_PrintAt(12 + i, 47, Buffer);
        FIM SE
    FIM PARA
}
```

## 4. Tratamento de Erros

```c
// error.h

#define ERR_HW_DETECT      0x01
#define ERR_NO_MEMORY      0x02
#define ERR_PCI_SCAN       0x03
#define ERR_ACPI_PARSE     0x04
#define ERR_NO_KEXTS       0x05
#define ERR_CONFIG_GEN     0x06
#define ERR_EFI_BUILD      0x07
#define ERR_OPENCORE       0x08

typedef struct {
    UINTN Code;
    CHAR16 *Message;
    CHAR16 *Suggestion;
} ErrorInfo;

VOID EnterRecoveryMode();
VOID ShowErrorScreen(UINTN ErrorCode, CHAR16 *Message);
BOOLEAN PromptUserRetry();

// error.c

VOID ShowErrorScreen(UINTN ErrorCode, CHAR16 *Message) {
    UI_Clear();
    
    // Draw error box
    UI_DrawBox(10, 5, 60, 15, L" ERROR ");
    
    UI_PrintAt(8, 20, L"Error Code: 0x%X", ErrorCode);
    UI_PrintAt(10, 15, Message);
    UI_PrintAt(12, 15, L"Suggestion: %s", GetSuggestion(ErrorCode));
    
    UI_PrintAt(16, 20, L"Press any key to continue...");
    
    // Aguardar input
    EFI_INPUT_KEY Key;
    gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
}

VOID EnterRecoveryMode() {
    Log("Entering recovery mode...");
    
    UI_Clear();
    UI_DrawBox(10, 5, 60, 15, L" Recovery Mode ");
    
    UI_PrintAt(8, 15, L"TETRATOSH encountered an error.");
    UI_PrintAt(10, 15, L"Options:");
    UI_PrintAt(12, 15, L"1. Retry");
    UI_PrintAt(13, 15, L"2. Shell");
    UI_PrintAt(14, 15, L"3. Exit");
    
    EFI_INPUT_KEY Key;
    gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
    
    SE Key.UnicodeChar == '1':
        TetratoshMain();
    SENÃO SE Key.UnicodeChar == '2':
        LaunchShell();
    FIM SE
}
```

## 5. Utilitários UEFI

```c
// utils.h

typedef struct {
    CHAR16 *Path;
    UINTN Size;
    EFI_FILE_HANDLE Handle;
} FileInfo;

EFI_STATUS ReadFile(CHAR16 *Path, VOID **Buffer, UINTN *Size);
EFI_STATUS WriteFile(CHAR16 *Path, VOID *Buffer, UINTN Size);
EFI_STATUS CreateDirectory(CHAR16 *Path);
BOOLEAN FileExists(CHAR16 *Path);

// utils.c

EFI_STATUS ReadFile(CHAR16 *Path, VOID **Buffer, UINTN *Size) {
    EFI_STATUS Status;
    EFI_FILE_HANDLE File;
    EFI_FILE_INFO *Info;
    UINTN BufferSize;
    
    // Abrir arquivo
    Status = gBS->OpenProtocol(
        gImageHandle,
        &gEfiSimpleFileSystemProtocolGuid,
        NULL,
        gImageHandle,
        NULL,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL
    );
    
    SE Status != EFI_SUCCESS:
        RETORNAR Status;
    FIM SE
    
    // Get file info para obter tamanho
    Info = LibFileInfo(File);
    
    // Alocar buffer
    *Buffer = AllocatePool(Info->FileSize);
    
    SE *Buffer == NULL:
        RETORNAR EFI_OUT_OF_RESOURCES;
    FIM SE
    
    // Ler arquivo
    Status = File->Read(File, &BufferSize, *Buffer);
    
    SE Status != EFI_SUCCESS:
        FreePool(*Buffer);
        RETORNAR Status;
    FIM SE
    
    *Size = BufferSize;
    RETORNAR EFI_SUCCESS;
}
```
