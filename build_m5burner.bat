@rem This script is used to build the firmware for the M5Burner
@rem It's necessary to merge the bootloader, partitions and the firmware binary
@rem Run "pip install esptool" if you don't have it installed already
esptool --chip esp32s3 merge_bin --output build/bonnethead.bin 0x0000 build/m5stack.esp32.m5stack_cardputer/cardputer_drums.ino.bootloader.bin 0x8000 build/m5stack.esp32.m5stack_cardputer/cardputer_drums.ino.partitions.bin 0x10000 build/m5stack.esp32.m5stack_cardputer/cardputer_drums.ino.bin