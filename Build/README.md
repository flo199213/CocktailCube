# Upload builds

<br>

For direct CocktailCube Firmware and SPIFFS upload simply start **"RunUpdate_Firmware_SPIFFS.bat"**. You will be asked for Upload variant and COM Port.

<br>

Otherwise you can upload manually:

---

* Firmware Build

You can use esptool and the following command to upload the firmware builds to your ESP32-S2:

```diff
"esptool.exe" --chip esp32s2 --port "COMXX" --baud 921600  --before default_reset --after hard_reset write_flash  -z --flash_mode keep --flash_freq keep --flash_size keep 0x1000 "ESP32S2_CocktailCube_V1.3.ino.bootloader.bin" 0x8000 "ESP32S2_CocktailCube_V1.3.ino.partitions.bin" 0xe000 "boot_app0.bin" 0x10000 "ESP32S2_CocktailCube_V1.3.ino.bin"
```

-> Will not upload the images for the startpage! (Use ESP32 Data Uploader Tool, Webpage "192.168.1.1/edit" for V1.2 or later or following SPIFFS image instead)

---

* SPIFFS image

You can use esptool and the following command to upload the SPIFFS image to your ESP32-S2:

```diff
"esptool.exe" --chip esp32s2 --port "COMXX" --baud 921600  --before default_reset --after hard_reset write_flash  -z --flash_mode keep --flash_freq keep --flash_size keep 0x00210000 "ESP32S2_CocktailCube_V1.3.ino.spiffs.bin"
```

---

* Notice:
```diff
- > You have to set the COM-Port to upload your files
```
