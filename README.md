# Saturation Arithmetic Lab

Laboratório em C17 para estudar aritmética saturada no Windows x64 com MSVC. O primeiro experimento compara a soma modular de `PADDW` com a soma signed saturada de `PADDSW`, mostra a conversão saturada de `PACKSSWB` e permite inspecionar o código gerado em `/Od` e `/O2`.

## O que observar

Em cada lane de 16 bits, os operandos `30000` e `10000` têm soma matemática `40000`:

| Operação | Resultado exibido como signed | Padrão de 16 bits | Regra |
| --- | ---: | ---: | --- |
| `PADDW` | `-25536` | `0x9C40` | Soma modular, sem saturação |
| `PADDSW` | `32767` | `0x7FFF` | Saturação signed no limite superior |

O programa exibe oito lanes lado a lado, imprime os bits da primeira e compara a implementação SIMD com uma referência escalar em `65.536 × 9` entradas. Também demonstra `PACKSSWB` e um exemplo **deliberadamente incorreto** que usa `LONG_MAX`/`LONG_MIN` como limites de `int64_t` no Windows x64.

## Compilar e executar

Requisitos: Visual Studio 2022 ou Build Tools com **Desenvolvimento para desktop com C++**, toolset MSVC v143 e Windows SDK 10.x. O projeto seleciona x64 e C17.

1. Abra [`SaturationLab/SaturationLab.sln`](SaturationLab/SaturationLab.sln) no Visual Studio.
2. Selecione `Debug | x64` (`/Od`) ou `Release | x64` (`/O2`).
3. Compile com `Ctrl+Shift+B` e execute com `Ctrl+F5`.

Como alternativa, abra o **x64 Native Tools Command Prompt** na pasta `SaturationLab` e execute `build_msvc.cmd`. Esse script compila as duas configurações, executa os testes e coleta logs, listings e disassembly em `SaturationLab/out/`. Consulte o [`LEIA-ME.txt`](SaturationLab/LEIA-ME.txt) para os caminhos gerados e os detalhes de inspeção no IDA/WinDbg.

## Arquivos

| Caminho | Conteúdo |
| --- | --- |
| [`saturation_msvc.c`](SaturationLab/saturation_msvc.c) | Funções escalares, intrinsics SSE2, demonstração e testes |
| [`SaturationLab.sln`](SaturationLab/SaturationLab.sln) e [`SaturationLab.vcxproj`](SaturationLab/SaturationLab.vcxproj) | Solução e projeto do Visual Studio |
| [`build_msvc.cmd`](SaturationLab/build_msvc.cmd) | Build `/Od` e `/O2` e coleta dos artefatos |
| [`LEIA-ME.txt`](SaturationLab/LEIA-ME.txt) | Instruções completas, limites e referências |

## Escopo da validação

Este repositório publica o **primeiro experimento SSE2**. A matriz posterior de quatro instruções saturantes (`PADDSW`, `PADDUSW`, `PSUBSW`, `PSUBUSW`) e 2.359.296 comparações descrita no artigo foi feita em uma etapa posterior; o código dessa etapa não está neste pacote.

O pacote original foi preparado em Linux e conferido com GCC e UndefinedBehaviorSanitizer. Nesta publicação, a lógica também foi executada com GCC `-O2` e UBSan sem falhas nos testes incluídos. Isso **não valida** a abertura da solução, o build MSVC nem o assembly no Windows; faça essa conferência no ambiente indicado antes de atribuir resultados de código gerado a uma versão do compilador.

O projeto é um laboratório de correção e inspeção de código gerado, sem medição de desempenho. As fontes técnicas usadas estão no [`LEIA-ME.txt`](SaturationLab/LEIA-ME.txt).
