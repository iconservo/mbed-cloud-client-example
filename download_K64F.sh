#!/bin/sh

#python tools/combine_bootloader_with_app.py -a BUILD/K64F/GCC_ARM/mbed-cloud-client-example_application.bin -b tools/mbed-bootloader-k64f-block_device-sotp-v3_4_0.bin --app-offset 0x0000a400 --header-offset 0x0000a000 -o combined.bin
python tools/combine_bootloader_with_app.py -m k64f -a BUILD/K64F/GCC_ARM/mbed-cloud-client-example_application.bin -b tools/mbed-bootloader-k64f-block_device-sotp-v3_4_0.bin -o combined.bin
JLinkExe -device MK64FN1M0XXX12 -if SWD -speed 4000 -jtagconf -1,-1 -autoconnect 1 -CommanderScript download_K64F.jlink
