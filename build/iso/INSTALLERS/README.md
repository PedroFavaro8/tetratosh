# TETRATOSH - Onde Colocar os Instaladores

## Localização dos Instaladores

Coloque suas imagens do macOS (`.dmg`) nesta pasta:

```
tetratosh/
└── installers/
    ├── HighSierra/      ← High Sierra (10.13) - melhor para hardware antigo
    ├── Mojave/         ← Mojave (10.14)
    ├── Catalina/       ← Catalina (10.15)
    ├── BigSur/         ← Big Sur (11.x)
    ├── Monterey/       ← Monterey (12.x)
    ├── Ventura/        ← Ventura (13.x)
    └── Sonoma/         ← Sonoma (14.x)
```

## Como Adicionar um Instalador

Coloque o arquivo `.dmg` do instalador na pasta da versão correspondente:

```
installers/
├── HighSierra/
│   └── Install_macOS_High_Sierra.dmg
├── Mojave/
│   └── Install_macOS_Mojave.dmg
├── Catalina/
│   └── Install_macOS_Catalina.dmg
├── BigSur/
│   └── Install_macOS_Big_Sur.dmg
├── Monterey/
│   └── Install_macOS_Monterey.dmg
├── Ventura/
│   └── Install_macOS_Ventura.dmg
└── Sonoma/
    └── Install_macOS_Sonoma.dmg
```

## Hardware Recomendado por Versão

| Versão      | Mínimo        | Recomendado     |
|-------------|---------------|-----------------|
| High Sierra | Core 2 Duo    | Core i3+        |
| Mojave      | Core 2 Duo    | Core i3+        |
| Catalina    | Core 2 Duo    | Core i3+        |
| Big Sur     | Core 2 Duo    | Core i3+        |
| Monterey    | Core i3       | Core i5+        |
| Ventura     | Core i3       | Core i5+        |
| Sonoma      | Core i3       | Core i5+        |

## Notas Importantes

1. **High Sierra é mais compatível** com hardware antigo (Pentium 4, Core 2, etc.)
2. **Não é necessário ter todos** - basta a versão que deseja usar
3. O bootloader detecta automaticamente qual versão você escolheu
4. Funciona **100% offline** - não precisa de internet no momento do boot

## Fontes para Baixar .dmg

### Direto da Apple (gratuitos):
- **macOS Sonoma**: https://apps.apple.com/br/app/macos-sonoma/id1676664810
- **macOS Ventura**: https://apps.apple.com/br/app/macos-ventura/id1634002220
- **macOS Monterey**: https://apps.apple.com/br/app/macos-monterey/id1620118043
- **macOS Big Sur**: https://apps.apple.com/br/app/macos-big-sur/id1520871294
- **macOS Catalina**: https://apps.apple.com/br/app/macos-catalina/id1466841314
- **macOS Mojave**: https://apps.apple.com/br/app/macos-mojave/id1528447930
- **macOS High Sierra**: https://support.apple.com/pt-br/HT208969

### Como converter .app para .dmg
Se você tem o `.app` mas precisa do `.dmg`:
```bash
# No macOS original
hdiutil create -o /tmp/HighSierra -size 8g -layout SPUD -fs HFS+J
hdiutil attach /tmp/HighSierra.dmg
# Copie o conteúdo do app para a imagem
hdiutil detach /tmp/HighSierra
hdiutil convert /tmp/HighSierra.dmg -format UDZO -o Install_macOS_High_Sierra.dmg
```

## Estrutura Interna Suportada

O TETRATOSH suporta os seguintes formatos de `.dmg`:

1. **InstallESD.dmg** - Imagem de instalação completa
2. **BaseSystem.dmg** - Sistema base
3. **Direct .dmg** - Imagem com o instalador

## Exemplo de Estrutura

```
installers/HighSierra/
├── InstallESD.dmg          ← Imagem principal
├── BaseSystem.dmg          ← (opcional)
└── packages/               ← (opcional, se usando installer de pasta)
    └── ...
```

O bootloader vai detectar automaticamente qual arquivo usar!
