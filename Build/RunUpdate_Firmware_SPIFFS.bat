@echo off
setlocal enabledelayedexpansion

echo CocktailCube Firmware Uploader
echo You are about to upload the CocktailCube Firmware and the SPIFFS file system image to the ESP32-S2 Microcontroller.
echo.
echo What do you want to upload?
echo 0: Firmware + SPIFFS file system image
echo 1: Only Firmware
echo 2: Only SPIFFS file system image

:select_uploadFile
echo.
set /p upload_index=Select upload by number: 
set "selected_upload="

for /L %%I in (0,1,2) do (
  if "!upload_index!"=="%%I" set "selected_upload=%%I"
)

if "%selected_upload%"=="" (
  echo Invalid selection. Please try again.
  goto select_uploadFile
)

echo.
echo Available COM ports:
set "ports="
set /a index=0
for /f "tokens=2 delims==" %%A in ('wmic path Win32_SerialPort get DeviceID /value ^| findstr "COM"') do (
  echo !index!: %%A
  set "ports[!index!]=%%A"
  set /a index+=1
)

if %index%==0 (
  echo.
  echo No COM ports found.
  echo Press any key to exit...
  pause >nul
  exit /b
)

:select_port
echo.
set /p port_index=Select a COM port by number: 
set "selected_port="

for /L %%I in (0,1,%index%) do (
  if "!port_index!"=="%%I" set "selected_port=!ports[%%I]!"
)

if "%selected_port%"=="" (
  echo Invalid selection. Please try again.
  goto select_port
)

echo.
echo.

rem Firmware + SPIFFS
if "%selected_upload%"=="0" (
  echo Starting esptool.exe program with COM port %selected_port% and "Firmware + SPIFFS":
  echo.
  esptool.exe --chip esp32s2 --port "%selected_port%" --baud 921600  --before default_reset --after hard_reset write_flash  -z --flash_mode keep --flash_freq keep --flash_size keep 0x1000 "ESP32S2_CocktailCube_V1.3.ino.bootloader.bin" 0x8000 "ESP32S2_CocktailCube_V1.3.ino.partitions.bin" 0xe000 "boot_app0.bin" 0x10000 "ESP32S2_CocktailCube_V1.3.ino.bin" 0x00210000 "ESP32S2_CocktailCube_V1.3.ino.spiffs.bin"
) else (
  rem Firmware
  if "%selected_upload%"=="1" (
    echo Starting esptool.exe program with COM port %selected_port% and "Only Firmware":
    echo.
    esptool.exe --chip esp32s2 --port "%selected_port%" --baud 921600  --before default_reset --after hard_reset write_flash  -z --flash_mode keep --flash_freq keep --flash_size keep 0x1000 "ESP32S2_CocktailCube_V1.3.ino.bootloader.bin" 0x8000 "ESP32S2_CocktailCube_V1.3.ino.partitions.bin" 0xe000 "boot_app0.bin" 0x10000 "ESP32S2_CocktailCube_V1.3.ino.bin"
  ) else (
    rem SPIFFS
    if "%selected_upload%"=="2" (
      echo Starting esptool.exe program with COM port %selected_port% and "Only SPIFFS":
      echo.
      esptool.exe --chip esp32s2 --port "%selected_port%" --baud 921600  --before default_reset --after hard_reset write_flash  -z --flash_mode keep --flash_freq keep --flash_size keep 0x00210000 "ESP32S2_CocktailCube_V1.3.ino.spiffs.bin"
    ) else (
      echo Unknown selection error..
      echo.
    )
  )
)

pause