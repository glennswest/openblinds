./bin/mklittlefs -s 1572864  -c data littlefs.bin
esptool.py --chip esp32c3 --port /dev/cu.usbmodem101 --baud 921600 write_flash 0x290000 littlefs.bin -u  

