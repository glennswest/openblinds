./bin/mkspiffs -c data spiffs.bin
esptool.py --chip esp32c3 --port /dev/cu.usbmodem1101 --baud 921600 write_flash 0x3D0000 spiffs.bin -u -e 

