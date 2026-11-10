@echo off
setlocal EnableExtensions DisableDelayedExpansion
pushd "%~dp0"
if errorlevel 1 exit /b 1

where cl >nul 2>&1
if errorlevel 1 (
    echo Abra o x64 Native Tools Command Prompt do Visual Studio e execute este script.
    popd
    exit /b 1
)
where dumpbin >nul 2>&1
if errorlevel 1 (
    echo dumpbin nao encontrado. Use o x64 Native Tools Command Prompt.
    popd
    exit /b 1
)
if not exist out mkdir out
if not exist out (
    echo Nao foi possivel criar a pasta out.
    popd
    exit /b 1
)
ver >out\windows.txt

call :build Od /Od
if errorlevel 1 goto :failed
call :build O2 /O2
if errorlevel 1 goto :failed

findstr /I "paddw paddsw packsswb" out\saturation_O2.disasm.txt >out\simd_instructions.txt
echo.
echo Compilacao e testes concluidos para /Od e /O2.
echo Logs, executaveis, PDBs e listings estao na pasta out.
echo.
type out\simd_instructions.txt
echo.
type out\saturation_O2.run.txt
popd
exit /b 0

:build
echo Compilando %1...
cl /nologo /Bv /TC /std:c17 /W4 /WX %2 /Zi /FAcs /Faout\saturation_%1.cod /Foout\saturation_%1.obj /Fdout\compiler_%1.pdb /Feout\saturation_%1.exe saturation_msvc.c /link /DEBUG /PDB:out\saturation_%1.pdb >out\saturation_%1.build.txt 2>&1
if errorlevel 1 (
    type out\saturation_%1.build.txt
    exit /b 1
)
out\saturation_%1.exe >out\saturation_%1.run.txt 2>&1
if errorlevel 1 (
    type out\saturation_%1.run.txt
    exit /b 1
)
dumpbin /nologo /disasm out\saturation_%1.exe >out\saturation_%1.disasm.txt 2>&1
if errorlevel 1 (
    type out\saturation_%1.disasm.txt
    exit /b 1
)
exit /b 0

:failed
echo.
echo Houve uma falha. Consulte os logs acima e na pasta out.
popd
exit /b 1
