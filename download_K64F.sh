#!/bin/sh

JLinkExe -device MK64FN1M0XXX12 -if SWD -speed 4000 -jtagconf -1,-1 -autoconnect 1 -CommanderScript download_K64F.jlink
