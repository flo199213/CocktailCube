# Upload builds

You can use esptool and the following command to upload the builds to your ESP32-S2:

"esptool.exe" --chip esp32s2 --port "COMXX" --baud 921600  --before default_reset --after hard_reset write_flash  -z --flash_mode keep --flash_freq keep --flash_size keep 0x1000 "ESP32S2_CocktailCube_V1.X.ino.bootloader.bin" 0x8000 "ESP32S2_CocktailCube_V1.X.ino.partitions.bin" 0xe000 "..\AppData\Local\Arduino15\packages\esp32\hardware\esp32\3.0.0-alpha2/tools/partitions/boot_app0.bin" 0x10000 "ESP32S2_CocktailCube_V1.X.ino.bin" 


* Notice:
You have to set the COM-Port, the pathes to your bin-Files, the version number you want to upload and the path to your AppData directory (with installed ESP32 package).

-> Will not upload the images for the startpage! (Use ESP32 Data Uploader Tool or Webpage "192.168.1.1/edit" for V1.2 or later instead)
