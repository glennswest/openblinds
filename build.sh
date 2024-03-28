./bin/mkspiffs -c data spiffs.bin
esptool.py --chip esp32c3 --port /dev/cu.usbmodem1101 --baud 921600 write_flash -z 0x110000 spiffs.bin

